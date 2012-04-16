#ifndef _NET_HTTP_SERVER_HPP_
#define _NET_HTTP_SERVER_HPP_

#include "utils/options.hpp"
#include "async/service.hpp"

#include "net/http/server/request.hpp"

namespace net { namespace http
{

	namespace impl
	{
		class Server;
		typedef boost::shared_ptr<Server> ServerPtr;
	}
	///////////////////////////////////////////////////////
	class Server
	{
	protected:
		typedef impl::ServerPtr ImplPtr;
		ImplPtr _impl;

	public:

		static utils::OptionsPtr prepareOptions(const char *prefix);

		Server(async::Service asrv, utils::OptionsPtr options);
		~Server();

		boost::signals2::connection connectOnRequest(const boost::function<void(const server::Request &)> &f);
		void start();
		void stop();

	};
}}

#endif
