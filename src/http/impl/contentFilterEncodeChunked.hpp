#ifndef _HTTP_IMPL_CONTENTFILTERENCODECHUNKED_HPP_
#define _HTTP_IMPL_CONTENTFILTERENCODECHUNKED_HPP_

#include "http/impl/contentFilter.hpp"

namespace http { namespace impl
{
	class ContentFilterEncodeChunked
		: public ContentFilter
	{
	public:
		ContentFilterEncodeChunked(ContentFilterPtr upstream, size_t granula);
		virtual ~ContentFilterEncodeChunked();

		virtual bool filterPush(const net::Packet &packet, size_t offset=0);
		virtual bool filterFlush();

	protected:
		struct SChunk
		{
			net::Packet	_packet;
			size_t	_offset;
		};

		ContentFilterPtr	_upstream;
		size_t				_granula;
		std::vector<SChunk>	_chunks;
		size_t				_size;
	protected:
		bool push2Upstream(bool finish = false);
	};
	typedef boost::shared_ptr<ContentFilterEncodeChunked> ContentFilterEncodeChunkedPtr;
}}

#endif
