#include "pch.hpp"
#include "http/impl/contentFilterDecodeChunked.hpp"


namespace http { namespace impl
{
	//////////////////////////////////////////////////////////////////////////////
	ContentFilterDecodeChunked::ContentFilterDecodeChunked(ContentFilter *upstream)
		: _upstream(upstream)
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	ContentFilterDecodeChunked::~ContentFilterDecodeChunked()
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	boost::system::error_code ContentFilterDecodeChunked::filterPush(const net::Packet &packet, size_t offset)
	{
		assert(!"not impl");
		return _upstream->filterPush(packet, offset);
	}
	
	//////////////////////////////////////////////////////////////////////////////
	boost::system::error_code ContentFilterDecodeChunked::filterFlush()
	{
		assert(!"not impl");
		return _upstream->filterFlush();
	}

}}
