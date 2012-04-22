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
	boost::uint32_t ContentFilterDecodeZlib::filterPush(const Packet &packet, boost::uint32_t offset)
	{
		return _upstream->filterPush(packet, offset);
	}
	
	//////////////////////////////////////////////////////////////////////////////
	boost::uint32_t ContentFilterDecodeZlib::filterFlush()
	{
		return _upstream->filterFlush();
	}

}}}
