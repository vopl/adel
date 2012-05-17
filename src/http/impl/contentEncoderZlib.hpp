#ifndef _HTTP_IMPL_CONTENTENCODERZLIB_HPP_
#define _HTTP_IMPL_CONTENTENCODERZLIB_HPP_

#include "http/impl/contentEncoder.hpp"
#include "http/contentEncoding.hpp"
#include <zlib.h>

namespace http { namespace impl
{
	class ContentEncoderZlib
		: public ContentEncoder
	{
	public:
		ContentEncoderZlib(ContentEncoderPtr upstream, EContentEncoding ece, int level, size_t granula);
		virtual ~ContentEncoderZlib();

		virtual boost::system::error_code encoderPush(const net::Packet &packet, size_t offset=0);
		virtual boost::system::error_code encoderFlush();

	protected:
		ContentEncoderPtr	_upstream;
		EContentEncoding	_ece;
		int 				_level;
		size_t				_granula;

		z_stream	_z_stream;
		//gz_header	_gz_header;

		net::Packet	_output;
		size_t		_outputOffset;
	};
	typedef boost::shared_ptr<ContentEncoderZlib> ContentEncoderZlibPtr;
}}
#endif
