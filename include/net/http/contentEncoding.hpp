#ifndef _NET_HTTP_CONTENTENCODING_HPP_
#define _NET_HTTP_CONTENTENCODING_HPP_

namespace net { namespace http
{

	enum EContentEncoding
	{
		ece_unknown,
		ece_identity,
		ece_gzip,
		ece_compress,
		ece_deflate,
	};
}}

#endif
