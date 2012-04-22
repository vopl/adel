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

		virtual boost::uint32_t filterPush(const Packet &packet, boost::uint32_t offset=0);
		virtual boost::uint32_t filterFlush();

	protected:
	};
}}}
#endif
