#ifndef _HTTP_IMPL_CONTENTFILTERENCODEZLIB_HPP_
#define _HTTP_IMPL_CONTENTFILTERENCODEZLIB_HPP_

#include "http/impl/contentFilter.hpp"
#include "http/contentEncoding.hpp"
#include <zlib.h>

namespace http { namespace impl
{
	class ContentFilterEncodeZlib
		: public ContentFilter
	{
	public:
		ContentFilterEncodeZlib(ContentFilterPtr upstream, EContentEncoding ece, int level, size_t granula);
		virtual ~ContentFilterEncodeZlib();

		virtual bool filterPush(const net::Packet &packet, size_t offset=0);
		virtual bool filterFlush();

	protected:
		ContentFilterPtr	_upstream;
		EContentEncoding	_ece;
		int 				_level;
		size_t				_granula;

		z_stream	_z_stream;
		//gz_header	_gz_header;

		net::Packet	_output;
		size_t		_outputOffset;
	};
	typedef boost::shared_ptr<ContentFilterEncodeZlib> ContentFilterEncodeZlibPtr;
}}
#endif
