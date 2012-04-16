#ifndef _NET_HTTP_SERVER_IMPL_REQUEST_HPP_
#define _NET_HTTP_SERVER_IMPL_REQUEST_HPP_

#include "net/channel.hpp"
#include <boost/shared_ptr.hpp>

namespace net { namespace http { namespace server { namespace impl
{
	class Request
	{
	public:
		Request(const Channel &channel);
		~Request();

	private:
		Channel _channel;
	};
	typedef boost::shared_ptr<Request> RequestPtr;

}}}}

#endif
