#include "pch.hpp"
#include "http/server.hpp"
#include "http/impl/server.hpp"
#include "http/server/log.hpp"

namespace http
{
	///////////////////////////////////////////////////////////////////////////
	utils::OptionsPtr Server::prepareOptions(const char *prefix)
	{
		return http::impl::Server::prepareOptions(prefix);
	}

	///////////////////////////////////////////////////////////////////////////
	Server::Server(async::Service asrv, utils::OptionsPtr options)
		: _impl(new http::impl::Server)
	{
		_impl->init(asrv, options);
	}

	///////////////////////////////////////////////////////////////////////////
	Server::~Server()
	{
	}

	///////////////////////////////////////////////////////////////////////////
	boost::signals2::connection Server::onRequest(const boost::function<void(const server::Request &)> &f)
	{
		return _impl->onRequest(f);
	}

	///////////////////////////////////////////////////////////////////////////
	void Server::start()
	{
		return _impl->start();
	}

	///////////////////////////////////////////////////////////////////////////
	void Server::stop()
	{
		return _impl->stop();
	}

}
