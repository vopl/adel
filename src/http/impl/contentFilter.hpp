#ifndef _HTTP_IMPL_CONTENTFILTER_HPP_
#define _HTTP_IMPL_CONTENTFILTER_HPP_

#include <boost/shared_ptr.hpp>
#include "net/packet.hpp"

namespace http { namespace impl
{
	class ContentFilter;
	typedef boost::shared_ptr<ContentFilter> ContentFilterPtr;
	
	class ContentFilter
	{
	public:
		ContentFilter();
		virtual ~ContentFilter();
		
		virtual boost::system::error_code filterPush(const net::Packet &packet, size_t offset=0)=0;
		virtual boost::system::error_code filterFlush()=0;

	protected:
	};
}}
#endif
