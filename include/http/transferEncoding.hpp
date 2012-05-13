#ifndef _HTTP_TRANSFERENCODING_HPP_
#define _HTTP_TRANSFERENCODING_HPP_

namespace http
{

	enum ETransferEncoding
	{
		ete_unknown		= 0,
		ete_identity	= 1<<1,
		ete_chunked		= 1<<2,
		ete_gzip		= 1<<3,
		ete_compress	= 1<<4,
		ete_deflate		= 1<<5,
		ete_any			= ete_deflate|ete_compress|ete_gzip|ete_chunked,
	};
}

#endif
