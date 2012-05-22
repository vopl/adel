#ifndef _HTTP_IMPL_CONTENTDECODERZLIB_HPP_
#define _HTTP_IMPL_CONTENTDECODERZLIB_HPP_

#include "http/impl/contentDecoder.hpp"
#include "http/contentEncoding.hpp"
#include <zlib.h>

namespace http { namespace impl
{
	class ContentDecoderZlib
		: public ContentDecoder
	{
	public:
		ContentDecoderZlib(const ContentDecoderPtr &upstream, EContentEncoding ece, size_t granula);
		virtual ~ContentDecoderZlib();

		virtual boost::system::error_code push(const net::Packet &packet, size_t offset=0);
		virtual boost::system::error_code flush();

	protected:
		ContentDecoderPtr	_upstream;
		EContentEncoding	_ece;
		size_t _granula;

		bool		_streamEnd;
		z_stream	_z_stream;
		net::Packet	_output;
		size_t		_outputOffset;

	};
}}

#endif
