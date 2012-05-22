#include "pch.hpp"

#include "http/impl/bodyExtractorUntilClose.hpp"

namespace http { namespace impl{

	//////////////////////////////////////////////////////////////////////////
	BodyExtractorUntilClose::BodyExtractorUntilClose(const ContentDecoderPtr &bodyDecoder, const ContentDecoderPtr &tailDecoder)
		: BodyExtractor(bodyDecoder, tailDecoder)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	BodyExtractorUntilClose::~BodyExtractorUntilClose()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	bool BodyExtractorUntilClose::isDone()
	{
		assert(0);
		return true;
	}
	
	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code BodyExtractorUntilClose::push(const net::Packet &p, size_t offset)
	{
		assert(0);
		return boost::system::error_code();
	}

}}
