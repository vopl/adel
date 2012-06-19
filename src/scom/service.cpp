#include "pch.hpp"
#include "scom/service.hpp"
#include "scom/impl/service.hpp"

namespace scom
{
	///////////////////////////////////////////////////////////////////
	utils::OptionsPtr Service::prepareOptions(const char *prefix)
	{
		return scom::impl::Service::prepareOptions(prefix);
	}

	///////////////////////////////////////////////////////////////////
	Service::Service(utils::OptionsPtr optionsPtr)
		: _impl(new impl::Service(optionsPtr))
	{
	}

	///////////////////////////////////////////////////////////////////
	Service::~Service()
	{
		_impl.reset();
	}

	///////////////////////////////////////////////////////////////////
	void Service::start()
	{
		return _impl->start();
	}

	///////////////////////////////////////////////////////////////////
	void Service::stop()
	{
		return _impl->stop();
	}

	///////////////////////////////////////////////////////////////////
	EError Service::create(
		Auth &auth,
		std::string password)
	{
		return _impl->create(auth, password);
	}

	///////////////////////////////////////////////////////////////////
	EError Service::ping(
		Status &status,
		const Auth &auth)
	{
		return _impl->ping(status, auth);
	}

	///////////////////////////////////////////////////////////////////
	EError Service::setup(
		const Auth &auth,
		const std::vector<PageRule> &rules)
	{
		return _impl->setup(auth, rules);
	}

	///////////////////////////////////////////////////////////////////
	EError Service::start(
		const Auth &auth)
	{
		return _impl->start(auth);
	}

	///////////////////////////////////////////////////////////////////
	EError Service::stop(
		const Auth &auth)
	{
		return _impl->stop(auth);
	}

}
