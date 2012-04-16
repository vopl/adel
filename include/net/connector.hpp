#ifndef _NET_CONNECTOR_HPP_
#define _NET_CONNECTOR_HPP_

#include "net/channel.hpp"

namespace net
{
	//////////////////////////////////////////////////////////////////////////
	namespace impl
	{
		class Connector;
		typedef boost::shared_ptr<Connector> ConnectorPtr;

	}

	//////////////////////////////////////////////////////////////////////////
	class Connector
	{
	protected:
		typedef impl::ConnectorPtr ImplPtr;
		ImplPtr	_impl;

	public:
		Connector();
		~Connector();
		async::Future2<boost::system::error_code, Channel> connect(const char *host, const char *service, bool useSsl=false);
	};
}
#endif
