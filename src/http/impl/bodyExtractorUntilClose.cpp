#include "pch.hpp"

#include "http/impl/bodyExtractorUntilClose.hpp"
#include "http/error.hpp"

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
	boost::system::error_code BodyExtractorUntilClose::read(net::Channel channel, size_t granula)
	{
		boost::system::error_code ec = BodyExtractor::read(channel, granula);

		if(ec)
		{
			if(ec == boost::asio::error::eof)
			{
				if((ec = _bodyDecoder->flush()))
				{
					return ec;
				}
				if((ec = _tailDecoder->flush()))
				{
					return ec;
				}
				return http::error::make();
			}
			return ec;
		}

		return http::error::make();
	}

	//////////////////////////////////////////////////////////////////////////
	bool BodyExtractorUntilClose::isDone()
	{
		return false;
	}
	
	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code BodyExtractorUntilClose::process(ContentDecoderAccumuler &data)
	{
		size_t bodySizeStub = std::numeric_limits<size_t>::max();
		return processBody(data, bodySizeStub);
	}

}}
