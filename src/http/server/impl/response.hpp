#ifndef _HTTP_SERVER_IMPL_RESPONSE_HPP_
#define _HTTP_SERVER_IMPL_RESPONSE_HPP_

#include "http/impl/outputMessage.hpp"
#include "http/statusCode.hpp"
#include "http/version.hpp"
#include "http/contentEncoding.hpp"
#include "http/transferEncoding.hpp"
#include "http/headerValue.hpp"
#include "http/headerName.hpp"

namespace http { namespace impl
{
	class Server;
	typedef boost::shared_ptr<Server> ServerPtr;
}}

namespace http { namespace server { namespace impl
{
	class Request;

	class Response;
	typedef boost::shared_ptr<Response> ResponsePtr;
	////////////////////////////////////////////////////////
	class Response
		: public http::impl::OutputMessage
	{
	public:
		ResponsePtr shared_from_this();

	public:
		Response(const http::impl::ServerPtr &server, const net::Channel &channel, Request *request);
		~Response();

		boost::system::error_code bodyFlush();


		boost::system::error_code firstLine(const Version &version, const EStatusCode &statusCode);
		boost::system::error_code firstLine(const EStatusCode &statusCode);

	private:
		http::impl::ServerPtr	_server;
		Request					*_request;

	private:
		virtual boost::system::error_code writeSystemHeaders();
		virtual boost::system::error_code setupBodyFilters();
	};

}}}

#endif
