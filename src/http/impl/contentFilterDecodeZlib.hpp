#ifndef _HTTP_IMPL_CONTENTFILTERDECODEZLIB_HPP_
#define _HTTP_IMPL_CONTENTFILTERDECODEZLIB_HPP_

#include "http/impl/contentFilter.hpp"

namespace http { namespace impl
{
	class ContentFilterDecodeZlib
		: public ContentFilter
	{
	public:
		ContentFilterDecodeZlib(ContentFilter *upstream);
		virtual ~ContentFilterDecodeZlib();

		virtual boost::system::error_code filterPush(const net::Packet &packet, size_t offset=0);
		virtual boost::system::error_code filterFlush();

	protected:
		ContentFilter *_upstream;
	};
}}

#endif
