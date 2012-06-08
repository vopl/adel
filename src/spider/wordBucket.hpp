#ifndef _SPIDER_WORDBUCKET_HPP_
#define _SPIDER_WORDBUCKET_HPP_

#include "spider/word.hpp"
#include <vector>

namespace spider
{
    struct WordBucket
    {
		std::vector<Word> _words;
    };
}

#endif
