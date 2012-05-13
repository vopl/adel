#include "pch.hpp"
#include "http/server/impl/request.hpp"
#include "http/impl/server.hpp"

#include "http/impl/server.hpp"
#include "http/impl/server.hpp"
#include "http/headerName.hpp"

#include "http/server/log.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_string.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_lit.hpp>

#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_container.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>

namespace http { namespace server { namespace impl
{

	//////////////////////////////////////////////////////////////////////////
	Request::Request(const http::impl::ServerPtr &server, const net::Channel &channel)
	{
		assert(0);
	}

	//////////////////////////////////////////////////////////////////////////
	Request::~Request()
	{
		assert(0);
	}

	//////////////////////////////////////////////////////////////////////////
	RequestPtr Request::shared_from_this()
	{
		assert(0);
		return RequestPtr();
	}

	//////////////////////////////////////////////////////////////////////////
	const EMethod &Request::method_() const
	{
		assert(0);
		return *(EMethod *)NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const Request::Segment &Request::method() const
	{
		assert(0);
		return *(Request::Segment *)NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const Version &Request::version_() const
	{
		assert(0);
		return *(Version *)NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const Request::Segment &Request::version() const
	{
		assert(0);
		return *(Request::Segment *)NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const Request::Segment &Request::uri() const
	{
		assert(0);
		return *(Request::Segment *)NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const Request::Segment &Request::path() const
	{
		assert(0);
		return *(Request::Segment *)NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const Request::Segment &Request::queryString() const
	{
		assert(0);
		return *(Request::Segment *)NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	ResponsePtr Request::response()
	{
		assert(0);
		return ResponsePtr();
	}

}}}
