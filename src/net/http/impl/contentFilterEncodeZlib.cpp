#include "pch.hpp"
#include "net/http/impl/contentFilterEncodeZlib.hpp"


namespace net { namespace http { namespace impl
{
	//////////////////////////////////////////////////////////////////////////////
	ContentFilterEncodeZlib::ContentFilterEncodeZlib(ContentFilter* upstream)
		: ContentFilter(upstream)
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	ContentFilterEncodeZlib::~ContentFilterEncodeZlib()
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	size_t ContentFilterEncodeZlib::filterPush(const Packet &packet, size_t offset)
	{
		return _upstream->filterPush(packet, offset);
	}
	
	//////////////////////////////////////////////////////////////////////////////
	size_t ContentFilterEncodeZlib::filterFlush()
	{
		return _upstream->filterFlush();
	}

}}}
