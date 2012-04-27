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
		const Segment &path_() const;
		const Segment &queryString_() const;

		//headers
		const Segment *header(const HeaderName &name) const;
		const Segment *header(size_t hash) const;
		const Segment *header(const std::string &name) const;
		const Segment *header(const char *namez) const;
		const Segment *header(const char *name, size_t nameSize) const;

		//body

		//params uri
		//params body

		//params all

	public:
		Response response();
	};
}}}

#endif
