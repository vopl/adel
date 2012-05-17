#include "pch.hpp"
#include "http/impl/contentFilter.hpp"
#include "http/error.hpp"


namespace http { namespace impl
{
	//////////////////////////////////////////////////////////////////////////////
	ContentFilter::ContentFilter()
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	ContentFilter::~ContentFilter()
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	boost::system::error_code ContentFilter::filterPush(const net::Packet &packet, size_t offset)
	{
		assert(!"must be reimplemented");
		return http::error::make(http::error::not_implemented);
	}
	
	//////////////////////////////////////////////////////////////////////////////
	boost::system::error_code ContentFilter::filterFlush()
	{
		assert(!"must be reimplemented");
		return http::error::make(http::error::not_implemented);
	}

}}
