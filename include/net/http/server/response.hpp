#ifndef _NET_HTTP_SERVER_RESPONSE_HPP_
#define _NET_HTTP_SERVER_RESPONSE_HPP_

#include "net/http/outputMessage.hpp"
#include "net/http/version.hpp"
#include "net/http/statusCode.hpp"
#include "net/http/headerValue.hpp"
#include "net/http/headerName.hpp"

namespace net { namespace http { namespace server
{

	namespace impl
	{
		class Response;
		typedef boost::shared_ptr<Response> ResponsePtr;
	}
	class Request;
	///////////////////////////////////////////////////////
	class Response
		: public OutputMessage
	{
	protected:
		typedef impl::ResponsePtr ImplPtr;
		ImplPtr _impl;

	private:
		friend class net::http::server::Request;
		Response(ImplPtr impl);
		Response();//without impl

	public:
		~Response();

		bool firstLine(const Version &version, const EStatusCode &statusCode);
		bool firstLine(const EStatusCode &statusCode);
		void setContentLength(size_t size);
		void setContentCompress(int level);
	};
}}}

#endif
