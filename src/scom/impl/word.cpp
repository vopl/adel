#include "pch.hpp"
#include "scom/impl/word.hpp"

#include <boost/crc.hpp>

namespace scom { namespace impl
{

	//////////////////////////////////////////////////////////////////////////
	Word::Word(const std::string &text)
		: _weight(1.0f)
#ifdef WORD_WITH_SOURCE
		, _text(text)
#endif
	{
		evalValue(text);
	}

	//////////////////////////////////////////////////////////////////////////
	Word::Word(const std::string &text, WeightType weight)
		: _weight(weight)
#ifdef WORD_WITH_SOURCE
		, _text(text)
#endif
	{
		evalValue(text);
	}

	//////////////////////////////////////////////////////////////////////////
	bool Word::operator<(const Word &w) const
	{
		return _value < w._value;
	}

	//////////////////////////////////////////////////////////////////////////
	void Word::evalValue(const std::string &text)
	{
		boost::crc_32_type  result;
		result.process_bytes(text.data(), text.size());
		_value = result.checksum();
	}

}}
