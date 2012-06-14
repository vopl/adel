#ifndef _HTTP_CLIENT_RESPONSE_HPP_
#define _HTTP_CLIENT_RESPONSE_HPP_

#include "http/inputMessage.hpp"
#include "http/version.hpp"
#include "http/statusCode.hpp"
#include "http/headerValue.hpp"
#include "http/headerName.hpp"

namespace http { namespace client
{

	namespace impl
	{
		class Response;
		typedef boost::shared_ptr<Response> ResponsePtr;
	}
	class Request;
	///////////////////////////////////////////////////////
	class Response
		: public InputMessage
	{
	protected:
		typedef impl::ResponsePtr ImplPtr;
		ImplPtr _impl;

	public:
		Response();
		Response(const ImplPtr &impl);
		~Response();

		EStatusCode status() const;
		const Version &version() const;

	};
}}

#endif
