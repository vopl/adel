#ifndef _NET_HTTP_IMPL_CONTENTFILTERENCODECHUNKED_HPP_
#define _NET_HTTP_IMPL_CONTENTFILTERENCODECHUNKED_HPP_

#include "net/http/impl/contentFilter.hpp"

namespace net { namespace http { namespace impl
{
	class ContentFilterEncodeChunked
		: public ContentFilter
	{
	public:
		ContentFilterEncodeChunked(ContentFilter* upstream);
		virtual ~ContentFilterEncodeChunked();

		virtual size_t filterPush(const Packet &packet, size_t offset=0);
		virtual size_t filterFlush();

	protected:
		static Packet _chunkFooter;
		static Packet _lastChunk;
	};
}}}
#endif
