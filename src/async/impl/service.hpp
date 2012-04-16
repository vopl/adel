#ifndef _ASYNC_IMPL_SERVICE_HPP_
#define _ASYNC_IMPL_SERVICE_HPP_

#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/signals2.hpp>

#include "async/impl/worker.hpp"
#include "async/future.hpp"

namespace async { namespace impl
{
	//////////////////////////////////////////////////////////////////////////
	class Service
		: public boost::enable_shared_from_this<Service>
	{
		boost::asio::io_service		_io;
		boost::shared_ptr<boost::asio::io_service::work>
									_work;

		std::vector<WorkerPtr>		_workers;
		FiberPoolPtr				_fiberPool;

		boost::signals2::signal<void ()> _onStart;
		boost::signals2::signal<void ()> _onStop;

		boost::signals2::signal<void ()> _onWorkerStart;
		boost::signals2::signal<void ()> _onWorkerStop;

		boost::mutex				_mtx;

		static ThreadLocalStorage<Service *>
									_current;
		static Service				*_global;

		size_t _stackSize;
		size_t _maxFibers;

	private:
		typedef boost::shared_ptr<boost::asio::deadline_timer> TTimerPtr;
		typedef std::set<TTimerPtr> TSTimers;
		TSTimers	_timers;
		boost::mutex		_mtxTimers;
		void onTimer(const TTimerPtr &timer, const boost::system::error_code &ec, Future<boost::system::error_code> res);

	public:
		Service();
		~Service();

		boost::signals2::connection connectOnStart(const boost::function<void()> &f);
		boost::signals2::connection connectOnStop(const boost::function<void()> &f);
		boost::signals2::connection connectOnWorkerStart(const boost::function<void()> &f);
		boost::signals2::connection connectOnWorkerStop(const boost::function<void()> &f);

		void setupFibers(size_t stackSize, size_t maxAmount);
		void start(size_t numThreads);
		void balance(size_t numThreads);
		void stop();

		size_t getStackSize();
		size_t getMaxFibers();


	public:
		void onStart();
		void onStop();
		void onThreadStart();
		void onThreadStop();

	public:
		void spawn(const boost::function<void ()> &code);
		Future<boost::system::error_code> timeout(size_t millisec);
		void cancelAllTimeouts();
		boost::asio::io_service &io();
		bool setAsGlobal(bool force);
		static Service *current();
	};
}}
#endif
