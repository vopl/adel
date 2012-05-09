#ifndef _NET_HTTP_IMPL_CONTENTFILTERDECODECHUNKED_HPP_
#define _NET_HTTP_IMPL_CONTENTFILTERDECODECHUNKED_HPP_

#include "net/http/impl/contentFilter.hpp"

namespace net { namespace http { namespace impl
{
	class ContentFilterDecodeChunked
		: public ContentFilter
	{
	public:
		ContentFilterDecodeChunked(ContentFilter *upstream);
		virtual ~ContentFilterDecodeChunked();

		virtual bool filterPush(const Packet &packet, size_t offset=0);
		virtual bool filterFlush();

	protected:
		ContentFilter *_upstream;
	};
}}}
#endif
