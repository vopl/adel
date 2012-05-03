#ifndef _NET_HTTP_IMPL_MESSAGEBUFFER_HPP_
#define _NET_HTTP_IMPL_MESSAGEBUFFER_HPP_

#include "net/packet.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_array.hpp>

namespace net { namespace http
{
	class MessageIterator;
}}

namespace net { namespace http { namespace impl
{
	class Message;
	class MessageBuffer;
	typedef boost::shared_ptr<MessageBuffer> MessageBufferPtr;

	class MessageBuffer
		: public boost::enable_shared_from_this<MessageBuffer>
	{
	public:
		MessageBuffer(
			Message *message,
			size_t offset,
			boost::shared_array<char> data,
			size_t dataBegin,
			size_t dataEnd);
		~MessageBuffer();

		Message *message() const;

		char *begin();
		char *end();

		bool hasNext();
		MessageBufferPtr next();
		bool hasPrev();
		MessageBufferPtr prev();

		size_t size() const;
		size_t offset() const;

		Packet asPacket(size_t &offsetInPacket);

		void moveFront(size_t distance);

	private:
		friend class Message;
		Message				*_message;

		MessageBufferPtr	_next;
		MessageBuffer		*_prev;

		size_t						_offset;
		boost::shared_array<char>	_data;
		char						*_begin;
		char						*_end;
	};
}}}


#endif
