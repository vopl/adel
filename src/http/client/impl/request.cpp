#include "pch.hpp"
#include "http/client/impl/request.hpp"
#include "http/impl/client.hpp"

#include "http/headerName.hpp"

#include "http/client/log.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_string.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_lit.hpp>

#include <boost/spirit/include/phoenix_core.hpp>

namespace http { namespace client { namespace impl
{

	//////////////////////////////////////////////////////////////////////////
	Request::Request(const http::impl::ClientPtr &server, const net::Channel &channel)
		: http::impl::OutputMessage(channel, 220)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	Request::~Request()
	{
	}


}}}
