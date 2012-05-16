#ifndef _HTTP_IMPL_SERVER_HPP_
#define _HTTP_IMPL_SERVER_HPP_

#include "utils/options.hpp"
#include "async/service.hpp"
#include "http/server/request.hpp"
#include "net/acceptor.hpp"
#include <boost/signals2.hpp>

namespace http { namespace impl
{
	class Server
		: public boost::enable_shared_from_this<Server>
	{
	public:
		static utils::OptionsPtr prepareOptions(const char *prefix);

		Server();
		~Server();

		void init(async::Service asrv, utils::OptionsPtr options);

		boost::signals2::connection onRequest(const boost::function<void(const http::server::Request &)> &f);
		void start();
		void stop();

	public:
		size_t requestReadGranula() const;
		size_t responseWriteGranula() const;

	public:
		void onRequest(http::server::impl::RequestPtr requestImpl);
	private:
		void onRequest_f(http::server::impl::RequestPtr requestImpl);

	private:
		async::Service _asrv;
		std::string _host;
		std::string _port;
		size_t _requestReadGranula;
		size_t _responseWriteGranula;
		size_t _timeout;

		net::Acceptor _acceptor;
		boost::signals2::connection _connectionOnAccept;

		boost::signals2::signal<void(http::server::Request)> _onRequest;

	private:
		void onAccept(boost::system::error_code ec, net::Channel channel);
	};

	typedef boost::shared_ptr<Server> ServerPtr;

}}

#endif
