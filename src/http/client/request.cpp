#include "pch.hpp"
#include "http/client/request.hpp"
#include "http/client/impl/request.hpp"
#include "http/client/impl/response.hpp"
#include "utils/implAccess.hpp"

namespace http { namespace client
{

	//////////////////////////////////////////////////////////////
	Request::Request(const ImplPtr &impl)
		: _impl(impl)
		, http::OutputMessage(impl)
	{

	}

	//////////////////////////////////////////////////////////////
	Request::~Request()
	{

	}

}}
