#ifndef _HTTP_IMPL_CONTENTDECODERZLIB_HPP_
#define _HTTP_IMPL_CONTENTDECODERZLIB_HPP_

#include "http/impl/contentDecoder.hpp"

namespace http { namespace impl
{
	class ContentDecoderZlib
		: public ContentDecoder
	{
	public:
		ContentDecoderZlib(ContentDecoder *upstream);
		virtual ~ContentDecoderZlib();

		virtual boost::system::error_code push(const net::Packet &packet, size_t offset=0);
		virtual boost::system::error_code flush();

	protected:
		ContentDecoder *_upstream;
	};
}}

#endif
