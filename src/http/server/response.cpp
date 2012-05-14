#include "pch.hpp"
#include "http/server/response.hpp"
#include "http/server/impl/response.hpp"

namespace http { namespace server
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
	bool Response::firstLine(const Version &version, const EStatusCode &statusCode)
	{
		return _impl->firstLine(version, statusCode);
	}

	/////////////////////////////////////////////////////////////////////
	bool Response::firstLine(const EStatusCode &statusCode)
	{
		return _impl->firstLine(statusCode);
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

	/////////////////////////////////////////////////////////////////////
	bool Response::bodyFlush()
	{
		return _impl->bodyFlush();
	}

}}
