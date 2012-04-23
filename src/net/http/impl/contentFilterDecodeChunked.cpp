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
	bool ContentFilterDecodeChunked::filterPush(const Packet &packet, size_t offset)
	{
		assert(!"not impl");
		return _upstream->filterPush(packet, offset);
	}
	
	//////////////////////////////////////////////////////////////////////////////
	bool ContentFilterDecodeChunked::filterFlush()
	{
		assert(!"not impl");
		return _upstream->filterFlush();
	}

}}}
