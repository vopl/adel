#include "pch.hpp"
#include "async/impl/service.hpp"
#include "async/log.hpp"
#include "async/freeFunctions.hpp"

#include <log4cplus/configurator.h>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

namespace async { namespace impl
{
	//////////////////////////////////////////////////////////////////////////
	ThreadLocalStorage<Service *> Service::_current;

	Service *Service::_global = NULL;

	//////////////////////////////////////////////////////////////////////////
	void Service::onTimer(const TTimerPtr &timer, const boost::system::error_code &ec, Future<boost::system::error_code> res)
	{
		res(ec);

		boost::mutex::scoped_lock sl(_mtxTimers);
		_timers.erase(timer);
	}


	//////////////////////////////////////////////////////////////////////////
	Service::Service()
		: _io()
		, _work()
		, _fiberPool(new FiberPool)
		, _stackSize(1024*64)
		, _maxFibers(1000)
		, _workersAmount(0)
	{
		TLOG("constructed");
	}

	//////////////////////////////////////////////////////////////////////////
	Service::~Service()
	{
		TLOG("destructed");
		//stop();
	}

	//////////////////////////////////////////////////////////////////////////
	boost::signals2::connection Service::connectOnStart(const boost::function<void()> &f)
	{
		return _onStart.connect(f);
	}

	//////////////////////////////////////////////////////////////////////////
	boost::signals2::connection Service::connectOnStop(const boost::function<void()> &f)
	{
		return _onStop.connect(f);
	}

	//////////////////////////////////////////////////////////////////////////
	boost::signals2::connection Service::connectOnWorkerStart(const boost::function<void()> &f)
	{
		return _onWorkerStart.connect(f);
	}

	//////////////////////////////////////////////////////////////////////////
	boost::signals2::connection Service::connectOnWorkerStop(const boost::function<void()> &f)
	{
		return _onWorkerStop.connect(f);
	}

	//////////////////////////////////////////////////////////////////////////
	void Service::setupFibers(size_t stackSize, size_t maxAmount)
	{
		_stackSize = stackSize;
		_maxFibers = maxAmount;
	}

	//////////////////////////////////////////////////////////////////////////
	void Service::start(size_t numThreads)
	{
		ILOG("start");
		balance(numThreads);

		spawn(boost::bind(&Service::onStart, shared_from_this()));

		ILOG("start done");
	}

	//////////////////////////////////////////////////////////////////////////
	void Service::balance(size_t numThreads)
	{
		ILOG("balance "<<numThreads);

		std::vector<WorkerPtr>	stopped;
		{
			boost::mutex::scoped_lock sl(_mtxWorkers);

			while(_workers.size() > numThreads)
			{
				stopped.push_back(_workers.back());
				_workers.pop_back();

			}
			while(_workers.size() < numThreads)
			{
				WorkerPtr swp(new Worker(shared_from_this(), _fiberPool));
				_workers.push_back(swp);
			}
		}

		if(!stopped.empty())
		{
			_work.reset();
			BOOST_FOREACH(WorkerPtr &swp, stopped)
			{
				swp.reset();
			}
		}

		//ждать пока воркеры запустятся/остановятся
		{
		    boost::unique_lock<boost::mutex> lock(_mtxWorkers);
			if(!_workers.empty())
			{
				_work.reset(new boost::asio::io_service::work(_io));
			}
			else
			{
				cancelAllTimeouts();
			}

		    while(_workers.size() != _workersAmount)
		    {
		    	_cvWorkersAmount.wait(lock);
		    }
			//TLOG(__FUNCTION__<<": "<<_workersAmount);
		}

		ILOG("balance done");
	}

	//////////////////////////////////////////////////////////////////////////
	void Service::stop()
	{
		ILOG("stop");
		spawn(boost::bind(&Service::onStop, shared_from_this()));
		balance(0);
		ILOG("stop done");
	}

	//////////////////////////////////////////////////////////////////////////
	size_t Service::getStackSize()
	{
		return _stackSize;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t Service::getMaxFibers()
	{
		return _maxFibers;
	}

	//////////////////////////////////////////////////////////////////////////
	void Service::spawn(const boost::function<void ()> &code)
	{
		{
			boost::mutex::scoped_lock sl(_fiberPool->_mtxTasks);
			if(!_fiberPool->_tasks.empty())
			{
				//исполнение уже идет, воркер будет выгребать все задачи, подсунуть ему еще и эту
				_fiberPool->_tasks.push_back(code);
				return;
			}
		}

		//небыло задач к исполнителю, надо инициировать воркера
		_io.post(bridge(code));
	}

	//////////////////////////////////////////////////////////////////////////
	Future<boost::system::error_code> Service::timeout(size_t millisec)
	{
		Future<boost::system::error_code> res;
		boost::mutex::scoped_lock sl(_mtxTimers);
		TTimerPtr timer(new boost::asio::deadline_timer(_io, boost::posix_time::milliseconds(millisec)));
		_timers.insert(timer);
		timer->async_wait(bridge(boost::bind(&Service::onTimer, shared_from_this(), timer, _1, res)));

		return res;
	}

	//////////////////////////////////////////////////////////////////////////
	void Service::cancelAllTimeouts()
	{
		boost::mutex::scoped_lock sl(_mtxTimers);
		BOOST_FOREACH(const TTimerPtr &timer, _timers)
		{
			timer->cancel();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	boost::asio::io_service &Service::io()
	{
		return _io;
	}

	//////////////////////////////////////////////////////////////////////////
	bool Service::setAsGlobal(bool force)
	{
		if(_global && !force)
		{
			return false;
		}
		_global = this;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	Service *Service::current()
	{
		Service *res = _current;
		if(!res)
		{
			res = _global;
		}
		return res;
	}

	//////////////////////////////////////////////////////////////////////////
	void Service::onStart()
	{
		_onStart();
	}

	//////////////////////////////////////////////////////////////////////////
	void Service::onStop()
	{
		_onStop();
	}

	//////////////////////////////////////////////////////////////////////////
	void Service::onThreadStart()
	{
		_current = this;

		{
			{
				boost::lock_guard<boost::mutex> lock(_mtxWorkers);
				_workersAmount++;
				//TLOG(__FUNCTION__<<": "<<_workersAmount);
			}
			_cvWorkersAmount.notify_one();
		}

		_onWorkerStart();
	}

	//////////////////////////////////////////////////////////////////////////
	void Service::onThreadStop()
	{
		_onWorkerStop();

		{
			{
				boost::lock_guard<boost::mutex> lock(_mtxWorkers);
				_workersAmount--;
				//TLOG(__FUNCTION__<<": "<<_workersAmount);
			}
			_cvWorkersAmount.notify_one();
		}

		_current = NULL;
	}

}}
