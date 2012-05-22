#ifndef _HTTP_IMPL_BODYEXTRACTORUNTILCLOSE_HPP_
#define _HTTP_IMPL_BODYEXTRACTORUNTILCLOSE_HPP_

#include "http/impl/bodyExtractor.hpp"


namespace http { namespace impl{

	class BodyExtractorUntilClose
		: public BodyExtractor
	{
	public:
		BodyExtractorUntilClose(const ContentDecoderPtr &bodyDecoder, const ContentDecoderPtr &tailDecoder);
		virtual ~BodyExtractorUntilClose();

		virtual boost::system::error_code read(net::Channel channel, size_t granula);

	private:
		virtual bool isDone();
		virtual boost::system::error_code process(ContentDecoderAccumuler &data);
	};
	typedef boost::shared_ptr<BodyExtractorUntilClose> BodyExtractorUntilClosePtr;
}}

#endif
