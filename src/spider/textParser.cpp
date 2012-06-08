#include "pch.hpp"
#include "spider/textParser.hpp"
#include "utf8proc/utf8proc.h"

#include <boost/shared_array.hpp>

namespace spider
{
	//////////////////////////////////////////////////////////////////////////
	TextParser::TextParser(Hunspell *hunspell)
		: _hunspell(hunspell)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	TextParser::~TextParser()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void TextParser::push(const std::string &text)
	{
		//нормализовать, разбить по словам
		size_t bufSize = text.size()*3+16;
		boost::shared_array<int32_t> buf(new int32_t[bufSize]);
		bufSize = utf8proc_decompose((const uint8_t *)text.data(), text.length(), buf.get(), bufSize, UTF8PROC_IGNORE|UTF8PROC_STABLE|UTF8PROC_REJECTNA|UTF8PROC_LUMP);

		enum EWordType
		{
			ewtNull,
			ewtLetters,
			ewtSymbols
		} ewt = ewtNull;
		std::vector<int32_t> wordChars;

		for(size_t i(0); i<bufSize; i++)
		{
			const utf8proc_property_t *prop = utf8proc_get_property(buf[i]);
			if(!prop)
			{
				assert(0);
				continue;
			}

			bool space = false;
			int32_t letter = 0;
			int32_t symbol = 0;
			switch(prop->category)
			{
			case UTF8PROC_CATEGORY_LU: //Letter, Uppercase
				letter = prop->lowercase_mapping;
				break;
			case UTF8PROC_CATEGORY_LL: //Letter, lowercase
				letter = buf[i];
				break;
			case UTF8PROC_CATEGORY_LT: //Letter, titlecase
				assert(!"check me");
				letter = prop->lowercase_mapping;
				break;
			case UTF8PROC_CATEGORY_LO: //Letter, other
			case UTF8PROC_CATEGORY_LM: //Letter, modifier
				break;
			case UTF8PROC_CATEGORY_ND: //Number, decimal digit
			case UTF8PROC_CATEGORY_NL: //Number, letter
			case UTF8PROC_CATEGORY_NO: //Number other
				symbol = 'N';
				break;
			case UTF8PROC_CATEGORY_SM: //Symbol, math
			case UTF8PROC_CATEGORY_SC: //Symbol, currency
			case UTF8PROC_CATEGORY_SK: //Symbol, modifier
			case UTF8PROC_CATEGORY_SO: //Symbol, other
				symbol = buf[i];
				break;

			case UTF8PROC_CATEGORY_PC: //Punctuation, connector
			case UTF8PROC_CATEGORY_PD: //Punctuation, dash
			case UTF8PROC_CATEGORY_PS: //Punctuation, open
			case UTF8PROC_CATEGORY_PE: //Punctuation, close
			case UTF8PROC_CATEGORY_PI: //Punctuation, initial quote
			case UTF8PROC_CATEGORY_PF: //Punctuation, final quote
			case UTF8PROC_CATEGORY_PO: //Punctuation, other
				symbol = buf[i];
				break;
			case UTF8PROC_CATEGORY_MN: //Mark, non-spacing
			case UTF8PROC_CATEGORY_MC: //Mark, spacing combining
			case UTF8PROC_CATEGORY_ME: //Mark, enclosing
				symbol = buf[i];
				break;
			case UTF8PROC_CATEGORY_ZS: //Separator, space
			case UTF8PROC_CATEGORY_ZL: //Seaprator, line
			case UTF8PROC_CATEGORY_ZP: //Seaparator, paragraph
			case UTF8PROC_CATEGORY_CC: //Other, control
			case UTF8PROC_CATEGORY_CF: //Other, format
			case UTF8PROC_CATEGORY_CS: //Other, surrogate
			case UTF8PROC_CATEGORY_CO: //Other, private use
			case UTF8PROC_CATEGORY_CN: //Other, not assigned
			default:
				space = true;
				break;
			}

			switch(ewt)
			{
			case ewtNull:
				if(letter)
				{
					ewt = ewtLetters;
					wordChars.push_back(letter);
				}
				else if(symbol)
				{
					ewt = ewtSymbols;
					wordChars.push_back(symbol);
				}
				break;

			case ewtLetters:
				if(letter)
				{
					wordChars.push_back(letter);
				}
				else if(symbol)
				{
					pushWord(wordChars);
					wordChars.clear();
					ewt = ewtSymbols;
					wordChars.push_back(symbol);
				}
				break;
			case ewtSymbols:
				if(letter)
				{
					pushWord(wordChars);
					wordChars.clear();
					ewt = ewtLetters;
					wordChars.push_back(letter);
				}
				else if(symbol)
				{
					wordChars.push_back(symbol);
				}
				break;
			default:
				assert(0);
				break;
			}
			if(space)
			{
				pushWord(wordChars);
				wordChars.clear();
				ewt = ewtNull;
			}

		}
		pushWord(wordChars);
		wordChars.clear();
		ewt = ewtNull;

	}

	//////////////////////////////////////////////////////////////////////////
	const std::deque<WordBucket> &TextParser::result()
	{
		return _data;
	}


	//////////////////////////////////////////////////////////////////////////
	void TextParser::pushWord(const std::vector<int32_t> &word)
	{
		if(!word.empty())
		{
			std::string encoded;
			encoded.resize(word.size()*3+16);
			char *data = const_cast<char *>(encoded.data());

			for(size_t i(0); i<word.size(); i++)
			{
				data += utf8proc_encode_char(word[i], (uint8_t *)data);
			}
			encoded.resize(data - encoded.data());
			pushWord(encoded);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void TextParser::pushWord(const std::string &word)
	{
		std::set<Word> means;

		char ** result, **result2;
		int ns;
		///////////////////////////////////////////
		/*if(word.size() > 2)
		{
			ns = _hunspell->suggest(&result, word.c_str());
			if(ns)
			{
				for(size_t i(0); i<ns; i++)
				{
					means.insert(Word(result[i], 0.5f));
				}
				_hunspell->free_list(&result, ns);
			}
		}*/

		///////////////////////////////////////////
		ns = _hunspell->analyze(&result, word.c_str());
		if(ns)
		{
			for(size_t i(0); i<ns; i++)
			{
				means.insert(Word(stripHunspellTech(result[i]), 1.3f));
			}

			///////////////////////////////////////////
			int ns2 = _hunspell->stem(&result2, result, ns);
			if(ns2)
			{
				for(size_t i(0); i<ns2; i++)
				{
					means.insert(Word(stripHunspellTech(result2[i]), 1.3f));
				}
				_hunspell->free_list(&result2, ns2);
			}
			_hunspell->free_list(&result, ns);
		}

		means.insert(Word(word.c_str(), 0.7f));

		//слить
		{
			std::set<Word>::iterator iter = means.begin();
			std::set<Word>::iterator end = means.end();

			WordBucket wb;
			size_t idx(0);
			for(; iter!=end; ++iter)
			{
				wb._words.push_back(*iter);
			}
			_data.push_back(wb);
		}

	}

	//////////////////////////////////////////////////////////////////////////
	std::string TextParser::stripHunspellTech(const std::string &data)
	{
		std::string res(data);
		std::string::size_type pos = res.find("st:");
		if(std::string::npos != pos)
		{
			res.erase(res.begin(), res.begin()+pos+3);
		}
		pos = res.find(" #");
		if(std::string::npos != pos)
		{
			res.erase(res.begin()+pos, res.end());
		}

		return res;
	}

}
