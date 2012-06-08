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
		bufSize = utf8proc_decompose((const uint8_t *)text.data(), text.length(), buf.get(), bufSize, UTF8PROC_DECOMPOSE|UTF8PROC_IGNORE|UTF8PROC_STABLE|UTF8PROC_COMPAT|UTF8PROC_CASEFOLD|UTF8PROC_STRIPMARK);

		std::vector<int32_t> wordChars;

		for(size_t i(0); i<bufSize; i++)
		{
			const utf8proc_property_t *prop = utf8proc_get_property(buf[i]);
			if(!prop)
			{
				assert(0);
				continue;
			}

			switch(prop->category)
			{
			case UTF8PROC_CATEGORY_LU: //Letter, Uppercase
				wordChars.push_back(prop->lowercase_mapping);
				break;
			case UTF8PROC_CATEGORY_LL: //Letter, lowercase
				wordChars.push_back(buf[i]);
				break;
			case UTF8PROC_CATEGORY_LT: //Letter, titlecase
				assert(!"check me");
				wordChars.push_back(prop->lowercase_mapping);
				break;
			case UTF8PROC_CATEGORY_LO: //Letter, other
			case UTF8PROC_CATEGORY_LM: //Letter, modifier
			case UTF8PROC_CATEGORY_ND: //Number, decimal digit
			case UTF8PROC_CATEGORY_NL: //Number, letter
			case UTF8PROC_CATEGORY_NO: //Number other
			case UTF8PROC_CATEGORY_SM: //Symbol, math
			case UTF8PROC_CATEGORY_SC: //Symbol, currency
			case UTF8PROC_CATEGORY_SK: //Symbol, modifier
			case UTF8PROC_CATEGORY_SO: //Symbol, other
			case UTF8PROC_CATEGORY_MN: //Mark, non-spacing
			case UTF8PROC_CATEGORY_MC: //Mark, spacing combining
			case UTF8PROC_CATEGORY_ME: //Mark, enclosing
			case UTF8PROC_CATEGORY_ZS: //Separator, space
			case UTF8PROC_CATEGORY_ZL: //Seaprator, line
			case UTF8PROC_CATEGORY_ZP: //Seaparator, paragraph
			case UTF8PROC_CATEGORY_PC: //Punctuation, connector
			case UTF8PROC_CATEGORY_PD: //Punctuation, dash
			case UTF8PROC_CATEGORY_PS: //Punctuation, open
			case UTF8PROC_CATEGORY_PE: //Punctuation, close
			case UTF8PROC_CATEGORY_PI: //Puntuation, initial quote
			case UTF8PROC_CATEGORY_PF: //Punctuation, final quote
			case UTF8PROC_CATEGORY_PO: //Punctuation, other
			case UTF8PROC_CATEGORY_CC: //Other, control
			case UTF8PROC_CATEGORY_CF: //Other, format
			case UTF8PROC_CATEGORY_CS: //Other, surrogate
			case UTF8PROC_CATEGORY_CO: //Other, private use
			case UTF8PROC_CATEGORY_CN: //Other, not assigned
			default:
				if(!wordChars.empty())
				{
					std::string encoded;
					encoded.resize(wordChars.size()*3+16);
					char *data = const_cast<char *>(encoded.data());

					for(size_t i(0); i<wordChars.size(); i++)
					{
						data += utf8proc_encode_char(wordChars[i], (uint8_t *)data);
					}
					encoded.resize(data - encoded.data());
					pushWord(encoded);
					wordChars.clear();
				}
				break;
			}
		}

	}

	//////////////////////////////////////////////////////////////////////////
	const std::deque<WordBucket> &TextParser::result()
	{
		return _data;
	}


	//////////////////////////////////////////////////////////////////////////
	namespace
	{
		boost::uint32_t hashWord(const char *src)
		{
			boost::crc_32_type  result;
			for(; *src; src++)
			{
				result.process_byte(*src);
			}

			return result.checksum();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void TextParser::pushWord(const std::string &word)
	{
		std::cout<<"---------\n"<<word<<std::endl;
		std::set<boost::uint32_t> means;

		char ** result, **result2;
		///////////////////////////////////////////
		int ns = _hunspell->suggest(&result, word.data());
		if(ns)
		{
			for(size_t i(0); i<ns; i++)
			{
				std::cout<<"sugg: "<<result[i]<<std::endl;
				means.insert(hashWord(result[i]));
			}
			_hunspell->free_list(&result, ns);
		}

		///////////////////////////////////////////
		ns = _hunspell->analyze(&result, word.data());
		if(ns)
		{
			for(size_t i(0); i<ns; i++)
			{
				std::cout<<"anal: "<<result[i]<<std::endl;
				means.insert(hashWord(result[i]));
			}

			///////////////////////////////////////////
			int ns2 = _hunspell->stem(&result2, result, ns);
			if(ns2)
			{
				for(size_t i(0); i<ns; i++)
				{
					std::cout<<"stem: "<<result[i]<<std::endl;
					means.insert(hashWord(result2[i]));
				}
				_hunspell->free_list(&result2, ns2);
			}
			_hunspell->free_list(&result, ns);
		}


		if(means.empty())
		{
			means.insert(hashWord(word.data()));
		}

		//слить
		{
			std::set<boost::uint32_t>::iterator iter = means.begin();
			std::set<boost::uint32_t>::iterator end = means.end();

			WordBucket wb;
			size_t idx(0);
			for(; iter!=end; ++iter)
			{
				wb._words.push_back(Word(*iter));
			}
			_data.push_back(wb);
		}

	}

}
