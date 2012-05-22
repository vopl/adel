#ifndef _HTTP_IMPL_BODYEXTRACTORSIZED_HPP_
#define _HTTP_IMPL_BODYEXTRACTORSIZED_HPP_

#include "http/impl/bodyExtractor.hpp"


namespace http { namespace impl{

	class BodyExtractorSized
		: public BodyExtractor
	{
	public:
		BodyExtractorSized(const ContentDecoderPtr &bodyDecoder, const ContentDecoderPtr &tailDecoder, size_t targetSize);
		virtual ~BodyExtractorSized();

	private:
		virtual bool isDone();
		virtual boost::system::error_code push(const net::Packet &p, size_t offset);
	};
	typedef boost::shared_ptr<BodyExtractorSized> BodyExtractorSizedPtr;
}}

#endif
