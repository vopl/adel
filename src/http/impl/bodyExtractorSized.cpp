#include "pch.hpp"

#include "http/impl/bodyExtractorSized.hpp"

namespace http { namespace impl{

	//////////////////////////////////////////////////////////////////////////
	BodyExtractorSized::BodyExtractorSized(const ContentDecoderPtr &bodyDecoder, const ContentDecoderPtr &tailDecoder, size_t targetSize)
		: BodyExtractor(bodyDecoder, tailDecoder)
	{
		assert(0);
	}

	//////////////////////////////////////////////////////////////////////////
	BodyExtractorSized::~BodyExtractorSized()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	bool BodyExtractorSized::isDone()
	{
		assert(0);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code BodyExtractorSized::push(const net::Packet &p, size_t offset)
	{
		assert(0);
		return boost::system::error_code();
	}

}}
