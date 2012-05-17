#ifndef _HTTP_IMPL_CONTENTDECODERACCUMULER_HPP_
#define _HTTP_IMPL_CONTENTDECODERACCUMULER_HPP_

#include "http/impl/contentDecoder.hpp"
#include "http/impl/inputMessageBuffer.hpp"
#include "http/inputMessage.hpp"

namespace http { namespace impl
{
	class ContentDecoderAccumuler
		: public ContentDecoder
	{
	public:
		ContentDecoderAccumuler();
		~ContentDecoderAccumuler();

		virtual boost::system::error_code decoderPush(const net::Packet &packet, size_t offset=0);
		virtual boost::system::error_code decoderFlush();

		InputMessageBuffer	*firstBuffer();
		InputMessageBuffer	*lastBuffer();

		http::InputMessage::Iterator	begin();
		http::InputMessage::Iterator	end();

		void dropFront(const http::InputMessage::Iterator &pos);

	private:
		InputMessageBufferPtr	_first;
		InputMessageBuffer		*_last;
		size_t					_size;
	};

	typedef boost::shared_ptr<ContentDecoderAccumuler> ContentDecoderAccumulerPtr;
}}


#endif
