#include "pch.hpp"

#include "http/impl/bodyExtractorSized.hpp"

namespace http { namespace impl{

	BodyExtractorSized::BodyExtractorSized(const ContentDecoderPtr &bodyDecoder, size_t targetSize)
		: BodyExtractor(bodyDecoder)
	{
		assert(0);
	}

	BodyExtractorSized::~BodyExtractorSized()
	{
	}

	boost::system::error_code BodyExtractorSized::read(const ContentDecoderAccumulerPtr &from, http::InputMessage::Iterator &begin)
	{
		assert(0);
		return boost::system::error_code();
	}

	boost::system::error_code BodyExtractorSized::read(net::Channel channel, size_t granula)
	{
		assert(0);
		return boost::system::error_code();
	}

	boost::system::error_code BodyExtractorSized::flush(ContentDecoderPtr decoder4tail)
	{
		assert(0);
		return boost::system::error_code();
	}

}}
