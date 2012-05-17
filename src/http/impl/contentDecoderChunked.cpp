#include "pch.hpp"
#include "http/impl/contentDecoderChunked.hpp"


namespace http { namespace impl
{
	//////////////////////////////////////////////////////////////////////////////
	ContentDecoderChunked::ContentDecoderChunked(ContentDecoder *upstream)
		: _upstream(upstream)
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	ContentDecoderChunked::~ContentDecoderChunked()
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	boost::system::error_code ContentDecoderChunked::decoderPush(const net::Packet &packet, size_t offset)
	{
		assert(!"not impl");
		return _upstream->decoderPush(packet, offset);
	}
	
	//////////////////////////////////////////////////////////////////////////////
	boost::system::error_code ContentDecoderChunked::decoderFlush()
	{
		assert(!"not impl");
		return _upstream->decoderFlush();
	}

}}
