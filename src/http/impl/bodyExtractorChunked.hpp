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

		virtual boost::system::error_code read(const ContentDecoderAccumulerPtr &from, const http::InputMessage::Iterator &begin);
		virtual boost::system::error_code read(net::Channel channel, size_t granula);
		virtual boost::system::error_code flush();

	private:
		boost::system::error_code push(const net::Packet &p, size_t offset);
		boost::system::error_code pushCaption(const net::Packet &p, size_t offset);
		boost::system::error_code pushBody(const net::Packet &p, size_t offset);
		boost::system::error_code pushHeader(const net::Packet &p, size_t offset);
		bool isDone();

	private:

		enum EState
		{
			es_caption,
			es_body,
			es_header,
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
