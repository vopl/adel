#include "pch.hpp"
#include "net/http/server/response.hpp"
#include "net/http/server/impl/response.hpp"

namespace net { namespace http { namespace server
{

	/////////////////////////////////////////////////////////////////////
	Response::Response(ImplPtr impl)
		: OutputMessage(impl)
		, _impl(impl)
	{

	}

	/////////////////////////////////////////////////////////////////////
	Response::~Response()
	{

	}

	/////////////////////////////////////////////////////////////////////
	bool Response::responseLine(const Version &version, const EStatusCode &statusCode)
	{
		return _impl->responseLine(version, statusCode);
	}

	/////////////////////////////////////////////////////////////////////
	bool Response::responseLine(const EStatusCode &statusCode)
	{
		return _impl->responseLine(statusCode);
	}

	/////////////////////////////////////////////////////////////////////
	void Response::setContentLength(size_t size)
	{
		return _impl->setContentLength(size);
	}

	/////////////////////////////////////////////////////////////////////
	void Response::setContentCompress(int level)
	{
		return _impl->setContentCompress(level);
	}

}}}
