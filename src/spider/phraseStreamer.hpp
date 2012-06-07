#ifndef _SPIDER_PHRASESTREAMER_HPP_
#define _SPIDER_PHRASESTREAMER_HPP_

namespace spider
{
    template <size_t phraseVolume>
    class PhraseStreamer
    {
    public:
	PhraseStreamer(const std::deque<WordBucket> &text, const size_t tmpl[phraseVolume]);
	~PhraseStreamer();
	bool next(Phrase<phraseVolume> &phrase);
    }
}

#endif
