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
	boost::uint32_t ContentFilterEncodeChunked::filterPush(const Packet &packet, boost::uint32_t offset)
	{
		return _upstream->filterPush(packet, offset);
	}
	
	//////////////////////////////////////////////////////////////////////////////
	boost::uint32_t ContentFilterEncodeChunked::filterFlush()
	{
		return _upstream->filterFlush();
	}

}}}
