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
		BodyExtractor(const ContentDecoderPtr &bodyDecoder, ContentDecoderPtr tailDecoder);
		virtual ~BodyExtractor();

		boost::system::error_code read(const ContentDecoderAccumulerPtr &from, const http::InputMessage::Iterator &begin);
		boost::system::error_code read(net::Channel channel, size_t granula);
		boost::system::error_code flush();

	protected:
		virtual bool isDone() = 0;
		virtual boost::system::error_code push(const net::Packet &p, size_t offset) = 0;

	protected:
		ContentDecoderPtr _bodyDecoder;
		ContentDecoderPtr _tailDecoder;
	};
	typedef boost::shared_ptr<BodyExtractor> BodyExtractorPtr;
}}

#endif
