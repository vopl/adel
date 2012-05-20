#include "pch.hpp"
#include "http/impl/contentDecoderChunked.hpp"


namespace http { namespace impl
{
	//////////////////////////////////////////////////////////////////////////////
	ContentDecoderChunked::ContentDecoderChunked(const ContentDecoderPtr &upstream)
		: _upstream(upstream)
		, _es(es_header)
		, _toRead(0)
		, _headerSize(0)
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	ContentDecoderChunked::~ContentDecoderChunked()
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	boost::system::error_code ContentDecoderChunked::decoderPush(const net::Packet &packet, size_t offset)
	{
		char *begin = packet._data.get() + offset;
		char *end = packet._data.get() + packet._size;
		assert(begin < end);

		switch(_es)
		{
		case es_header:
			break;
		case es_body:
			break;
		default:
			assert(0);
			return http::error::make(http::error::unexpected);
		}
		return _upstream->decoderPush(packet, offset);
	}

	//////////////////////////////////////////////////////////////////////////////
	boost::system::error_code ContentDecoderChunked::decoderFlush()
	{
		assert(!"not impl");
	}


	//////////////////////////////////////////////////////////////////////////////
	bool ContentDecoderChunked::isDone() const
	{
		assert(!"not impl");
	}

	//////////////////////////////////////////////////////////////////////////////
	size_t ContentDecoderChunked::contentLength() const
	{
		assert(!"not impl");
	}

}}
