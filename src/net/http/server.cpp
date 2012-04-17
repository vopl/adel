#include "pch.hpp"
#include "net/http/server.hpp"
#include "net/http/impl/server.hpp"
#include "net/http/server/log.hpp"

namespace net { namespace http
{
	///////////////////////////////////////////////////////////////////////////
	utils::OptionsPtr Server::prepareOptions(const char *prefix)
	{
		return net::http::impl::Server::prepareOptions(prefix);
	}

	///////////////////////////////////////////////////////////////////////////
	Server::Server(async::Service asrv, utils::OptionsPtr options)
		: _impl(new net::http::impl::Server)
	{
		_impl->init(asrv, options);
	}

	///////////////////////////////////////////////////////////////////////////
	Server::~Server()
	{
	}

	///////////////////////////////////////////////////////////////////////////
	boost::signals2::connection Server::connectOnRequest(const boost::function<void(const server::Request &)> &f)
	{
		return _impl->connectOnRequest(f);
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

}}
