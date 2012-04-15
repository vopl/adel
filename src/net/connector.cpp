#include "pch.hpp"
#include "net/connector.hpp"
#include "net/impl/connector.hpp"

namespace net
{

	//////////////////////////////////////////////////////////////////////////
	Connector::Connector()
		: _impl(new impl::Connector)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	Connector::~Connector()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	async::Future2<boost::system::error_code, Channel> Connector::connect(const char *host, const char *service, bool useSsl)
	{
		return _impl->connect(host, service, useSsl);
	}
}
