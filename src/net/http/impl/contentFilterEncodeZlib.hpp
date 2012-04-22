#ifndef _NET_HTTP_IMPL_CONTENTFILTERENCODEZLIB_HPP_
#define _NET_HTTP_IMPL_CONTENTFILTERENCODEZLIB_HPP_

#include "net/http/impl/contentFilter.hpp"

namespace net { namespace http { namespace impl
{
	class ContentFilterEncodeZlib
		: public ContentFilter
	{
	public:
		ContentFilterEncodeZlib(ContentFilter* upstream);
		virtual ~ContentFilterEncodeZlib();

		virtual size_t filterPush(const Packet &packet, size_t offset=0);
		virtual size_t filterFlush();

	protected:
	};
}}}
#endif
