#ifndef _HTTP_CLIENT_HPP_
#define _HTTP_CLIENT_HPP_

#include "utils/options.hpp"
#include "async/service.hpp"

#include "http/client/request.hpp"

namespace http
{

	namespace impl
	{
		class Client;
		typedef boost::shared_ptr<Client> ClientPtr;
	}
	///////////////////////////////////////////////////////
	class Client
	{
	protected:
		typedef impl::ClientPtr ImplPtr;
		ImplPtr _impl;

	public:

		static utils::OptionsPtr prepareOptions(const char *prefix);

		Client(async::Service asrv, utils::OptionsPtr options);
		~Client();

		boost::system::error_code connect(
			client::Request &request,
			const char *host, const char *service, bool useSsl=false);

		boost::system::error_code connect(
			client::Request &request,
			const char *url);
	};
}

#endif

