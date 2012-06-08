#ifndef _SPIDER_PHRASESTREAMER_HPP_
#define _SPIDER_PHRASESTREAMER_HPP_

#include "spider/wordBucket.hpp"
#include "spider/phrase.hpp"

namespace spider
{
    template <size_t phraseVolume>
    class PhraseStreamer
    {
    public:
		PhraseStreamer(const std::deque<WordBucket> &text, const size_t tmpl[phraseVolume]);
		~PhraseStreamer();
		bool next(Phrase<phraseVolume> &phrase);
    };




	//////////////////////////////////////////////////////////////////////////
	template <size_t phraseVolume>
	PhraseStreamer<phraseVolume>::PhraseStreamer(const std::deque<WordBucket> &text, const size_t tmpl[phraseVolume])
	{
		assert(0);
	}

	//////////////////////////////////////////////////////////////////////////
	template <size_t phraseVolume>
	PhraseStreamer<phraseVolume>::~PhraseStreamer()
	{
		assert(0);
	}

	//////////////////////////////////////////////////////////////////////////
	template <size_t phraseVolume>
	bool PhraseStreamer<phraseVolume>::next(Phrase<phraseVolume> &phrase)
	{
		assert(0);
		return false;
	}

}

#endif
