#ifndef _SPIDER_PHRASE_HPP_
#define _SPIDER_PHRASE_HPP_

#include "spider/wordBucket.hpp"

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





	//////////////////////////////////////////////////////////////////////////
	template <size_t volume>
	Phrase<volume>::Phrase()
	{
		assert(0);
	}

	//////////////////////////////////////////////////////////////////////////
	template <size_t volume>
	Phrase<volume>::~Phrase()
	{
		assert(0);
	}

	//////////////////////////////////////////////////////////////////////////
	template <size_t volume>
	bool Phrase<volume>::addBucket(const WordBucket *bucket)
	{
		assert(0);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	template <size_t volume>
	bool Phrase<volume>::isFull() const
	{
		assert(0);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	template <size_t volume>
	size_t Phrase<volume>::getCombinationsAmount() const
	{
		assert(0);
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	template <size_t volume>
	bool Phrase<volume>::getCombination(const Word * (&words)[volume], size_t idx) const
	{
		assert(0);
		return false;
	}

}

#endif
