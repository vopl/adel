#include "pch.hpp"

#include "http/impl/bodyExtractor.hpp"

namespace http { namespace impl{

	BodyExtractor::BodyExtractor(const ContentDecoderPtr &bodyDecoder)
		: _bodyDecoder(bodyDecoder)
	{
	}

	BodyExtractor::~BodyExtractor()
	{
	}

	boost::system::error_code BodyExtractor::read(const ContentDecoderAccumulerPtr &from, http::InputMessage::Iterator &begin)
	{
		assert(0);
		return boost::system::error_code();
	}

	boost::system::error_code BodyExtractor::read(net::Channel channel, size_t granula)
	{
		assert(0);
		return boost::system::error_code();
	}

	boost::system::error_code BodyExtractor::flush(ContentDecoderPtr decoder4tail)
	{
		assert(0);
		return boost::system::error_code();
	}

}}
