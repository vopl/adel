#ifndef _HTTP_CONNECTION_HPP_
#define _HTTP_CONNECTION_HPP_

namespace http
{

	enum EConnection
	{
		ec_keepAlive	= 1,
		ec_close		= 2,
	};
}

#endif
