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
		ResponsePtr shared_from_this()
		{
			return boost::static_pointer_cast<Response>(http::impl::OutputMessage::shared_from_this());
		}

	public:
		Response(const http::impl::ServerPtr &server, const net::Channel &channel, Request *request);
		~Response();

		bool bodyFlush();


		bool firstLine(const Version &version, const EStatusCode &statusCode);
		bool firstLine(const EStatusCode &statusCode);
		void setContentLength(size_t size);
		void setContentCompress(int level);

	private:
		http::impl::ServerPtr	_server;
		Request					*_request;

	private:
		Version				_version;

		size_t				_contentLength;
		static const size_t	_unknownContentLength = (size_t)-1;
		bool				_chunked;
		bool				_keepAlive;
		EContentEncoding	_contentEncoding;
		int					_contentEncodingCompressLevel;

	private:
		virtual bool writeSystemHeaders();
		virtual bool setupBodyFilters();
	};

}}}

#endif
