#ifndef _NET_HTTP_SERVER_REQUEST_HPP_
#define _NET_HTTP_SERVER_REQUEST_HPP_

#include "net/http/server/response.hpp"
#include "net/channel.hpp"
#include "net/http/inputMessage.hpp"
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
		: public net::http::InputMessage
	{
	protected:
		typedef impl::RequestPtr ImplPtr;
		ImplPtr _impl;

	protected:
		Request();

	public:
		~Request();

		//method uri version
		const EMethod &method_() const;
		const Segment &method() const;

		const Version &version_() const;
		const Segment &version() const;

		const Segment &uri() const;
		const Segment &path() const;
		const Segment &queryString() const;

		//params uri
		//params body

		//params all

	public:
		Response response();
	};
}}}

#endif
