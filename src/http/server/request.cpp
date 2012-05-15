#include "pch.hpp"
#include "http/server/request.hpp"
#include "http/server/impl/request.hpp"
#include "http/server/impl/response.hpp"
#include "utils/implAccess.hpp"

namespace http { namespace server
{

	//////////////////////////////////////////////////////////////
	Request::Request(const ImplPtr &impl)
		: _impl(impl)
		, http::InputMessage(impl)
	{

	}

	//////////////////////////////////////////////////////////////
	Request::~Request()
	{

	}

	//////////////////////////////////////////////////////////////
	boost::system::error_code Request::readFirstLine()
	{
		return _impl->readFirstLine();
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

}}
