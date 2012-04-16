#include "pch.hpp"
#include "async/impl/worker.hpp"
#include "async/impl/service.hpp"
#include "async/log.hpp"
#include "async/service.hpp"

#include <boost/bind.hpp>

namespace async { namespace impl
{

	//////////////////////////////////////////////////////////////////////////
	ThreadLocalStorage<Worker *> Worker::_current;

	//////////////////////////////////////////////////////////////////////////
	void Worker::threadProc()
	{
		_current = this;

		//наболтать головной фибер
		_fiberRoot.reset(new FiberRoot());
		_fiberRoot->initialize();

		_service->onThreadStart();

		while(!_stop)
		{
			boost::system::error_code ec;
			try
			{
				_service->io().run(ec);
			}
			catch(...)
			{
				ELOG(__FUNCTION__<<", exception catched: "<<boost::current_exception_diagnostic_information());
			}

		}

		_service->onThreadStop();

		_fiberRoot.reset();
		_current = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void Worker::fiberExecuted(Fiber *fiber)
	{
		assert(fiber != _fiberRoot.get());
		assert(fiber == Fiber::current());
		{
			boost::mutex::scoped_lock sl(_fiberPool->_mtxFibers);
			_fiberPool->_fibersIdle.insert(fiber->shared_from_this());
			assert(
				_fiberPool->_fibersReady.end() == 
				std::find(_fiberPool->_fibersReady.begin(), _fiberPool->_fibersReady.end(), fiber->shared_from_this()));
		}
		bool b = _fiberRoot->activate();
		assert(b);
	}

	//////////////////////////////////////////////////////////////////////////
	void Worker::fiberReady(FiberPtr fiber)
	{
		boost::mutex::scoped_lock sl(_fiberPool->_mtxFibers);
		assert(fiber != _fiberRoot);
		//assert(fiber.get() != FiberImpl::current());
		assert(_fiberPool->_fibersIdle.end() == _fiberPool->_fibersIdle.find(fiber));
		_fiberPool->_fibersReady.push_back(fiber);
	}

	//////////////////////////////////////////////////////////////////////////
	bool Worker::fiberReadyIfWait(FiberPtr fiber)
	{
		boost::mutex::scoped_lock sl(_fiberPool->_mtxFibers);
		assert(fiber != _fiberRoot);
		assert(fiber.get() != Fiber::current());

		if(_fiberPool->_fibersIdle.end() != _fiberPool->_fibersIdle.find(fiber))
		{
			return false;
		}
		_fiberPool->_fibersReady.push_back(fiber);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void Worker::fiberYield()
	{
		assert(Fiber::current() != _fiberRoot.get());
		bool b = _fiberRoot->activate();
		assert(b);
	}


	//////////////////////////////////////////////////////////////////////////
	void Worker::processReadyFibers()
	{
		std::deque<FiberPtr> fibersNotActivated;


		for(;;)
		{
			FiberPtr fiber;
			{
				boost::mutex::scoped_lock sl(_fiberPool->_mtxFibers);
				if(!_fiberPool->_fibersReady.empty())
				{
					fiber = _fiberPool->_fibersReady.front();
					_fiberPool->_fibersReady.pop_front();
				}
			}

			if(fiber)
			{
				if(!fiber->activate())
				{
					fibersNotActivated.push_back(fiber);
				}
			}
			else
			{
				break;
			}
		}

		if(!fibersNotActivated.empty())
		{
			boost::mutex::scoped_lock sl(_fiberPool->_mtxFibers);
			_fiberPool->_fibersReady.insert(
				_fiberPool->_fibersReady.end(),
				fibersNotActivated.begin(),
				fibersNotActivated.end());
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void Worker::exec(const TTask &task)
	{
		//assert(FiberImpl::current() == _fiberRoot.get());
		if(Fiber::current() != _fiberRoot.get())
		{
			task();
			return;
		}
		assert(Fiber::current() == _fiberRoot.get());

		//сначала отработать все готовые
		processReadyFibers();

		//потом отложенные задачи
		//потом входящую задачу
		std::set<FiberPtr>	fibersNotActivated;
		for(;;)
		{
			FiberPtr fiber;
			TTask tasksFromQueue;
			size_t totalFibersAmount=0;//это просто кэш

			//////////////////////////////////////////////////////////////////////////
			{
				boost::mutex::scoped_lock sl(_fiberPool->_mtxTasks);
				if(!_fiberPool->_tasks.empty())
				{
					tasksFromQueue.swap(*_fiberPool->_tasks.begin());
					_fiberPool->_tasks.erase(_fiberPool->_tasks.begin());
				}
			}
			{
				boost::mutex::scoped_lock sl(_fiberPool->_mtxFibers);
				totalFibersAmount = _fiberPool->_totalAmount;
				if(!_fiberPool->_fibersIdle.empty())
				{
					fiber.swap((FiberPtr &)*_fiberPool->_fibersIdle.begin());
					_fiberPool->_fibersIdle.erase(_fiberPool->_fibersIdle.begin());
				}
			}

			//////////////////////////////////////////////////////////////////////////
			if(!fiber)
			{
				if(totalFibersAmount < _service->getMaxFibers())
				{
					try
					{
						fiber.reset(new Fiber(_service->getStackSize()));
						if(fiber->initialize())
						{
							boost::mutex::scoped_lock sl(_fiberPool->_mtxFibers);
							_fiberPool->_totalAmount++;
						}
						else
						{
							fiber.reset();
						}
					}
					catch(...)
					{
						fiber.reset();
					}
				}
				else
				{
					ELOG("fibers limit exceeded ("<<_service->getMaxFibers()<<")");
				}
			}

			if(fiber)
			{
				if(tasksFromQueue)
				{
					if(!fiber->execute(tasksFromQueue))
					{
						fibersNotActivated.insert(fiber);
						continue;
					}
				}
				else
				{
					if(!fiber->execute(task))
					{
						fibersNotActivated.insert(fiber);
						continue;
					}
					break;
				}
			}
			else
			{
				boost::mutex::scoped_lock sl(_fiberPool->_mtxTasks);
				if(tasksFromQueue)
				{
					_fiberPool->_tasks.push_front(tasksFromQueue);
				}
				_fiberPool->_tasks.push_back(task);
				break;
			}
		}

		if(!fibersNotActivated.empty())
		{
			boost::mutex::scoped_lock sl(_fiberPool->_mtxFibers);
			_fiberPool->_fibersIdle.insert(fibersNotActivated.begin(), fibersNotActivated.end());
		}

		//теперь снова готовые
		processReadyFibers();
	}

	//////////////////////////////////////////////////////////////////////////
	void Worker::yield()
	{
		assert(Fiber::current());

		FiberPtr fiber = Fiber::current()->shared_from_this();
		if(fiber == _fiberRoot)
		{
			assert(!"root fiber unable to yield");
			return;
		}

		service()->io().post(bridge(bind(&Worker::fiberReady, shared_from_this(), fiber)));
		fiberYield();
	}

	//////////////////////////////////////////////////////////////////////////
	Worker::Worker(ServicePtr service, FiberPoolPtr	fiberPool)
		: _service(service)
		, _stop(false)
		, _fiberRoot()
		, _fiberPool(fiberPool)
	{
		_thread = boost::thread(boost::bind(&Worker::threadProc, this));
	}

	//////////////////////////////////////////////////////////////////////////
	Worker::~Worker()
	{
		_stop = true;
		_thread.join();

		assert(!_fiberRoot);
	}

	//////////////////////////////////////////////////////////////////////////
	const ServicePtr &Worker::service()
	{
		return _service;
	}

	//////////////////////////////////////////////////////////////////////////
	Worker *Worker::current()
	{
		return _current;
	}

}}
