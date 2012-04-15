#ifndef _ASYNC_SERVICE_HPP_
#define _ASYNC_SERVICE_HPP_

#include "async/exception.hpp"
#include "async/event.hpp"
#include "async/future.hpp"

#include <boost/signals2.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

namespace async
{

	namespace impl
	{
		class Service;
		typedef boost::shared_ptr<Service> ServicePtr;
	}

	/////////////////////////////////////////////////////////////////////////
	class Service
	{
	public:
		Service();
		Service(impl::ServicePtr impl);
		virtual ~Service();

		boost::signals2::connection connectOnStart(const boost::function<void()> &f);
		boost::signals2::connection connectOnStop(const boost::function<void()> &f);
		boost::signals2::connection connectOnWorkerStart(const boost::function<void()> &f);
		boost::signals2::connection connectOnWorkerStop(const boost::function<void()> &f);

		void setupFibers(size_t stackSize, size_t maxAmount);
		void start(size_t numThreads);
		void balance(size_t numThreads);
		void stop();


		void spawn(const boost::function<void ()> &code);
		Future<boost::system::error_code> timeout(size_t millisec);
		void cancelAllTimeouts();
		boost::asio::io_service &io();
		bool setAsGlobal(bool force);

	private:

		impl::ServicePtr _impl;
	};
}

#include "async/freeFunctions.hpp"

#endif
