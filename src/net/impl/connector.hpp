#ifndef _NET_IMPL_CONNECTOR_HPP_
#define _NET_IMPL_CONNECTOR_HPP_

#include "net/connector.hpp"
#include "async/service.hpp"
#include "net/impl/channel.hpp"

namespace net { namespace impl
{
	using namespace async;

	//////////////////////////////////////////////////////////////////////////
	class Connector;
	typedef boost::shared_ptr<Connector> ConnectorPtr;

	class Connector
		: public boost::enable_shared_from_this<Connector>
	{
		boost::mutex	_mtx;
		TSslContextPtr	_sslContext;

		std::string onSslPassword();

		void connect_f(Future2<boost::system::error_code, net::Channel> res, const std::string &host, const std::string &service, bool useSsl);

	public:
		Connector();
		~Connector();

		Future2<boost::system::error_code, net::Channel> connect(const char *host, const char *service, bool useSsl);
	};
}}

#endif
