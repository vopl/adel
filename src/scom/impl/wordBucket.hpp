#ifndef _SCOM_IMPL_WORDBUCKET_HPP_
#define _SCOM_IMPL_WORDBUCKET_HPP_

#include "scom/impl/word.hpp"
#include <vector>

namespace scom { namespace impl
{
    struct WordBucket
    {
		std::vector<Word> _words;
    };
}}

#endif
