#ifndef _SPIDER_PHRASESTREAMER_HPP_
#define _SPIDER_PHRASESTREAMER_HPP_

#include "spider/wordBucket.hpp"
#include "spider/phrase.hpp"

namespace spider
{
    template <size_t phraseVolume, size_t sp1=0, size_t sp2=0, size_t sp3=0, size_t sp4=0, size_t sp5=0>
    class PhraseStreamer
    {
    public:
		PhraseStreamer(const std::deque<WordBucket> *text);
		~PhraseStreamer();
		bool next(Phrase<phraseVolume> &phrase);

	private:
		static const size_t areaSize = 
			phraseVolume + 
			(phraseVolume>1?sp1:0) +
			(phraseVolume>2?sp2:0) +
			(phraseVolume>3?sp3:0) +
			(phraseVolume>4?sp4:0) +
			(phraseVolume>5?sp5:0) +
			0;
		static const size_t wordIndices[6];
	private:
		const std::deque<WordBucket> &_text;
		size_t _pos;
    };


	template <size_t phraseVolume, size_t sp1, size_t sp2, size_t sp3, size_t sp4, size_t sp5>
	const size_t PhraseStreamer<phraseVolume, sp1, sp2, sp3, sp4, sp5>::wordIndices[6] = 
	{
		0,
		1+sp1,
		1+sp1+1+sp2,
		1+sp1+1+sp2+1+sp3,
		1+sp1+1+sp2+1+sp3+1+sp4,
		1+sp1+1+sp2+1+sp3+1+sp4+1+sp5,
	};


	//////////////////////////////////////////////////////////////////////////
	template <size_t phraseVolume, size_t sp1, size_t sp2, size_t sp3, size_t sp4, size_t sp5>
	PhraseStreamer<phraseVolume, sp1, sp2, sp3, sp4, sp5>::PhraseStreamer(const std::deque<WordBucket> *text)
		: _text(*text)
		, _pos(0)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	template <size_t phraseVolume, size_t sp1, size_t sp2, size_t sp3, size_t sp4, size_t sp5>
	PhraseStreamer<phraseVolume, sp1, sp2, sp3, sp4, sp5>::~PhraseStreamer()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	template <size_t phraseVolume, size_t sp1, size_t sp2, size_t sp3, size_t sp4, size_t sp5>
	bool PhraseStreamer<phraseVolume, sp1, sp2, sp3, sp4, sp5>::next(Phrase<phraseVolume> &phrase)
	{
		if(_text.size() < areaSize)
		{
			return false;
		}
		if(_pos > _text.size() - areaSize)
		{
			return false;
		}

		phrase.reset();
		for(size_t i(0); i<phraseVolume; i++)
		{
			phrase.setBucket(i, &_text[_pos + wordIndices[i]]);
		}
		_pos++;
		return true;
	}

}

#endif
