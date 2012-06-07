#ifndef _SPIDER_TEXTPARSER_HPP_
#define _SPIDER_TEXTPARSER_HPP_

namespace spider
{
    //перегоняет линейный текст в букеты слов
    /*
	побить по небуквам
	бросить стопы
    */
    class TextParser
    {
    public:
	TextParser(const char *affpath, const char *dicpath);
	~TextParser();

	void parse(text);
	const std::deque<WordBucket> &result();
    };
}

#endif
