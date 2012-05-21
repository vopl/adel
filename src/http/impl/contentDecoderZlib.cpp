#include "pch.hpp"
#include "http/impl/contentDecoderZlib.hpp"


namespace http { namespace impl
{
	//////////////////////////////////////////////////////////////////////////////
	ContentDecoderZlib::ContentDecoderZlib(const ContentDecoderPtr &upstream, EContentEncoding ece)
		: _upstream(upstream)
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	ContentDecoderZlib::~ContentDecoderZlib()
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	boost::system::error_code ContentDecoderZlib::push(const net::Packet &packet, size_t offset)
	{
		assert(!"not impl");
		return _upstream->push(packet, offset);
	}

	//////////////////////////////////////////////////////////////////////////////
	boost::system::error_code ContentDecoderZlib::flush()
	{
		return _upstream->flush();
	}

}}
