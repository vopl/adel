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
	Response &Response::header(const char *data)
	{
		_impl->header(data, strlen(data));
		return *this;
	}

	/////////////////////////////////////////////////////////////////////
	Response &Response::header(const std::string &data)
	{
		_impl->header(data.data(), data.size());
		return *this;
	}

	/////////////////////////////////////////////////////////////////////
	Response &Response::header(const HeaderName &name, const char *value, size_t valueSize)
	{
		Message::Iterator outIter = beginWriteHeader(name.str.data(), name.str.size());
		outIter = std::copy(value, value+valueSize, outIter);
		endWriteHeader(outIter);
		return *this;
	}

	/////////////////////////////////////////////////////////////////////
	Response &Response::header(const HeaderName &name, const char *valuez)
	{
		return header(name, valuez, strlen(valuez));
	}

	/////////////////////////////////////////////////////////////////////
	Response &Response::header(const HeaderName &name, const std::string &value)
	{
		return header(name, value.data(), value.size());
	}

	/////////////////////////////////////////////////////////////////////
	Response &Response::body(const char *data, size_t size)
	{
		_impl->body(data, size);
		return *this;
	}

	/////////////////////////////////////////////////////////////////////
	Response &Response::body(const char *data)
	{
		_impl->body(data, strlen(data));
		return *this;
	}

	/////////////////////////////////////////////////////////////////////
	Response &Response::body(const std::string &data)
	{
		_impl->body(data.data(), data.size());
		return *this;
	}

	/////////////////////////////////////////////////////////////////////
	MessageIterator Response::getWriteIterator()
	{
		return _impl->getWriteIterator();
	}

	/////////////////////////////////////////////////////////////////////
	void Response::setWriteIterator(MessageIterator iter)
	{
		return _impl->setWriteIterator(iter);
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

	/////////////////////////////////////////////////////////////////////
	Message::Iterator Response::beginWriteHeader(const char *name, size_t size)
	{
		return _impl->beginWriteHeader(name, size);
	}

	/////////////////////////////////////////////////////////////////////
	void Response::endWriteHeader(Message::Iterator iter)
	{
		return _impl->endWriteHeader(iter);
	}


}}}
