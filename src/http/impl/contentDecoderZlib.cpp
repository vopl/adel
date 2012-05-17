#include "pch.hpp"
#include "http/impl/contentDecoderZlib.hpp"


namespace http { namespace impl
{
	//////////////////////////////////////////////////////////////////////////////
	ContentDecoderZlib::ContentDecoderZlib(ContentDecoder *upstream)
		: _upstream(upstream)
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	ContentDecoderZlib::~ContentDecoderZlib()
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	boost::system::error_code ContentDecoderZlib::decoderPush(const net::Packet &packet, size_t offset)
	{
		assert(!"not impl");
		return _upstream->decoderPush(packet, offset);
	}
	
	//////////////////////////////////////////////////////////////////////////////
	boost::system::error_code ContentDecoderZlib::decoderFlush()
	{
		assert(!"not impl");
		return _upstream->decoderFlush();
	}

}}
