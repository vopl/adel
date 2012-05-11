#ifndef _NET_HTTP_SERVER_IMPL_RESPONSE_HPP_
#define _NET_HTTP_SERVER_IMPL_RESPONSE_HPP_

#include "net/http/impl/outputMessage.hpp"
#include "net/http/statusCode.hpp"
#include "net/http/version.hpp"
#include "net/http/contentEncoding.hpp"
#include "net/http/transferEncoding.hpp"
#include "net/http/headerValue.hpp"
#include "net/http/headerName.hpp"

namespace net { namespace http { namespace impl
{
	class Server;
	typedef boost::shared_ptr<Server> ServerPtr;
}}}

namespace net { namespace http { namespace server { namespace impl
{
	class Request;

	class Response;
	typedef boost::shared_ptr<Response> ResponsePtr;
	////////////////////////////////////////////////////////
	class Response
		: public net::http::impl::OutputMessage
	{
	public:
		ResponsePtr shared_from_this()
		{
			return boost::static_pointer_cast<Response>(net::http::impl::OutputMessage::shared_from_this());
		}

	public:
		Response(const net::http::impl::ServerPtr &server, const Channel &channel, Request *request);
		~Response();

		bool		bodyFlush();


		bool firstLine(const Version &version, const EStatusCode &statusCode);
		bool firstLine(const EStatusCode &statusCode);
		void setContentLength(size_t size);
		void setContentCompress(int level);

	private:
		net::http::impl::ServerPtr	_server;
		Request						*_request;

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

}}}}

#endif
