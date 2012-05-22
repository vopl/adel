#ifndef _HTTP_IMPL_CONTENTENCODER_HPP_
#define _HTTP_IMPL_CONTENTENCODER_HPP_

#include <boost/shared_ptr.hpp>
#include "net/packet.hpp"

namespace http { namespace impl
{
	class ContentEncoder;
	typedef boost::shared_ptr<ContentEncoder> ContentEncoderPtr;
	
	class ContentEncoder
	{
	public:
		ContentEncoder();
		virtual ~ContentEncoder();
		
		virtual boost::system::error_code push(const net::Packet &packet, size_t offset=0)=0;
		virtual boost::system::error_code flush()=0;

	protected:
	};
}}
#endif
