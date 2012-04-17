#ifndef _ASYNC_IMPL_WORKER_HPP_
#define _ASYNC_IMPL_WORKER_HPP_

#include "async/impl/fiber.hpp"
#include "async/impl/fiberRoot.hpp"
#include "async/impl/fiberPool.hpp"

#include <boost/thread.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <set>
#include <deque>

namespace async { namespace impl
{
	class Service;
	typedef boost::shared_ptr<Service> ServicePtr;

	//////////////////////////////////////////////////////////////////////////
	class Worker
		: public boost::enable_shared_from_this<Worker>
	{
		ServicePtr		_service;
		boost::thread	_thread;
		volatile bool	_stop;

	private:
		typedef boost::function<void()> TTask;

		//головной фибер, в нем исполняется цикл выкачивания событий проактора
		FiberRootPtr	_fiberRoot;

		//шареный буфер с фиберами, в нем простаивающие без кода и готовые к исполнению
		FiberPoolPtr		_fiberPool;

		static ThreadLocalStorage<Worker *>
							_current;

	private:
		void threadProc();

	private:
		bool processReadyFibers();

	public://для фиберов

		//не умный потому что этот метод будет вызываться из рабочей процедуры фибера и она не должна делать RAII чтобы не унести с собой объект фибера при разрушении
		void fiberExecuted(Fiber *fiber);
		void fiberReady(FiberPtr fiber);
		bool fiberReadyIfWait(FiberPtr fiber);
		void fiberYield();

	public://для врапера asio
		void exec(const TTask &);
		void yield();

	public:
		Worker(ServicePtr service, FiberPoolPtr fiberPool);
		~Worker();

		const ServicePtr &service();

		static Worker *current();
	};
	typedef boost::shared_ptr<Worker> WorkerPtr;
}}

#endif
