#ifndef _NET_HTTP_SERVER_IMPL_REQUEST_HPP_
#define _NET_HTTP_SERVER_IMPL_REQUEST_HPP_

#include "net/channel.hpp"
#include <boost/shared_ptr.hpp>
#include "net/http/method.hpp"
#include "net/http/version.hpp"
#include "net/http/impl/inputMessage.hpp"
#include "net/http/server/impl/response.hpp"

#include <boost/unordered_map.hpp>

namespace net { namespace http { namespace impl
{
	class Server;
	typedef boost::shared_ptr<Server> ServerPtr;
}}}

namespace net { namespace http { namespace server { namespace impl
{
	class Request;
	typedef boost::shared_ptr<Request> RequestPtr;

	class Response;
	typedef boost::shared_ptr<Response> ResponsePtr;

	///////////////////////////////////////////////////////////////////////////
	class Request
		: public net::http::impl::InputMessage
	{
	public:
		typedef net::http::InputMessage::Segment Segment;

		Request(const net::http::impl::ServerPtr &server, const Channel &channel);
		~Request();
		RequestPtr shared_from_this();
		
		//method uri version
		const EMethod &method_() const;
		const Segment &method() const;

		const Version &version_() const;
		const Segment &version() const;

		const Segment &uri() const;
		const Segment &path() const;
		const Segment &queryString() const;

		ResponsePtr response();
	};
}}}}

#endif
