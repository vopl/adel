#ifndef _NET_HTTP_SERVER_IMPL_REQUEST_HPP_
#define _NET_HTTP_SERVER_IMPL_REQUEST_HPP_

#include "net/channel.hpp"
#include <boost/shared_ptr.hpp>
#include "net/http/method.hpp"
#include "net/http/version.hpp"
#include "net/impl/message.hpp"
#include "net/http/server/impl/response.hpp"

namespace net { namespace http { namespace impl
{
	class Server;
	typedef boost::shared_ptr<Server> ServerPtr;
}}}

namespace net { namespace http { namespace server { namespace impl
{
	///////////////////////////////////////////////////////////////////////////
	class Request
		: public net::impl::Message
	{
	public:
		typedef net::Message::Segment Segment;

	public:
		Request(const net::http::impl::ServerPtr &server, const Channel &channel);
		~Request();

		bool readRequestLine();
		bool readHeaders();
		bool readBody();
		bool ignoreBody();

		//method uri version
		const Segment &requestLine_() const;

		const EMethod &method() const;
		const Segment &method_() const;

		const Version &version() const;
		const Segment &version_() const;

		const Segment &uri_() const;

	public:
		ResponsePtr response();

	private:
		net::http::impl::ServerPtr	_server;
		Channel						_channel;

		virtual bool obtainMoreChunks();

	private:
		Segment	_request_;

		////////////////////////////////
		Segment _requestLine_;
		EMethod _method;
		Segment _method_;
		Segment _uri_;
		Version _version;
		Segment _version_;

		////////////////////////////////
		Segment _headers_;
		struct SHeader
		{
			Segment _header_;
			Segment _name_;
			Segment _value_;
		};
		std::vector<SHeader> _headersVector;


		Segment _body_;

	private:
		ResponsePtr _response;
	};
	typedef boost::shared_ptr<Request> RequestPtr;

}}}}

#endif
