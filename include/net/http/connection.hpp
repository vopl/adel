#ifndef _NET_HTTP_CONNECTION_HPP_
#define _NET_HTTP_CONNECTION_HPP_

namespace net { namespace http
{

	enum EConnection
	{
		ec_keepAlive	= 1,
		ec_close		= 2,
	};
}}

#endif
