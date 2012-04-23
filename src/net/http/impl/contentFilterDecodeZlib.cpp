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
	bool ContentFilterDecodeZlib::filterPush(const Packet &packet, size_t offset)
	{
		assert(!"not impl");
		return _upstream->filterPush(packet, offset);
	}
	
	//////////////////////////////////////////////////////////////////////////////
	bool ContentFilterDecodeZlib::filterFlush()
	{
		assert(!"not impl");
		return _upstream->filterFlush();
	}

}}}
