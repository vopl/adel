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
	boost::uint32_t ContentFilterDecodeChunked::filterPush(const Packet &packet, boost::uint32_t offset)
	{
		return _upstream->filterPush(packet, offset);
	}
	
	//////////////////////////////////////////////////////////////////////////////
	boost::uint32_t ContentFilterDecodeChunked::filterFlush()
	{
		return _upstream->filterFlush();
	}

}}}
