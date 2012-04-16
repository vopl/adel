#ifndef _NET_HTTP_SERVER_REQUEST_HPP_
#define _NET_HTTP_SERVER_REQUEST_HPP_

#include "net/http/server/response.hpp"

namespace net { namespace http { namespace server
{

	namespace impl
	{
		class Request;
		typedef boost::shared_ptr<Request> RequestPtr;
	}
	///////////////////////////////////////////////////////
	class Request
	{
	protected:
		typedef impl::RequestPtr ImplPtr;
		ImplPtr _impl;

	private:
		Request();

	public:
		~Request();

		//method uri version

		//headers

		//	general header
		//	request header
		//	entity header
		//	message header

		//body

		//params uri
		//params body

		//params all

	};
}}}

#endif
