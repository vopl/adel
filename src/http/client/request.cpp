#include "pch.hpp"
#include "http/client/request.hpp"
#include "http/client/impl/request.hpp"
#include "http/client/impl/response.hpp"
#include "utils/implAccess.hpp"

namespace http { namespace client
{

	//////////////////////////////////////////////////////////////
	Request::Request()
		: _impl()
		, http::OutputMessage()
	{

	}

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

	//////////////////////////////////////////////////////////////
	boost::system::error_code Request::firstLine(EMethod method, const char *path, size_t pathSize, const Version &version)
	{
		return _impl->firstLine(method, path, pathSize, version);
	}

	//////////////////////////////////////////////////////////////
	boost::system::error_code Request::firstLine(EMethod method, const char *pathz, const Version &version)
	{
		return _impl->firstLine(method, pathz, version);
	}

	//////////////////////////////////////////////////////////////
	boost::system::error_code Request::firstLine(EMethod method, const std::string &path, const Version &version)
	{
		return _impl->firstLine(method, path, version);
	}

}}
