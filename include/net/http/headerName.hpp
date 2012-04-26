#ifndef _NET_HTTP_HN_HPP_
#define _NET_HTTP_HN_HPP_

#include "net/http/impl/headerName.hpp"

namespace net { namespace http
{
	struct HeaderName
	{
		template <class Initier>
		HeaderName(Initier *stub)
			: csz(Initier::csz())
			, cszlc(Initier::cszlc())
			, str(Initier::str())
			, strlc(Initier::strlc())
			, hash(Initier::hash)
		{
		}

		const char * const	csz;
		const char * const	cszlc;
		const std::string	&str;
		const std::string	&strlc;
		const size_t		hash;
	};
}}

NET_HTTP_HN_INSTANCE(date, 'D','a','t','e')
NET_HTTP_HN_INSTANCE(server, 'S','e','r','v','e','r')

NET_HTTP_HN_INSTANCE(connection, 'C','o','n','n','e','c','t','i','o','n')

NET_HTTP_HN_INSTANCE(transferEncoding, 'T','r','a','n','s','f','e','r','-','E','n','c','o','d','i','n','g')
NET_HTTP_HN_INSTANCE(contentEncoding, 'C','o','n','t','e','n','t','-','E','n','c','o','d','i','n','g')
NET_HTTP_HN_INSTANCE(contentLength, 'C','o','n','t','e','n','t','-','L','e','n','g','t','h')
NET_HTTP_HN_INSTANCE(contentType, 'C','o','n','t','e','n','t','-','T','y','p','e')

NET_HTTP_HN_INSTANCE(eTag, 'E','T','a','g')
NET_HTTP_HN_INSTANCE(ifMatch, 'I','f','-','M','a','t','c','h')
NET_HTTP_HN_INSTANCE(ifNoneMatch, 'I','f','-','N','o','n','e','-','M','a','t','c','h')


NET_HTTP_HN_INSTANCE(lastModified, 'L','a','s','t','-','M','o','d','i','f','i','e','d')
NET_HTTP_HN_INSTANCE(ifModifiedSince, 'I','f','-','M','o','d','i','f','i','e','d','-','S','i','n','c','e')
NET_HTTP_HN_INSTANCE(ifUnmodifiedSince, 'I','f','-','U','n','m','o','d','i','f','i','e','d','-','S','i','n','c','e')

namespace net { namespace http { namespace hn
{
	template <class Iterator>
	inline size_t hash(Iterator begin, Iterator end)
	{
		size_t seed = 0;

		for(; begin != end; ++begin)
		{
			char ch = *begin;
			if(ch >= 'A' && ch <= 'Z')
			{
				ch += 'a' - 'A';
			}
			boost::hash_combine(seed, ch);
		}

		return seed;
	}

	inline size_t hash(const std::string &str)
	{
		return hash(str.begin(), str.end());
	}

	inline size_t hash(const char *csz)
	{
		return hash(csz, csz + strlen(csz));
	}
}}}

#endif
