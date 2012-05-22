#ifndef _HTTP_IMPL_BODYEXTRACTORCHUNKED_HPP_
#define _HTTP_IMPL_BODYEXTRACTORCHUNKED_HPP_

#include "http/impl/bodyExtractor.hpp"


namespace http { namespace impl{

	class BodyExtractorChunked
		: public BodyExtractor
	{
	public:
		BodyExtractorChunked(const ContentDecoderPtr &bodyDecoder, ContentDecoderPtr tailDecoder);
		virtual ~BodyExtractorChunked();

	private:
		virtual bool isDone();
		virtual boost::system::error_code process(ContentDecoderAccumuler &data);

	private:
		boost::system::error_code processCaption(ContentDecoderAccumuler &data);
		boost::system::error_code processTrailerHeader(ContentDecoderAccumuler &data);

	private:

		enum EState
		{
			es_caption,
			es_body,
			es_trailerHeader,
			es_done,
			es_error,
		};
		EState _es;

		size_t					_bodySize;
	};
	typedef boost::shared_ptr<BodyExtractorChunked> BodyExtractorChunkedPtr;
}}

#endif
