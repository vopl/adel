#ifndef _NET_HTTP_IMPL_CONTENTFILTER_HPP_
#define _NET_HTTP_IMPL_CONTENTFILTER_HPP_

#include <boost/shared_ptr.hpp>
#include "net/packet.hpp"

namespace net { namespace http { namespace impl
{
	class ContentFilter;
	typedef boost::shared_ptr<ContentFilter> ContentFilterPtr;
	
	class ContentFilter
	{
	public:
		ContentFilter();
		virtual ~ContentFilter();
		
		virtual bool filterPush(const Packet &packet, size_t offset=0)=0;
		virtual bool filterFlush()=0;

	protected:
	};
}}}
#endif
