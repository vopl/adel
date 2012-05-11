#include "pch.hpp"
#include "net/http/server/request.hpp"
#include "net/http/server/impl/request.hpp"
#include "net/http/server/impl/response.hpp"
#include "utils/implAccess.hpp"

namespace net { namespace http { namespace server
{

	//////////////////////////////////////////////////////////////
	Request::Request()
	{

	}

	//////////////////////////////////////////////////////////////
	Request::~Request()
	{

	}

	//////////////////////////////////////////////////////////////
	const EMethod &Request::method_() const
	{
		return _impl->method_();
	}

	//////////////////////////////////////////////////////////////
	const InputMessage::Segment &Request::method() const
	{
		return _impl->method();
	}

	//////////////////////////////////////////////////////////////
	const Version &Request::version_() const
	{
		return _impl->version_();
	}

	//////////////////////////////////////////////////////////////
	const InputMessage::Segment &Request::version() const
	{
		return _impl->version();
	}

	//////////////////////////////////////////////////////////////
	const InputMessage::Segment &Request::uri() const
	{
		return _impl->uri();
	}

	//////////////////////////////////////////////////////////////
	const InputMessage::Segment &Request::path() const
	{
		return _impl->path();
	}

	//////////////////////////////////////////////////////////////
	const InputMessage::Segment &Request::queryString() const
	{
		return _impl->queryString();
	}


	//////////////////////////////////////////////////////////////
	Response Request::response()
	{
		return Response(_impl->response());
	}

}}}
