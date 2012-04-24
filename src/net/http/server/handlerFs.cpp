#include "pch.hpp"
#include "net/http/server/handlerFs.hpp"
#include "net/http/server/impl/handlerFs.hpp"

namespace net { namespace http { namespace server
{

	/////////////////////////////////////////////////////////////////////
	utils::OptionsPtr HandlerFs::prepareOptions(const char *prefix)
	{
		return impl::HandlerFs::prepareOptions(prefix);
	}

	/////////////////////////////////////////////////////////////////////
	HandlerFs::HandlerFs(utils::OptionsPtr options)
		: _impl(new impl::HandlerFs(options))
	{
	}

	/////////////////////////////////////////////////////////////////////
	HandlerFs::~HandlerFs()
	{
	}

	/////////////////////////////////////////////////////////////////////
	void HandlerFs::onRequest(const Request &r)
	{
		return _impl->onRequest(r);
	}

}}}
