#ifndef _HTTP_IMPL_CONTENTDECODER_HPP_
#define _HTTP_IMPL_CONTENTDECODER_HPP_

#include <boost/shared_ptr.hpp>
#include "net/packet.hpp"

namespace http { namespace impl
{
	class ContentDecoder;
	typedef boost::shared_ptr<ContentDecoder> ContentDecoderPtr;
	
	class ContentDecoder
	{
	public:
		ContentDecoder();
		virtual ~ContentDecoder();
		
		virtual boost::system::error_code decoderPush(const net::Packet &packet, size_t offset=0)=0;
		virtual boost::system::error_code decoderFlush()=0;

	protected:
	};
}}
#endif
