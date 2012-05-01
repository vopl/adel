#ifndef _NET_HTTP_IMPL_MESSAGE_HPP_
#define _NET_HTTP_IMPL_MESSAGE_HPP_

#include "net/packet.hpp"
#include "net/http/message.hpp"
#include "net/http/impl/messageBuffer.hpp"
#include <boost/enable_shared_from_this.hpp>

namespace net { namespace http { namespace impl
{
	///////////////////////////////////////////////////////////////
	class Message
		: public boost::enable_shared_from_this<Message>
	{
	public:
		Message();
		virtual ~Message();

		MessageIterator begin();
		MessageIterator end();
		MessageIterator endInfinity();

		const MessageIterator::size_type &size() const;
		virtual bool obtainMoreBuffers(bool force) =0;

		void dropTail(MessageIterator::size_type size);
		void dropFront();
	protected:
		MessageBufferPtr _firstBuffer;
		MessageBuffer 	*_lastBuffer;
		MessageIterator::size_type _size;
		void pushBuffer(const Packet &packet);

	};

	typedef boost::shared_ptr<Message> MessagePtr;

}}}

#endif
