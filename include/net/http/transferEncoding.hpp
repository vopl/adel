#ifndef _NET_HTTP_TRANSFERENCODING_HPP_
#define _NET_HTTP_TRANSFERENCODING_HPP_

namespace net { namespace http
{

	enum ETransferEncoding
	{
		ete_identity,
		ete_chunked,
		ete_gzip,
		ete_compress,
		ete_deflate,
	};
}}

#endif
