#ifndef _ASYNC_IMPL_MUTEX_HPP_
#define _ASYNC_IMPL_MUTEX_HPP_

#include "async/impl/mutex.hpp"
#include "async/impl/fiber.hpp"

#include <vector>

namespace async { namespace impl
{
	class Mutex
	{
		boost::mutex			_mtx;
		std::vector<FiberPtr>	_owners;
	public:
		Mutex();
		~Mutex();

		bool tryLock();
		void lock();
		bool isLocked();
		void unlock();
	};
	typedef boost::shared_ptr<Mutex> MutexPtr;
}}
#endif
