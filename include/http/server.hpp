#ifndef _HTTP_SERVER_HPP_
#define _HTTP_SERVER_HPP_

#include "utils/options.hpp"
#include "async/service.hpp"

#include "http/server/request.hpp"

namespace http
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

		boost::signals2::connection onRequest(const boost::function<void(const server::Request &)> &f);
		void start();
		void stop();

	};
}

#endif
