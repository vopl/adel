#include "pch.hpp"
#include "http/impl/contentFilterDecodeZlib.hpp"


namespace http { namespace impl
{
	//////////////////////////////////////////////////////////////////////////////
	ContentFilterDecodeZlib::ContentFilterDecodeZlib(ContentFilter *upstream)
		: _upstream(upstream)
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	ContentFilterDecodeZlib::~ContentFilterDecodeZlib()
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	bool ContentFilterDecodeZlib::filterPush(const net::Packet &packet, size_t offset)
	{
		assert(!"not impl");
		return _upstream->filterPush(packet, offset);
	}
	
	//////////////////////////////////////////////////////////////////////////////
	bool ContentFilterDecodeZlib::filterFlush()
	{
		assert(!"not impl");
		return _upstream->filterFlush();
	}

}}