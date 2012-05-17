#include "pch.hpp"
#include "http/client/impl/response.hpp"
#include "http/client/impl/request.hpp"
#include "http/impl/client.hpp"
#include "http/impl/contentFilterEncodeChunked.hpp"
#include "http/impl/contentFilterEncodeZlib.hpp"
#include "http/headerName.hpp"

#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/karma_string.hpp>
#include <boost/spirit/include/karma_char.hpp>

#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_container.hpp>


namespace http { namespace client { namespace impl
{

	////////////////////////////////////////////////////////////////////////////////////////
	Response::Response(const http::impl::ClientPtr &client, const net::Channel &channel, Request *request)
		: http::impl::InputMessage(channel, client->responseReadGranula())
	{
	}

	////////////////////////////////////////////////////////////////////////////////////////
	Response::~Response()
	{
	}


}}}
