#ifndef _HTTP_SERVER_IMPL_REQUEST_HPP_
#define _HTTP_SERVER_IMPL_REQUEST_HPP_

#include "net/channel.hpp"
#include <boost/shared_ptr.hpp>
#include "http/method.hpp"
#include "http/version.hpp"
#include "http/impl/inputMessage.hpp"
#include "http/server/impl/response.hpp"

#include <boost/unordered_map.hpp>

namespace http { namespace impl
{
	class Server;
	typedef boost::shared_ptr<Server> ServerPtr;
}}

namespace http { namespace server { namespace impl
{
	class Request;
	typedef boost::shared_ptr<Request> RequestPtr;

	class Response;
	typedef boost::shared_ptr<Response> ResponsePtr;

	///////////////////////////////////////////////////////////////////////////
	class Request
		: public http::impl::InputMessage
	{
	public:
		typedef http::InputMessage::Segment Segment;

		Request(const http::impl::ServerPtr &server, const net::Channel &channel);
		~Request();

		bool readFirstLine();

		RequestPtr shared_from_this();
		
		//method uri version
		const EMethod &method_() const;
		const Segment &method() const;

		const Version &version_() const;
		const Segment &version() const;

		const Segment &uri() const;
		const Segment &path() const;
		const Segment &queryString() const;

		const ResponsePtr &response();

		void reinit();

	private:
		EMethod	_method_;
		Segment	_method;

		Version	_version_;
		Segment	_version;

		Segment	_uri;
		Segment	_path;
		Segment	_queryString;

		http::impl::ServerPtr	_server;
		net::Channel			_channel;
		ResponsePtr				_response;
	};
}}}

#endif
