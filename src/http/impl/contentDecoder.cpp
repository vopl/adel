#include "pch.hpp"
#include "http/impl/contentDecoder.hpp"
#include "http/error.hpp"


namespace http { namespace impl
{
	//////////////////////////////////////////////////////////////////////////////
	ContentDecoder::ContentDecoder()
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	ContentDecoder::~ContentDecoder()
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	boost::system::error_code ContentDecoder::push(const net::Packet &packet, size_t offset)
	{
		assert(!"must be reimplemented");
		return http::error::make(http::error::not_implemented);
	}
	
	//////////////////////////////////////////////////////////////////////////////
	boost::system::error_code ContentDecoder::flush()
	{
		assert(!"must be reimplemented");
		return http::error::make(http::error::not_implemented);
	}

}}
