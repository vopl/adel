#ifndef _HTTP_IMPL_BODYEXTRACTORUNTILCLOSE_HPP_
#define _HTTP_IMPL_BODYEXTRACTORUNTILCLOSE_HPP_

#include "http/impl/bodyExtractor.hpp"


namespace http { namespace impl{

	class BodyExtractorUntilClose
		: public BodyExtractor
	{
	public:
		BodyExtractorUntilClose(const ContentDecoderPtr &bodyDecoder);
		virtual ~BodyExtractorUntilClose();

		virtual boost::system::error_code read(const ContentDecoderAccumulerPtr &from, http::InputMessage::Iterator &begin);
		virtual boost::system::error_code read(net::Channel channel, size_t granula);
		virtual boost::system::error_code flush(ContentDecoderPtr decoder4tail);
	};
	typedef boost::shared_ptr<BodyExtractorUntilClose> BodyExtractorUntilClosePtr;
}}

#endif
