#include "pch.hpp"
#include "net/http/server/response.hpp"
#include "net/http/server/impl/response.hpp"

namespace net { namespace http { namespace server
{

	/////////////////////////////////////////////////////////////////////
	Response::Response()
	{

	}

	/////////////////////////////////////////////////////////////////////
	Response::~Response()
	{

	}

	/////////////////////////////////////////////////////////////////////
	Response &Response::version(const Version &version)
	{
		_impl->version(version);
		return *this;
	}

	/////////////////////////////////////////////////////////////////////
	Response &Response::statusCode(const EStatusCode &statusCode)
	{
		_impl->statusCode(statusCode);
		return *this;
	}

	/////////////////////////////////////////////////////////////////////
	Response &Response::header(const char *data, size_t size)
	{
		_impl->header(data, size);
		return *this;
	}

	/////////////////////////////////////////////////////////////////////
	Response &Response::header(const char *line)
	{
		_impl->header(line, strlen(line));
		return *this;
	}

	/////////////////////////////////////////////////////////////////////
	Response &Response::header(const std::string &data)
	{
		_impl->header(data.data(), data.size());
		return *this;
	}

	/////////////////////////////////////////////////////////////////////
	Response &Response::body(const char *data, size_t size)
	{
		_impl->body(data, size);
		return *this;
	}

	/////////////////////////////////////////////////////////////////////
	Response &Response::body(const char *line)
	{
		_impl->body(line, strlen(line));
		return *this;
	}

	/////////////////////////////////////////////////////////////////////
	Response &Response::body(const std::string &data)
	{
		_impl->body(data.data(), data.size());
		return *this;
	}

	/////////////////////////////////////////////////////////////////////
	bool Response::flush()
	{
		return _impl->flush();
	}

	/////////////////////////////////////////////////////////////////////
	void Response::setBodySize(size_t size)
	{
		return _impl->setBodySize(size);
	}

	/////////////////////////////////////////////////////////////////////
	void Response::setBodyCompress(int level, size_t buffer)
	{
		return _impl->setBodyCompress(level, buffer);
	}

}}}
