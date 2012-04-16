#ifndef _NET_HTTP_SERVER_REQUEST_HPP_
#define _NET_HTTP_SERVER_REQUEST_HPP_

#include "net/http/server/response.hpp"
#include "net/channel.hpp"
#include "net/message.hpp"

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
		enum Method
		{
			OPTIONS,
			GET,
			POST,
			HEAD,
			TRACE,
			PUT,
			DELETE,
			CONNECT,
		};

		struct Version
		{
			unsigned short _hi;
			unsigned short _lo;
		};


	public:
		~Request();

		bool readCaption();
		bool readHeaders();

		//method uri version
		Method method() const;
		Sequence method_() const;

		Version version() const;
		Sequence version_() const;

		Sequence uri_() const;

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
