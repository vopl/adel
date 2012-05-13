#ifndef _HTTP_SERVER_RESPONSE_HPP_
#define _HTTP_SERVER_RESPONSE_HPP_

#include "http/outputMessage.hpp"
#include "http/version.hpp"
#include "http/statusCode.hpp"
#include "http/headerValue.hpp"
#include "http/headerName.hpp"

namespace http { namespace server
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
		friend class http::server::Request;
		Response(ImplPtr impl);
		Response();//without impl

	public:
		~Response();

		bool firstLine(const Version &version, const EStatusCode &statusCode);
		bool firstLine(const EStatusCode &statusCode);
		void setContentLength(size_t size);
		void setContentCompress(int level);
	};
}}

#endif
