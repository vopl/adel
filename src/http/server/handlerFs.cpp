#include "pch.hpp"
#include "http/server/handlerFs.hpp"
#include "http/server/impl/handlerFs.hpp"

namespace http { namespace server
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
	void HandlerFs::onRequest(Request r)
	{
		return _impl->onRequest(r);
	}

}}
