#ifndef _HTTP_SERVER_REQUEST_HPP_
#define _HTTP_SERVER_REQUEST_HPP_

#include "http/server/response.hpp"
#include "http/inputMessage.hpp"
#include "http/method.hpp"
#include "http/version.hpp"

namespace http { namespace server
{

	namespace impl
	{
		class Request;
		typedef boost::shared_ptr<Request> RequestPtr;
	}
	///////////////////////////////////////////////////////
	class Request
		: public http::InputMessage
	{
	protected:
		typedef impl::RequestPtr ImplPtr;
		ImplPtr _impl;

	public:
		Request(ImplPtr impl);
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
}}

#endif
