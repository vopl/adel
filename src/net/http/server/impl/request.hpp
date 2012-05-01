#ifndef _NET_HTTP_SERVER_IMPL_REQUEST_HPP_
#define _NET_HTTP_SERVER_IMPL_REQUEST_HPP_

#include "net/channel.hpp"
#include <boost/shared_ptr.hpp>
#include "net/http/method.hpp"
#include "net/http/version.hpp"
#include "net/http/impl/message.hpp"
#include "net/http/server/impl/response.hpp"

#include <boost/unordered_map.hpp>

namespace net { namespace http { namespace impl
{
	class Server;
	typedef boost::shared_ptr<Server> ServerPtr;
}}}

namespace net { namespace http { namespace server { namespace impl
{
	///////////////////////////////////////////////////////////////////////////
	class Request
		: public net::http::impl::Message
	{
	public:
		typedef net::http::Message::Segment Segment;

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

		const Segment &path_() const;
		const Segment &queryString_() const;

		//headers
		const Segment *header(const HeaderName &name) const;
		const Segment *header(size_t hash) const;
		const Segment *header(const std::string &name) const;
		const Segment *header(const char *namez) const;
		const Segment *header(const char *name, size_t nameSize) const;

	public:
		ResponsePtr response();

	private:
		net::http::impl::ServerPtr	_server;
		Channel						_channel;

		virtual bool obtainMoreBuffers(bool force);

	private:
		Segment	_request_;

		////////////////////////////////
		Segment _requestLine_;
		EMethod _method;
		Segment _method_;
		Segment _uri_;
		Segment _path_;
		Segment _queryString_;
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
		//std::vector<SHeader> _headersVector;

		struct HVHash
			: public std::unary_function<size_t, size_t>
		{
			const std::size_t &operator()(size_t const& v) const
			{
				return v;
			}
		};
		struct HVEqual
			: public std::binary_function<size_t, size_t, bool>
		{
			bool operator()(const size_t &k1, const size_t &k2) const
			{
				return k1 == k2;
			}
		};
		typedef boost::unordered_map<size_t, SHeader, HVHash, HVEqual > TMHeaders;
		TMHeaders _headersMap;

		Segment _body_;

	private:
		ResponsePtr _response;
	};
	typedef boost::shared_ptr<Request> RequestPtr;

}}}}

#endif
