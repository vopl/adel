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
	bool ContentFilterEncodeZlib::filterPush(const Packet &packet, size_t offset)
	{
		assert(!"not impl");
		return _upstream->filterPush(packet, offset);
	}
	
	//////////////////////////////////////////////////////////////////////////////
	bool ContentFilterEncodeZlib::filterFlush()
	{
		assert(!"not impl");
		return _upstream->filterFlush();
	}

}}}
