#ifndef _HTTP_IMPL_CLIENT_HPP_
#define _HTTP_IMPL_CLIENT_HPP_

#include "utils/options.hpp"
#include "async/service.hpp"
#include "net/connector.hpp"

#include "http/client/impl/request.hpp"

namespace http { namespace impl
{
	///////////////////////////////////////////////////////
	class Client
		: public boost::enable_shared_from_this<Client>
	{
	public:

		static utils::OptionsPtr prepareOptions(const char *prefix);

		Client();
		~Client();

		void init(utils::OptionsPtr options);

		boost::system::error_code connect(
			client::impl::RequestPtr &request,
			const char *host, const char *service, bool useSsl=false);

		boost::system::error_code connectGet(
			client::impl::RequestPtr &request,
			const char *url,
			const Version &version);

		boost::system::error_code get(
			client::impl::ResponsePtr &response,
			const char *url,
			const Version &version);

	public:
		size_t requestWriteGranula() const;
		size_t responseReadGranula() const;

	private:
		size_t _responseReadGranula;
		size_t _requestWriteGranula;
		size_t _timeout;

		net::Connector _connector;

	};
}}

#endif

