#ifndef _NET_HTTP_METHOD_HPP_
#define _NET_HTTP_METHOD_HPP_


namespace net { namespace http
{
	enum EMethod
	{
		em_UNKNOWN	= 0,
		em_OPTIONS	= 1 << 1,
		em_GET		= 1 << 2,
		em_POST		= 1 << 3,
		em_HEAD		= 1 << 4,
		em_TRACE	= 1 << 5,
		em_PUT		= 1 << 6,
		em_DELETE	= 1 << 7,
		em_CONNECT	= 1 << 8,
	};
}}
#endif
