#ifndef _NET_HTTP_SERVER_REQUEST_HPP_
#define _NET_HTTP_SERVER_REQUEST_HPP_

#include "net/http/server/response.hpp"
#include "net/channel.hpp"
#include "net/message.hpp"
#include "net/http/method.hpp"
#include "net/http/version.hpp"

namespace net { namespace http { namespace server
{

	namespace impl
	{
		class Request;
		typedef boost::shared_ptr<Request> RequestPtr;
	}
	///////////////////////////////////////////////////////
	class Request
		: public net::Message
	{
	protected:
		typedef impl::RequestPtr ImplPtr;
		ImplPtr _impl;

	protected:
		Request();

	public:
		~Request();

		bool readRequestLine();
		bool readHeaders();
		bool readBody();
		bool ignoreBody();

		const Segment &requestLine_() const;

		//method uri version
		const EMethod &method() const;
		const Segment &method_() const;

		const Version &version() const;
		const Segment &version_() const;

		const Segment &uri_() const;

		//headers

		//	general header
		//	request header
		//	entity header
		//	message header

		//body

		//params uri
		//params body

		//params all

	public:
		Response response();
	};
}}}

#endif
