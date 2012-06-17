
///////////////////////////////////////////////////////////////////
struct Page
{
	Uri	_uri;
	
	template <size_t phraseVolume>
	struct Page2Phrase
	{
		Phrase<phraseVolume>	*_phrase;
		size_t			_amount;
	};
	deque<Page2Phrase<1> > _cross1;
	deque<Page2Phrase<2> > _cross2;
	deque<Page2Phrase<3> > _cross3;
};
typedef boost::shared_ptr<Page> PagePtr;

///////////////////////////////////////////////////////////////////
template <size_t phraseVolume>
struct Phrase
{
	Word[phraseVolume]	_words;
	PagePtr			_originalPage;
	
	struct Phrase2Page
	{
		PagePtr	_page;
		size_t	_amount;
	};
	deque<Page2Page> _cross;
};

///////////////////////////////////////////////////////////////////
struct Arena
{
	deque<PagePtr>		_pages;
	deque<Phrase<1> >	_phrases1;
	deque<Phrase<2> >	_phrases2;
	deque<Phrase<3> >	_phrases3;
};
/*
1. цикл по добавлению новых страниц
    1. добавить страницу на арену
    2. получить слова, 3 цикла по стримерам слов -> фразы
	3. добавить фразу на арену, ей прописать оригинальную страницу
2. сортировать фразы
3. перебор диапазонов одинаковых фраз
    1. выделить набор страниц
    2. перебор фраз в диапазоне
	1. фразе добавить в кросс все страницы из диапазона
4. перебор страниц
    1. сортировать кроссы по количеству фраз
    2. бросить фразы < 97%
5. сформировать отчет
*/
