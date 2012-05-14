#ifndef _HTTP_IMPL_CONTENTFILTERBUFFERACCUMULER_HPP_
#define _HTTP_IMPL_CONTENTFILTERBUFFERACCUMULER_HPP_

#include "http/impl/contentFilter.hpp"
#include "http/impl/inputMessageBuffer.hpp"
#include "http/inputMessage.hpp"

namespace http { namespace impl
{
	class ContentFilterBufferAccumuler
		: public ContentFilter
	{
	public:
		ContentFilterBufferAccumuler();
		~ContentFilterBufferAccumuler();

		virtual bool filterPush(const net::Packet &packet, size_t offset=0);
		virtual bool filterFlush();

		InputMessageBuffer	*firstBuffer();
		InputMessageBuffer	*lastBuffer();

		http::InputMessage::Iterator	begin();
		http::InputMessage::Iterator	end();

	private:
		InputMessageBufferPtr	_first;
		InputMessageBuffer		*_last;
		size_t					_size;
	};

	typedef boost::shared_ptr<ContentFilterBufferAccumuler> ContentFilterBufferAccumulerPtr;
}}


#endif
