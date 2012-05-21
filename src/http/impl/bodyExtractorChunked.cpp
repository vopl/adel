#include "pch.hpp"

#include "http/impl/bodyExtractorChunked.hpp"

namespace http { namespace impl{

	BodyExtractorChunked::BodyExtractorChunked(const ContentDecoderPtr &bodyDecoder)
		: BodyExtractor(bodyDecoder)
	{
	}

	BodyExtractorChunked::~BodyExtractorChunked()
	{
	}

	boost::system::error_code BodyExtractorChunked::read(const ContentDecoderAccumulerPtr &from, http::InputMessage::Iterator &begin)
	{
		assert(0);
		return boost::system::error_code();
	}

	boost::system::error_code BodyExtractorChunked::read(net::Channel channel, size_t granula)
	{
		assert(0);
		return boost::system::error_code();
	}

	boost::system::error_code BodyExtractorChunked::flush(ContentDecoderPtr decoder4tail)
	{
		assert(0);
		return boost::system::error_code();
	}

}}
