#ifndef _HTTP_IMPL_BODYEXTRACTORCHUNKED_HPP_
#define _HTTP_IMPL_BODYEXTRACTORCHUNKED_HPP_

#include "http/impl/bodyExtractor.hpp"


namespace http { namespace impl{

	class BodyExtractorChunked
		: public BodyExtractor
	{
	public:
		BodyExtractorChunked(const ContentDecoderPtr &bodyDecoder);
		virtual ~BodyExtractorChunked();

		virtual boost::system::error_code read(const ContentDecoderAccumulerPtr &from, http::InputMessage::Iterator &begin);
		virtual boost::system::error_code read(net::Channel channel, size_t granula);
		virtual boost::system::error_code flush(ContentDecoderPtr decoder4tail);
	};
	typedef boost::shared_ptr<BodyExtractorChunked> BodyExtractorChunkedPtr;
}}

#endif
