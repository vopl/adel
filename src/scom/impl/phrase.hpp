#ifndef _SCOM_IMPL_PHRASE_HPP_
#define _SCOM_IMPL_PHRASE_HPP_

#include "scom/impl/wordBucket.hpp"

namespace scom { namespace impl
{
    template <size_t volume>
    class Phrase
    {
    public:
		Phrase();
		~Phrase();

		void reset();
		void setBucket(size_t idx, const WordBucket *bucket);

		size_t getCombinationsAmount() const;
		bool nextCombination(const Word * (&words)[volume]);

	private:
		const WordBucket *_buckets[volume];
		size_t _wordIndices[volume];
    };





	//////////////////////////////////////////////////////////////////////////
	template <size_t volume>
	Phrase<volume>::Phrase()
	{
		reset();
	}

	//////////////////////////////////////////////////////////////////////////
	template <size_t volume>
	Phrase<volume>::~Phrase()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	template <size_t volume>
	void Phrase<volume>::reset()
	{
		memset(_buckets, 0, sizeof(_buckets));
		memset(_wordIndices, 0, sizeof(_wordIndices));
	}

	//////////////////////////////////////////////////////////////////////////
	template <size_t volume>
	void Phrase<volume>::setBucket(size_t idx, const WordBucket *bucket)
	{
		_buckets[idx] = bucket;
	}

	//////////////////////////////////////////////////////////////////////////
	template <size_t volume>
	size_t Phrase<volume>::getCombinationsAmount() const
	{
		size_t amount = 0;
		for(size_t i(0); i<volume; i++)
		{
			assert(_buckets[i]);
			amount += _buckets[i]->_words.size();
		}
		return amount;
	}

	//////////////////////////////////////////////////////////////////////////
	template <size_t volume>
	bool Phrase<volume>::nextCombination(const Word * (&words)[volume])
	{
		
		if(_wordIndices[volume-1] >= _buckets[volume-1]->_words.size())
		{
			return false;
		}

		for(size_t i(0); i<volume; i++)
		{
			words[i] = &_buckets[i]->_words[_wordIndices[i]];
		}

		_wordIndices[0]++;
		for(size_t i(0); i<volume-1; i++)
		{
			if(_wordIndices[i] >= _buckets[i]->_words.size())
			{
				_wordIndices[i] = 0;
				_wordIndices[i+1]++;
			}
		}
		return true;
	}

}}

#endif
