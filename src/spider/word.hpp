#ifndef _SPIDER_WORD_HPP_
#define _SPIDER_WORD_HPP_

#include <boost/cstdint.hpp>

namespace spider
{
    struct Word
    {
		typedef boost::uint32_t RawType;
		RawType _value;

		Word(RawType value)
			: _value(value)
		{
		}
    };
}

#endif
