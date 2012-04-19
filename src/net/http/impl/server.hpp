#ifndef _NET_HTTP_IMPL_SERVER_HPP_
#define _NET_HTTP_IMPL_SERVER_HPP_

#include "utils/options.hpp"
#include "async/service.hpp"
#include "net/http/server/request.hpp"
#include "net/acceptor.hpp"
#include <boost/signals2.hpp>

namespace net { namespace http { namespace impl
{
	class Server
		: public boost::enable_shared_from_this<Server>
	{
	public:
		static utils::OptionsPtr prepareOptions(const char *prefix);

		Server();
		~Server();

		void init(async::Service asrv, utils::OptionsPtr options);

		boost::signals2::connection connectOnRequest(const boost::function<void(const net::http::server::Request &)> &f);
		void start();
		void stop();

	public:
		boost::uint32_t requestReadGranula() const;

	private:
		std::string _host;
		std::string _port;
		boost::uint32_t _requestReadGranula;

		net::Acceptor _acceptor;
		boost::signals2::connection _connectionOnAccept;

		boost::signals2::signal<void(net::http::server::Request)> _onRequest;

	private:
		void onAccept(boost::system::error_code ec, Channel channel);
	};

	typedef boost::shared_ptr<Server> ServerPtr;

}}}

#endif
