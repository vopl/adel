#ifndef _SPIDER_PHRASE_HPP_
#define _SPIDER_PHRASE_HPP_

namespace spider
{
    template <size_t volume>
    class Phrase
    {
    public:
	Phrase();
	~Phrase();

	bool addBucket(const WordBucket *bucket);
	bool isFull() const;

	size_t getCombinationsAmount() const;
	bool getCombination(const Word * (&words)[volume], size_t idx) const;
    };
}

#endif
