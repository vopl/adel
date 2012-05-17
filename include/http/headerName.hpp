#ifndef _HTTP_HEADERNAME_HPP_
#define _HTTP_HEADERNAME_HPP_

#include "http/impl/headerName.hpp"
# include <boost/range.hpp>

namespace http
{
	struct HeaderName
	{
		template <class Initier>
		HeaderName(Initier *stub)
			: csz(Initier::csz())
			, cszlc(Initier::cszlc())
			, str(Initier::str())
			, strlc(Initier::strlc())
			, key(Initier::key)
			, size(Initier::size)
		{
		}

		const char * const	csz;
		const char * const	cszlc;
		const std::string	&str;
		const std::string	&strlc;
		const size_t		key;
		const size_t		size;
	};
}

HTTP_HN_INSTANCE(date, 'D','a','t','e')
HTTP_HN_INSTANCE(server, 'S','e','r','v','e','r')

HTTP_HN_INSTANCE(host, 'H','o','s','t')

HTTP_HN_INSTANCE(connection, 'C','o','n','n','e','c','t','i','o','n')

HTTP_HN_INSTANCE(transferEncoding, 'T','r','a','n','s','f','e','r','-','E','n','c','o','d','i','n','g')
HTTP_HN_INSTANCE(te, 'T','E')
HTTP_HN_INSTANCE(contentEncoding, 'C','o','n','t','e','n','t','-','E','n','c','o','d','i','n','g')
HTTP_HN_INSTANCE(acceptEncoding, 'A','c','c','e','p','t','-','E','n','c','o','d','i','n','g')
HTTP_HN_INSTANCE(contentLength, 'C','o','n','t','e','n','t','-','L','e','n','g','t','h')
HTTP_HN_INSTANCE(contentType, 'C','o','n','t','e','n','t','-','T','y','p','e')

HTTP_HN_INSTANCE(eTag, 'E','T','a','g')
HTTP_HN_INSTANCE(ifMatch, 'I','f','-','M','a','t','c','h')
HTTP_HN_INSTANCE(ifNoneMatch, 'I','f','-','N','o','n','e','-','M','a','t','c','h')


HTTP_HN_INSTANCE(lastModified, 'L','a','s','t','-','M','o','d','i','f','i','e','d')
HTTP_HN_INSTANCE(ifModifiedSince, 'I','f','-','M','o','d','i','f','i','e','d','-','S','i','n','c','e')
HTTP_HN_INSTANCE(ifUnmodifiedSince, 'I','f','-','U','n','m','o','d','i','f','i','e','d','-','S','i','n','c','e')

HTTP_HN_INSTANCE(userAgent, 'U','s','e','r','-','A','g','e','n','t')

namespace http { namespace hn
{
	//////////////////////////////////////////////////////////////////////////////
	template <class Iterator>
	inline size_t key(Iterator begin, Iterator end)
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

	//////////////////////////////////////////////////////////////////////////////
	template <class Iterator>
	inline size_t key(const boost::iterator_range<Iterator> &range)
	{
		return hn::key(range.begin(), range.end());
	}

	//////////////////////////////////////////////////////////////////////////////
	inline size_t key(const std::string &str)
	{
		return key(str.begin(), str.end());
	}

	//////////////////////////////////////////////////////////////////////////////
	inline size_t key(const char *csz)
	{
		return key(csz, csz + strlen(csz));
	}

	//////////////////////////////////////////////////////////////////////////////
	inline size_t key(const char *data, size_t dataSize)
	{
		return key(data, data + dataSize);
	}

}}

#endif
