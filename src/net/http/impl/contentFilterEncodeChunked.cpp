#include "pch.hpp"
#include "net/http/impl/contentFilterEncodeChunked.hpp"


namespace net { namespace http { namespace impl
{
	//////////////////////////////////////////////////////////////////////////////
	ContentFilterEncodeChunked::ContentFilterEncodeChunked(ContentFilter* upstream)
		: ContentFilter(upstream)
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	ContentFilterEncodeChunked::~ContentFilterEncodeChunked()
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	size_t ContentFilterEncodeChunked::filterPush(const Packet &packet, size_t offset)
	{
		return _upstream->filterPush(packet, offset);
	}
	
	//////////////////////////////////////////////////////////////////////////////
	size_t ContentFilterEncodeChunked::filterFlush()
	{
		return _upstream->filterFlush();
	}

}}}
