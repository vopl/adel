#ifndef _NET_HTTP_IMPL_SERVER_HPP_
#define _NET_HTTP_IMPL_SERVER_HPP_

#include "utils/options.hpp"
#include "async/service.hpp"
#include "net/http/server/request.hpp"
#include <boost/signals2.hpp>

namespace net { namespace http { namespace impl
{
	class Server
	{
	public:
		static utils::OptionsPtr prepareOptions(const char *prefix);

		Server(async::Service asrv, utils::OptionsPtr options);
		~Server();

		boost::signals2::connection connectOnRequest(const boost::function<void(const net::http::server::Request &)> &f);
		void start();
		void stop();
	};

}}}

#endif
