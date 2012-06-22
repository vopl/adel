#ifndef _SCOM_IMPL_WORD_HPP_
#define _SCOM_IMPL_WORD_HPP_

#include <boost/cstdint.hpp>

//#define WORD_WITH_SOURCE

namespace scom { namespace impl
{
    struct Word
    {
		typedef boost::uint32_t ValueType;
		typedef float WeightType;
#ifdef WORD_WITH_SOURCE
		std::string _text;
#endif
		ValueType	_value;
		WeightType	_weight;

		Word(const std::string &text);
		Word(const std::string &text, WeightType weight);

		bool operator<(const Word &w) const;

	private:
		void evalValue(const std::string &text);
    };
}}

#endif
