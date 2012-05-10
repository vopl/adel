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
	bool Request::readRequestLine()
	{
		return _impl->readRequestLine();
	}

	//////////////////////////////////////////////////////////////
	bool Request::readHeaders()
	{
		return _impl->readHeaders();
	}

	//////////////////////////////////////////////////////////////
	bool Request::readBody()
	{
		return _impl->readBody();
	}

	//////////////////////////////////////////////////////////////
	const InputMessage::Segment &Request::requestLine() const
	{
		return _impl->requestLine();
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
	const InputMessage::Segment &Request::headers() const
	{
		return _impl->headers();
	}

	//////////////////////////////////////////////////////////////
	const InputMessage::Segment *Request::header(const HeaderName &name) const
	{
		return _impl->header(name);
	}

	//////////////////////////////////////////////////////////////
	const InputMessage::Segment *Request::header(size_t key) const
	{
		return _impl->header(key);
	}

	//////////////////////////////////////////////////////////////
	const InputMessage::Segment *Request::header(const std::string &name) const
	{
		return _impl->header(name);
	}

	//////////////////////////////////////////////////////////////
	const InputMessage::Segment *Request::header(const char *namez) const
	{
		return _impl->header(namez);
	}

	//////////////////////////////////////////////////////////////
	const InputMessage::Segment *Request::header(const char *name, size_t nameSize) const
	{
		return _impl->header(name, nameSize);
	}

	//////////////////////////////////////////////////////////////
	const InputMessage::Segment &Request::body() const
	{
		return _impl->body();
	}

	//////////////////////////////////////////////////////////////
	Response Request::response()
	{
		return Response(_impl->response());
	}

}}}
