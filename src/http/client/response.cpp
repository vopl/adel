#include "pch.hpp"
#include "http/client/response.hpp"
#include "http/client/impl/response.hpp"

namespace http { namespace client
{

	/////////////////////////////////////////////////////////////////////
	Response::Response()
		: InputMessage()
		, _impl()
	{

	}

	/////////////////////////////////////////////////////////////////////
	Response::Response(const ImplPtr &impl)
		: InputMessage(impl)
		, _impl(impl)
	{

	}

	/////////////////////////////////////////////////////////////////////
	Response::~Response()
	{

	}

	/////////////////////////////////////////////////////////////////////
	EStatusCode Response::status() const
	{

		return _impl->status();
	}

	/////////////////////////////////////////////////////////////////////
	const Version &Response::version() const
	{
		return _impl->version();
	}

}}
