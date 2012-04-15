#ifndef _ASYNC_MUTEX_HPP_
#define _ASYNC_MUTEX_HPP_

#include <boost/shared_ptr.hpp>


//////////////////////////////////////////////////////////////////////////
namespace async
{
	namespace impl
	{
		class Mutex;
		typedef boost::shared_ptr<Mutex> MutexPtr;
	}

	//////////////////////////////////////////////////////////////////////////
	class Mutex
	{
		impl::MutexPtr	_impl;

	public:
		Mutex();

		bool tryLock();
		void lock();
		bool isLocked();
		void unlock();

	public:
		struct ScopedLock
		{
		public:
			ScopedLock(Mutex &mutex)
				: _mutex(mutex)
			{
				_mutex.lock();
			}

			~ScopedLock()
			{
				_mutex.unlock();
			}

		private:
			Mutex &_mutex;
		};

	};
}

#endif
