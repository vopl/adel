#ifndef _NET_HTTP_IMPL_CONTENTFILTERENCODECHUNKED_HPP_
#define _NET_HTTP_IMPL_CONTENTFILTERENCODECHUNKED_HPP_

#include "net/http/impl/contentFilter.hpp"

namespace net { namespace http { namespace impl
{
	class ContentFilterEncodeChunked
		: public ContentFilter
	{
	public:
		ContentFilterEncodeChunked(ContentFilter* upstream, size_t granula);
		virtual ~ContentFilterEncodeChunked();

		virtual bool filterPush(const Packet &packet, size_t offset=0);
		virtual bool filterFlush();

	protected:
		size_t _granula;
		struct SChunk
		{
			Packet	_packet;
			size_t	_offset;
		};
		std::vector<SChunk> _chunks;
		size_t _size;
	protected:
		bool push2Upstream(bool finish = false);
	};
}}}
#endif
