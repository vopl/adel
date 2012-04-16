#ifndef _NET_HTTP_SERVER_RESPONSE_HPP_
#define _NET_HTTP_SERVER_RESPONSE_HPP_

namespace net { namespace http { namespace server
{

	namespace impl
	{
		class Response;
		typedef boost::shared_ptr<Response> ResponsePtr;
	}
	///////////////////////////////////////////////////////
	class Response
	{
	protected:
		typedef impl::ResponsePtr ImplPtr;
		ImplPtr _impl;

	private:
		Response();

	public:
		~Response();

	};
}}}

#endif
