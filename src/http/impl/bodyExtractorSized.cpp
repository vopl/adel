#include "pch.hpp"

#include "http/impl/bodyExtractorSized.hpp"
#include "http/error.hpp"

namespace http { namespace impl{

	//////////////////////////////////////////////////////////////////////////
	BodyExtractorSized::BodyExtractorSized(const ContentDecoderPtr &bodyDecoder, const ContentDecoderPtr &tailDecoder, size_t targetSize)
		: BodyExtractor(bodyDecoder, tailDecoder)
		, _targetSize(targetSize)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	BodyExtractorSized::~BodyExtractorSized()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	bool BodyExtractorSized::isDone()
	{
		return _targetSize?false:true;
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code BodyExtractorSized::process(ContentDecoderAccumuler &data)
	{
		if(_targetSize)
		{
			return processBody(data, _targetSize);
		}
		return processTail(data);
	}

}}
