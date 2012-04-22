#include "pch.hpp"
#include "net/http/impl/contentFilterDecodeZlib.hpp"


namespace net { namespace http { namespace impl
{
	//////////////////////////////////////////////////////////////////////////////
	ContentFilterDecodeZlib::ContentFilterDecodeZlib(ContentFilter* upstream)
		: ContentFilter(upstream)
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	ContentFilterDecodeZlib::~ContentFilterDecodeZlib()
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	size_t ContentFilterDecodeZlib::filterPush(const Packet &packet, size_t offset)
	{
		return _upstream->filterPush(packet, offset);
	}
	
	//////////////////////////////////////////////////////////////////////////////
	size_t ContentFilterDecodeZlib::filterFlush()
	{
		return _upstream->filterFlush();
	}

}}}
