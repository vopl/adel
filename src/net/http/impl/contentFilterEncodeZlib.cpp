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
	boost::uint32_t ContentFilterEncodeZlib::filterPush(const Packet &packet, boost::uint32_t offset)
	{
		return _upstream->filterPush(packet, offset);
	}
	
	//////////////////////////////////////////////////////////////////////////////
	boost::uint32_t ContentFilterEncodeZlib::filterFlush()
	{
		return _upstream->filterFlush();
	}

}}}
