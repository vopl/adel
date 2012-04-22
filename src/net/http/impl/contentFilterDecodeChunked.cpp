#include "pch.hpp"
#include "net/http/impl/contentFilterDecodeChunked.hpp"


namespace net { namespace http { namespace impl
{
	//////////////////////////////////////////////////////////////////////////////
	ContentFilterDecodeChunked::ContentFilterDecodeChunked(ContentFilter* upstream)
		: ContentFilter(upstream)
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	ContentFilterDecodeChunked::~ContentFilterDecodeChunked()
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	size_t ContentFilterDecodeChunked::filterPush(const Packet &packet, size_t offset)
	{
		return _upstream->filterPush(packet, offset);
	}
	
	//////////////////////////////////////////////////////////////////////////////
	size_t ContentFilterDecodeChunked::filterFlush()
	{
		return _upstream->filterFlush();
	}

}}}
