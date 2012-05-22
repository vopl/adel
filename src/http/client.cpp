#include "pch.hpp"
#include "http/client.hpp"
#include "http/impl/client.hpp"

namespace http
{

	///////////////////////////////////////////////////////////////////////
	utils::OptionsPtr Client::prepareOptions(const char *prefix)
	{
		return http::impl::Client::prepareOptions(prefix);
	}

	///////////////////////////////////////////////////////////////////////
	Client::Client(utils::OptionsPtr options)
		: _impl(new http::impl::Client)
	{
		_impl->init(options);
	}


	///////////////////////////////////////////////////////////////////////
	Client::~Client()
	{

	}

	///////////////////////////////////////////////////////////////////////
	boost::system::error_code Client::connect(
		client::Request &request,
		const char *host, const char *service, bool useSsl)
	{
		http::client::impl::RequestPtr requestImpl;
		boost::system::error_code ec = _impl->connect(requestImpl, host, service, useSsl);
		if(requestImpl)
		{
			request = client::Request(requestImpl);
		}
		return ec;
	}

	///////////////////////////////////////////////////////////////////////
	boost::system::error_code Client::connectGet(
		client::Request &request,
		const char *url,
		const Version &version)
	{
		http::client::impl::RequestPtr requestImpl;
		boost::system::error_code ec = _impl->connectGet(requestImpl, url, version);
		if(requestImpl)
		{
			request = client::Request(requestImpl);
		}
		return ec;
	}

}
