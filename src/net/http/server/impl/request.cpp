#include "pch.hpp"
#include "net/http/server/impl/request.hpp"

namespace net { namespace http { namespace server { namespace impl
{

	//////////////////////////////////////////////////////////////
	Request::Request(const Channel &channel)
		: _channel(channel)
	{

	}

	//////////////////////////////////////////////////////////////
	Request::~Request()
	{

	}

}}}}
