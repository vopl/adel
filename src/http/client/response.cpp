#include "pch.hpp"
#include "http/client/response.hpp"
#include "http/client/impl/response.hpp"

namespace http { namespace client
{

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
}}
