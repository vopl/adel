#ifndef _HTTP_IMPL_CONTENTFILTERDECODECHUNKED_HPP_
#define _HTTP_IMPL_CONTENTFILTERDECODECHUNKED_HPP_

#include "http/impl/contentFilter.hpp"

namespace http { namespace impl
{
	class ContentFilterDecodeChunked
		: public ContentFilter
	{
	public:
		ContentFilterDecodeChunked(ContentFilter *upstream);
		virtual ~ContentFilterDecodeChunked();

		virtual bool filterPush(const net::Packet &packet, size_t offset=0);
		virtual bool filterFlush();

	protected:
		ContentFilter *_upstream;
	};
}}

#endif