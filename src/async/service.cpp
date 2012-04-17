#include "pch.hpp"
#include "async/service.hpp"
#include "async/impl/service.hpp"

namespace async
{
	////////////////////////////////////////////////////////////////
	Service::Service()
		: _impl(new impl::Service)
	{
	}

	////////////////////////////////////////////////////////////////
	Service::Service(impl::ServicePtr impl)
		: _impl(impl)
	{
	}

	////////////////////////////////////////////////////////////////
	Service::~Service()
	{
	}

	////////////////////////////////////////////////////////////////
	boost::signals2::connection Service::connectOnStart(const boost::function<void()> &f)
	{
		return _impl->connectOnStart(f);
	}

	////////////////////////////////////////////////////////////////
	boost::signals2::connection Service::connectOnStop(const boost::function<void()> &f)
	{
		return _impl->connectOnStop(f);
	}

	////////////////////////////////////////////////////////////////
	boost::signals2::connection Service::connectOnWorkerStart(const boost::function<void()> &f)
	{
		return _impl->connectOnWorkerStart(f);
	}

	////////////////////////////////////////////////////////////////
	boost::signals2::connection Service::connectOnWorkerStop(const boost::function<void()> &f)
	{
		return _impl->connectOnWorkerStop(f);
	}

	////////////////////////////////////////////////////////////////
	void Service::setupFibers(size_t stackSize, size_t maxAmount)
	{
		return _impl->setupFibers(stackSize, maxAmount);
	}

	////////////////////////////////////////////////////////////////
	void Service::start(size_t numThreads)
	{
		return _impl->start(numThreads);
	}

	////////////////////////////////////////////////////////////////
	void Service::balance(size_t numThreads)
	{
		return _impl->balance(numThreads);
	}

	////////////////////////////////////////////////////////////////
	void Service::stop()
	{
		return _impl->stop();
	}

	////////////////////////////////////////////////////////////////
	void Service::spawn(const boost::function<void ()> &code)
	{
		return _impl->spawn(code);
	}

	////////////////////////////////////////////////////////////////
	Future<boost::system::error_code> Service::timeout(size_t millisec)
	{
		return _impl->timeout(millisec);
	}

	////////////////////////////////////////////////////////////////
	void Service::cancelAllTimeouts()
	{
		return _impl->cancelAllTimeouts();
	}

	////////////////////////////////////////////////////////////////
	boost::asio::io_service &Service::io()
	{
		return _impl->io();
	}

	////////////////////////////////////////////////////////////////
	bool Service::setAsGlobal(bool force)
	{
		return _impl->setAsGlobal(force);
	}

}
