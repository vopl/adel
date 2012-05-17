#ifndef _HTTP_IMPL_CONTENTDECODERCHUNKED_HPP_
#define _HTTP_IMPL_CONTENTDECODERCHUNKED_HPP_

#include "http/impl/contentDecoder.hpp"

namespace http { namespace impl
{
	class ContentDecoderChunked
		: public ContentDecoder
	{
	public:
		ContentDecoderChunked(ContentFilter *upstream);
		virtual ~ContentDecoderChunked();

		virtual boost::system::error_code filterPush(const net::Packet &packet, size_t offset=0);
		virtual boost::system::error_code filterFlush();

	protected:
		ContentDecoder *_upstream;
	};
}}

#endif
