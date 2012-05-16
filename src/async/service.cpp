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
	boost::signals2::connection Service::onStart(const boost::function<void()> &f)
	{
		return _impl->onStart(f);
	}

	////////////////////////////////////////////////////////////////
	boost::signals2::connection Service::onStop(const boost::function<void()> &f)
	{
		return _impl->onStop(f);
	}

	////////////////////////////////////////////////////////////////
	boost::signals2::connection Service::onWorkerStart(const boost::function<void()> &f)
	{
		return _impl->onWorkerStart(f);
	}

	////////////////////////////////////////////////////////////////
	boost::signals2::connection Service::onWorkerStop(const boost::function<void()> &f)
	{
		return _impl->onWorkerStop(f);
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
