#ifndef _HTTP_IMPL_CLIENT_HPP_
#define _HTTP_IMPL_CLIENT_HPP_

#include "utils/options.hpp"
#include "async/service.hpp"

#include "http/client/impl/request.hpp"

namespace http { namespace impl
{
	///////////////////////////////////////////////////////
	class Client
	{
	public:

		static utils::OptionsPtr prepareOptions(const char *prefix);

		Client();
		~Client();

		void init(async::Service asrv, utils::OptionsPtr options);

		boost::system::error_code connect(
			client::impl::RequestPtr &request,
			const char *host, const char *service, bool useSsl=false);

		boost::system::error_code connect(
			client::impl::RequestPtr &request,
			const char *url);
	};
}}

#endif

