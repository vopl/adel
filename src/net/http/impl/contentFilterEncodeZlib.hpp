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

		virtual bool filterPush(const Packet &packet, size_t offset=0);
		virtual bool filterFlush();

	protected:
	};
}}}
#endif
