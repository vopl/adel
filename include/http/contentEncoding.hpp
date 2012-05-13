#ifndef _HTTP_CONTENTENCODING_HPP_
#define _HTTP_CONTENTENCODING_HPP_

namespace http
{

	enum EContentEncoding
	{
		ece_unknown		= 0,
		ece_identity	= 1<<1,
		ece_gzip		= 1<<2,
		ece_compress	= 1<<3,
		ece_deflate		= 1<<4,
		ece_any			= ece_identity|ece_gzip|ece_compress|ece_deflate,
	};
}

#endif
