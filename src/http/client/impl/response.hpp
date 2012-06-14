#ifndef _HTTP_CLIENT_IMPL_RESPONSE_HPP_
#define _HTTP_CLIENT_IMPL_RESPONSE_HPP_

#include "http/impl/inputMessage.hpp"
#include "http/statusCode.hpp"
#include "http/version.hpp"
#include "http/contentEncoding.hpp"
#include "http/transferEncoding.hpp"
#include "http/headerValue.hpp"
#include "http/headerName.hpp"

namespace http { namespace impl
{
	class Client;
	typedef boost::shared_ptr<Client> ClientPtr;
}}

namespace http { namespace client { namespace impl
{
	class Request;

	class Response;
	typedef boost::shared_ptr<Response> ResponsePtr;
	////////////////////////////////////////////////////////
	class Response
		: public http::impl::InputMessage
	{
	public:
		Response(const http::impl::ClientPtr &client, const net::Channel &channel, Request *request);
		~Response();

		EStatusCode status() const;
		const Version &version() const;

	private:
		EStatusCode _status;

	private:
		virtual boost::system::error_code readFirstLine();

	};

}}}

#endif
