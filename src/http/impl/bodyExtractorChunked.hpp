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
		virtual boost::system::error_code push(const net::Packet &p, size_t offset);

	private:
		boost::system::error_code pushCaption(const net::Packet &p, size_t offset);
		boost::system::error_code pushBody(const net::Packet &p, size_t offset);
		boost::system::error_code pushTrailerHeader(const net::Packet &p, size_t offset);

		boost::system::error_code pushFromAccumuler();

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

		ContentDecoderAccumuler	_accumuler;
		size_t					_bodySize;
	};
	typedef boost::shared_ptr<BodyExtractorChunked> BodyExtractorChunkedPtr;
}}

#endif
