#ifndef _NET_HTTP_HN_HPP_
#define _NET_HTTP_HN_HPP_

#include "net/http/impl/headerName.hpp"





NET_HTTP_HN_INSTANCE(date, "Date",
	'date')

NET_HTTP_HN_INSTANCE(ifModifiedSince, "If-Modified-Since",
	'if-m','odif','ied-','sins','e')

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
