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
	bool ContentFilter::filterPush(const Packet &packet, size_t offset)
	{
		return _upstream->filterPush(packet, offset);
	}
	
	//////////////////////////////////////////////////////////////////////////////
	bool ContentFilter::filterFlush()
	{
		return _upstream->filterFlush();
	}

}}}