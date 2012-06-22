#ifndef _SCOM_IMPL_TEXTPARSER_HPP_
#define _SCOM_IMPL_TEXTPARSER_HPP_

#include "scom/impl/wordBucket.hpp"

#ifdef near
#	undef near
#endif
#include <hunspell.hxx>

namespace scom { namespace impl
{
    //перегоняет линейный текст в букеты слов
    /*
	побить по небуквам
	бросить стопы
    */
    class TextParser
    {
    public:
		TextParser(Hunspell *hunspell, std::deque<WordBucket> &wordBuckets);
		~TextParser();

		void push(const std::string &text);

		const std::deque<WordBucket> &result();

	private:
		void pushWord(const std::vector<boost::int32_t> &word);
		void pushWord(const std::string &word);
		std::string stripHunspellTech(const std::string &data);

	private:
		Hunspell *_hunspell;
		std::deque<WordBucket> &_data;
    };
}}

#endif
