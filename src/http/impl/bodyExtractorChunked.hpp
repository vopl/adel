#ifndef _HTTP_IMPL_CONTENTDECODERCHUNKED_HPP_
#define _HTTP_IMPL_CONTENTDECODERCHUNKED_HPP_

#include "http/impl/contentDecoder.hpp"
#include "http/impl/contentDecoderAccumuler.hpp"

namespace http { namespace impl
{
	class ContentDecoderChunked
		: public ContentDecoder
	{
	public:
		ContentDecoderChunked(const ContentDecoderPtr &upstream);
		virtual ~ContentDecoderChunked();

		virtual boost::system::error_code decoderPush(const net::Packet &packet, size_t offset=0);
		virtual boost::system::error_code decoderFlush();

		bool isDone() const;
		size_t contentLength() const;

	protected:
		ContentDecoderPtr _upstream;

		enum EState
		{
			es_header,
			es_body,
			es_done,
		};
		EState	_es;
		size_t	_toRead;
		char	_header[64];
		size_t	_headerSize;
	};
}}

#endif
