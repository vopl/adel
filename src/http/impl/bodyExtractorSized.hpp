#ifndef _HTTP_IMPL_BODYEXTRACTORSIZED_HPP_
#define _HTTP_IMPL_BODYEXTRACTORSIZED_HPP_

#include "http/impl/bodyExtractor.hpp"


namespace http { namespace impl{

	class BodyExtractorSized
		: public BodyExtractor
	{
	public:
		BodyExtractorSized(const ContentDecoderPtr &bodyDecoder, size_t targetSize);
		virtual ~BodyExtractorSized();

		virtual boost::system::error_code read(const ContentDecoderAccumulerPtr &from, http::InputMessage::Iterator &begin);
		virtual boost::system::error_code read(net::Channel channel, size_t granula);
		virtual boost::system::error_code flush(ContentDecoderPtr decoder4tail);
	};
	typedef boost::shared_ptr<BodyExtractorSized> BodyExtractorSizedPtr;
}}

#endif
