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
	bool Request::ignoreBody()
	{
		return _impl->ignoreBody();
	}

	//////////////////////////////////////////////////////////////
	const Message::Segment &Request::requestLine_() const
	{
		return _impl->requestLine_();
	}

	//////////////////////////////////////////////////////////////
	const EMethod &Request::method() const
	{
		return _impl->method();
	}

	//////////////////////////////////////////////////////////////
	const Message::Segment &Request::method_() const
	{
		return _impl->method_();
	}

	//////////////////////////////////////////////////////////////
	const Version &Request::version() const
	{
		return _impl->version();
	}

	//////////////////////////////////////////////////////////////
	const Message::Segment &Request::version_() const
	{
		return _impl->version_();
	}

	//////////////////////////////////////////////////////////////
	const Message::Segment &Request::uri_() const
	{
		return _impl->uri_();
	}

	//////////////////////////////////////////////////////////////
	const Message::Segment &Request::path_() const
	{
		return _impl->path_();
	}

	//////////////////////////////////////////////////////////////
	const Message::Segment &Request::queryString_() const
	{
		return _impl->queryString_();
	}

	//////////////////////////////////////////////////////////////
	const Message::Segment *Request::header(const HeaderName &name) const
	{
		return _impl->header(name.hash);
	}

	//////////////////////////////////////////////////////////////
	const Message::Segment *Request::header(size_t hash) const
	{
		return _impl->header(hash);
	}

	//////////////////////////////////////////////////////////////
	const Message::Segment *Request::header(const std::string &name) const
	{
		return _impl->header(hn::hash(name));
	}

	//////////////////////////////////////////////////////////////
	const Message::Segment *Request::header(const char *namez) const
	{
		return _impl->header(hn::hash(namez));
	}

	//////////////////////////////////////////////////////////////
	const Message::Segment *Request::header(const char *name, size_t nameSize) const
	{
		return _impl->header(hn::hash(name, nameSize));
	}

	//////////////////////////////////////////////////////////////
	Response Request::response()
	{
		return utils::ImplAccess<Response>(_impl->response());
	}

}}}
