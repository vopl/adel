#include "pch.hpp"
#include "net/http/impl/contentFilter.hpp"


namespace net { namespace http { namespace impl
{
	//////////////////////////////////////////////////////////////////////////////
	ContentFilter::ContentFilter(ContentFilter* upstream)
		: _upstream(upstream)
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	ContentFilter::~ContentFilter()
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	boost::uint32_t ContentFilter::filterPush(const Packet &packet, boost::uint32_t offset)
	{
		return _upstream->filterPush(packet, offset);
	}
	
	//////////////////////////////////////////////////////////////////////////////
	boost::uint32_t ContentFilter::filterFlush()
	{
		return _upstream->filterFlush();
	}

}}}
