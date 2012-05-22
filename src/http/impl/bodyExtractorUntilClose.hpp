#ifndef _HTTP_IMPL_BODYEXTRACTORUNTILCLOSE_HPP_
#define _HTTP_IMPL_BODYEXTRACTORUNTILCLOSE_HPP_

#include "http/impl/bodyExtractor.hpp"


namespace http { namespace impl{

	class BodyExtractorUntilClose
		: public BodyExtractor
	{
	public:
		BodyExtractorUntilClose(const ContentDecoderPtr &bodyDecoder, const ContentDecoderPtr &tailDecoder);
		virtual ~BodyExtractorUntilClose();

	private:
		virtual bool isDone();
		virtual boost::system::error_code push(const net::Packet &p, size_t offset);
	};
	typedef boost::shared_ptr<BodyExtractorUntilClose> BodyExtractorUntilClosePtr;
}}

#endif
