#include "pch.hpp"
#include "async/impl/event.hpp"
#include "async/impl/worker.hpp"
#include "async/exception.hpp"

#include <boost/foreach.hpp>

namespace async { namespace impl
{
	//////////////////////////////////////////////////////////////////////////
	Event::Event(bool autoReset)
		: _isSet(false)
		, _autoReset(autoReset)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	Event::~Event()
	{
		boost::mutex::scoped_lock sl(_mtx);
		assert(_waiters.empty());
		BOOST_FOREACH(FiberPtr &fiber, _waiters)
		{
			Worker::current()->fiberReady(_waiters.front());
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void Event::set()
	{
		boost::mutex::scoped_lock sl(_mtx);
		if(!_isSet)
		{
			if(!_autoReset)
			{
				_isSet = true;

				BOOST_FOREACH(MultiNotifierMarked &mn, _mnWaiters)
				{
					mn._pmn->notifyReady(mn._key);
				}
				_mnWaiters.clear();

				BOOST_FOREACH(FiberPtr &f, _waiters)
				{
					assert(f.get() != Fiber::current());
					Worker::current()->fiberReady(f);
				}
				_waiters.clear();
				return;
			}

			if(!_mnWaiters.empty())
			{
				for(std::vector<MultiNotifierMarked>::iterator ipmn(_mnWaiters.begin()); ipmn!=_mnWaiters.end(); ipmn++)
				{
					if(ipmn->_pmn->notifyReady(ipmn->_key))
					{
						_mnWaiters.erase(_mnWaiters.begin(), ipmn+1);
						return;
					}
				}
				_mnWaiters.clear();
			}

			if(!_waiters.empty())
			{
				FiberPtr f;
				f.swap(_waiters.front());
				_waiters.erase(_waiters.begin());
				Worker::current()->fiberReady(f);
				return;
			}

			_isSet = true;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void Event::reset()
	{
		boost::mutex::scoped_lock sl(_mtx);
		if(_isSet)
		{
			assert(_waiters.empty());
			_waiters.clear();
			_isSet = false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool Event::isSet()
	{
		return _isSet;
	}

	//////////////////////////////////////////////////////////////////////////
	void Event::wait()
	{
		{
			boost::mutex::scoped_lock sl(_mtx);

			assert(_waiters.size() < 200);
			if(_isSet)
			{
				if(_autoReset)
				{
					_isSet = false;
				}
				return;
			}

			Fiber *f = Fiber::current();
			assert(f);
			_waiters.push_back(f->shared_from_this());
		}

		Worker::current()->fiberYield();
	}

	//////////////////////////////////////////////////////////////////////////
	bool Event::MultiNotifier::notifyReady(void *key)
	{
		boost::mutex::scoped_lock sl(_mtx);
		if(_readyKey)
		{
			return false;
		}

		_readyKey = key;
		Worker::current()->fiberReady(_initiator);

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	Event::MultiNotifierMarked::MultiNotifierMarked(const MultiNotifierPtr &pmn, void *key)
		: _pmn(pmn)
		, _key(key)
	{
	}



	//////////////////////////////////////////////////////////////////////////
	async::Event *Event::waitAny(async::Event *begin, async::Event *end)
	{
		MultiNotifierPtr pmn(new MultiNotifier);
		pmn->_readyKey = NULL;
		pmn->_initiator = Fiber::current()->shared_from_this();

		//старт
		pmn->_mtx.lock();
		async::Event *iter = begin;
		for(; iter!=end; iter++)
		{
			if(!iter->_impl->mnWait(MultiNotifierMarked(pmn, iter)))
			{
				pmn->_readyKey = iter->_impl.get();
				pmn->_mtx.unlock();
				//готов, откатить установленные
				for(async::Event *iterBack = begin; iterBack!=iter; iterBack++)
				{
					iterBack->_impl->mnCancel(pmn);
				}

				return iter;
			}
		}
		pmn->_mtx.unlock();

		//никто не сработал при старте, ждать
		Worker::current()->fiberYield();

		//один сработал, отменить остальные
		assert(pmn->_readyKey);
		{
			for(async::Event *iterBack = begin; iterBack!=end; iterBack++)
			{
				if(iterBack == pmn->_readyKey)
				{
					continue;
				}
				iterBack->_impl->mnCancel(pmn);
			}
		}

		return (async::Event *)pmn->_readyKey;
	}

	//////////////////////////////////////////////////////////////////////////
	bool Event::mnWait(const MultiNotifierMarked &mn)
	{
		boost::mutex::scoped_lock sl(_mtx);

		if(_isSet)
		{
			if(_autoReset)
			{
				_isSet = false;
			}
			return false;
		}

		_mnWaiters.push_back(mn);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void Event::mnCancel(const MultiNotifierPtr &pmn)
	{
		boost::mutex::scoped_lock sl(_mtx);

		std::vector<MultiNotifierMarked>::iterator iter = _mnWaiters.begin();
		std::vector<MultiNotifierMarked>::iterator end = _mnWaiters.end();
		for(; iter!=end; ++iter)
		{
			if(pmn == iter->_pmn)
			{
				_mnWaiters.erase(iter);
				return;
			}
		}
	}

}}
