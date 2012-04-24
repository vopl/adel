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
		Packet _output;
		size_t _outputOffset;
	protected:
		bool push(const char *data, size_t size);
		bool flush(bool finish = false);
	};
}}}
#endif
