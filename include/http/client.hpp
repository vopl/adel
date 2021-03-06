#ifndef _HTTP_CLIENT_HPP_
#define _HTTP_CLIENT_HPP_

#include "utils/options.hpp"
#include "async/service.hpp"

#include "http/client/request.hpp"
#include "http/version.hpp"

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

		Client(utils::OptionsPtr options);
		~Client();

		boost::system::error_code connect(
			client::Request &request,
			const char *host, const char *service, bool useSsl=false);

		boost::system::error_code connectGet(
			client::Request &request,
			const char *url,
			const Version &version = Version(1,1));

		boost::system::error_code get(
			client::Response &response,
			const char *url,
			const Version &version = Version(1,1));

	};
}

#endif

