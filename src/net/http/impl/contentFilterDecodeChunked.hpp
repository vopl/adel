#ifndef _NET_HTTP_IMPL_CONTENTFILTERDECODECHUNKED_HPP_
#define _NET_HTTP_IMPL_CONTENTFILTERDECODECHUNKED_HPP_

#include "net/http/impl/contentFilter.hpp"

namespace net { namespace http { namespace impl
{
	class ContentFilterDecodeChunked
		: public ContentFilter
	{
	public:
		ContentFilterDecodeChunked(ContentFilter* upstream);
		virtual ~ContentFilterDecodeChunked();

		virtual size_t filterPush(const Packet &packet, size_t offset=0);
		virtual size_t filterFlush();

	protected:
	};
}}}
#endif
