#include "pch.hpp"
#include "async/freeFunctions.hpp"
#include "async/impl/service.hpp"
#include "async/exception.hpp"
#include "async/log.hpp"

namespace async
{
	//////////////////////////////////////////////////////////////////////////
	void spawn(const boost::function<void ()> &code)
	{
		impl::Service *service = impl::Service::current();
		if(!service)
		{
			assert(0);
			ELOG("call spawn with empty service");
			throw exception("call spawn with empty service");
			return;
		}

		return service->spawn(code);
	}

	//////////////////////////////////////////////////////////////////////////
	Future<boost::system::error_code> timeout(size_t millisec)
	{
		impl::Service *service = impl::Service::current();
		if(!service)
		{
			assert(0);
			ELOG("call timeout with empty service");
			throw exception("call timeout with empty service");

			Future<boost::system::error_code> stub;
			return stub;
		}

		return service->timeout(millisec);
	}

	//////////////////////////////////////////////////////////////////////////
	void exec(const boost::function<void ()> &code)
	{
		impl::Worker *worker = impl::Worker::current();
		if(!worker)
		{
			assert(0);
			ELOG("exec request with no current worker");
			throw exception("exec request with no current worker");
			return;
		}

		return worker->exec(code);
	}

	//////////////////////////////////////////////////////////////////////////
	void yield()
	{
		impl::Worker *worker = impl::Worker::current();
		if(!worker)
		{
			assert(0);
			ELOG("yield request with no current worker");
			throw exception("yield request with no current worker");
			return;
		}

		return worker->yield();
	}

	//////////////////////////////////////////////////////////////////////////
	boost::asio::io_service &io()
	{
		impl::Service *service = impl::Service::current();
		if(!service)
		{
			assert(0);
			ELOG("io request with no current service");
			throw exception("io request with no current service");
			static boost::asio::io_service stub;
			return stub;
		}

		return service->io();
	}

	//////////////////////////////////////////////////////////////////////////
	bool serviceExists()
	{
		return impl::Service::current()?true:false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool workerExists()
	{
		return impl::Worker::current()?true:false;
	}

	//////////////////////////////////////////////////////////////////////////
	Service service()
	{
		impl::Service *service = impl::Service::current();
		if(!service)
		{
			ELOG("service request with no current service");
			throw exception("service request with no current service");
			return Service();
		}

		return Service(service->shared_from_this());
	}

}
