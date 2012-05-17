#ifndef _HTTP_IMPL_CONTENTENCODERCHUNKED_HPP_
#define _HTTP_IMPL_CONTENTENCODERCHUNKED_HPP_

#include "http/impl/contentEncoder.hpp"

namespace http { namespace impl
{
	class ContentEncoderChunked
		: public ContentEncoder
	{
	public:
		ContentEncoderChunked(ContentEncoderPtr upstream, size_t granula);
		virtual ~ContentEncoderChunked();

		virtual boost::system::error_code filterPush(const net::Packet &packet, size_t offset=0);
		virtual boost::system::error_code filterFlush();

	protected:
		struct SChunk
		{
			net::Packet	_packet;
			size_t	_offset;
		};

		ContentEncoderPtr	_upstream;
		size_t				_granula;
		std::vector<SChunk>	_chunks;
		size_t				_size;
	protected:
		boost::system::error_code push2Upstream(bool finish = false);
	};
	typedef boost::shared_ptr<ContentEncoderChunked> ContentEncoderChunkedPtr;
}}

#endif
