#ifndef _HTTP_IMPL_BODYEXTRACTOR_HPP_
#define _HTTP_IMPL_BODYEXTRACTOR_HPP_

#include "http/impl/contentDecoderAccumuler.hpp"
#include "http/inputMessage.hpp"
#include "net/channel.hpp"
#include <boost/shared_ptr.hpp>


namespace http { namespace impl{

	class BodyExtractor
	{
	public:
		BodyExtractor(const ContentDecoderPtr &bodyDecoder);
		virtual ~BodyExtractor();

		virtual boost::system::error_code read(const ContentDecoderAccumulerPtr &from, http::InputMessage::Iterator &begin) =0;
		virtual boost::system::error_code read(net::Channel channel, size_t granula) =0;
		virtual boost::system::error_code flush(ContentDecoderPtr decoder4tail) =0;

	protected:
		ContentDecoderPtr _bodyDecoder;
	};
	typedef boost::shared_ptr<BodyExtractor> BodyExtractorPtr;
}}

#endif
