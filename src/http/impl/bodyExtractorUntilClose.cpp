#include "pch.hpp"

#include "http/impl/bodyExtractorUntilClose.hpp"

namespace http { namespace impl{

	BodyExtractorUntilClose::BodyExtractorUntilClose(const ContentDecoderPtr &bodyDecoder)
		: BodyExtractor(bodyDecoder)
	{
	}

	BodyExtractorUntilClose::~BodyExtractorUntilClose()
	{
	}

	boost::system::error_code BodyExtractorUntilClose::read(const ContentDecoderAccumulerPtr &from, http::InputMessage::Iterator &begin)
	{
		assert(0);
		return boost::system::error_code();
	}

	boost::system::error_code BodyExtractorUntilClose::read(net::Channel channel, size_t granula)
	{
		assert(0);
		return boost::system::error_code();
	}

	boost::system::error_code BodyExtractorUntilClose::flush(ContentDecoderPtr decoder4tail)
	{
		assert(0);
		return boost::system::error_code();
	}

}}
