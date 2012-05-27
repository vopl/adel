#include "pch.hpp"
#include "async/impl/mutex.hpp"
#include "async/impl/worker.hpp"
#include "async/exception.hpp"
#include "async/log.hpp"

#include <boost/foreach.hpp>

namespace async { namespace impl
{
	//////////////////////////////////////////////////////////////////////////
	Mutex::Mutex()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	Mutex::~Mutex()
	{
		boost::mutex::scoped_lock scopeLock(_mtx);
		assert(_owners.empty());

		BOOST_FOREACH(FiberPtr &fiber, _owners)
		{
			Worker::current()->fiberReady(fiber);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool Mutex::tryLock()
	{
		boost::mutex::scoped_lock scopeLock(_mtx);

		if(_owners.empty())
		{
			FiberPtr current = Fiber::current()->shared_from_this();
			_owners.push_back(current);
			return true;
		}

		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void Mutex::lock()
	{
		{
			boost::mutex::scoped_lock scopeLock(_mtx);

			//assert(_owners.size() < 2000);

			FiberPtr current = Fiber::current()->shared_from_this();
			_owners.push_back(current);
			if(_owners.size() == 1)
			{
				return;
			}
		}
		Worker::current()->fiberYield();
	}

	//////////////////////////////////////////////////////////////////////////
	bool Mutex::isLocked()
	{
		boost::mutex::scoped_lock scopeLock(_mtx);
		return !_owners.empty();
	}

	//////////////////////////////////////////////////////////////////////////
	void Mutex::unlock()
	{
		boost::mutex::scoped_lock scopeLock(_mtx);
		if(_owners.empty())
		{
			ELOG("unlock mutex which is not locked");
			throw exception("unlock mutex which is not locked");
			return;
		}

		//не важно, что текущий фибер не является владельцем
// 		if(_owners.front().get() != FiberImpl::current())
// 		{
// 			ELOG("unlockStrict mutex which from alien fiber");
// 			throw exception("unlockStrict mutex which from alien fiber");
// 			return;
// 		}


		_owners.erase(_owners.begin());

		if(!_owners.empty())
		{
#ifdef _DEBUG
			bool fired = Worker::current()->fiberReadyIfWait(_owners.front());
			if(!fired)
			{
				assert("unlockStrict mutex for non-wait fiber");
			}
#else
			Worker::current()->fiberReady(_owners.front());
#endif
		}
	}
}}
