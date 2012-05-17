#include "pch.hpp"
#include "http/impl/contentEncoder.hpp"
#include "http/error.hpp"


namespace http { namespace impl
{
	//////////////////////////////////////////////////////////////////////////////
	ContentEncoder::ContentEncoder()
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	ContentEncoder::~ContentEncoder()
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	boost::system::error_code ContentEncoder::filterPush(const net::Packet &packet, size_t offset)
	{
		assert(!"must be reimplemented");
		return http::error::make(http::error::not_implemented);
	}
	
	//////////////////////////////////////////////////////////////////////////////
	boost::system::error_code ContentEncoder::filterFlush()
	{
		assert(!"must be reimplemented");
		return http::error::make(http::error::not_implemented);
	}

}}
