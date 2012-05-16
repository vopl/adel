#ifndef _HTTP_CLIENT_REQUEST_HPP_
#define _HTTP_CLIENT_REQUEST_HPP_

#include "http/client/response.hpp"
#include "http/outputMessage.hpp"
#include "http/method.hpp"
#include "http/version.hpp"

namespace http { namespace client
{

	namespace impl
	{
		class Request;
		typedef boost::shared_ptr<Request> RequestPtr;
	}
	///////////////////////////////////////////////////////
	class Request
		: public http::OutputMessage
	{
	protected:
		typedef impl::RequestPtr ImplPtr;
		ImplPtr _impl;

	public:
		Request(const ImplPtr &impl);
		~Request();
	};
}}

#endif
