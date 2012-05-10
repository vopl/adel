#ifndef _NET_HTTP_IMPL_INPUTMESSAGEBUFFER_HPP_
#define _NET_HTTP_IMPL_INPUTMESSAGEBUFFER_HPP_

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_array.hpp>

namespace net { namespace http { namespace impl
{
	class InputMessage;
	class InputMessageBuffer;
	typedef boost::shared_ptr<InputMessageBuffer> InputMessageBufferPtr;

	class InputMessageBuffer
		: public boost::enable_shared_from_this<InputMessageBuffer>
	{
	public:
		InputMessageBuffer(
			size_t offset,
			boost::shared_array<char> data,
			size_t begin,
			size_t end);
		~InputMessageBuffer();

		size_t offset() const;
		size_t size() const;

		const char *begin();
		const char *end();

		InputMessageBuffer *next();
		InputMessageBuffer *prev();

		void setNext(const InputMessageBufferPtr &b);
		void setPrev(InputMessageBuffer *b);

	private:
		size_t						_offset;
		boost::shared_array<char>	_data;
		const char					*_begin;
		const char					*_end;

		InputMessageBufferPtr	_next;
		InputMessageBuffer		*_prev;
	};
}}}


#endif
