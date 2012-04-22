#ifndef _NET_IMPL_MESSAGE_HPP_
#define _NET_IMPL_MESSAGE_HPP_

#include "net/packet.hpp"
#include "net/message.hpp"
#include <boost/enable_shared_from_this.hpp>

namespace net { namespace impl
{
	///////////////////////////////////////////////////////////////
	class Message
		: public boost::enable_shared_from_this<Message>
	{
	public:
		typedef net::Message::Iterator Iterator;
		struct SChunk
		{
			Iterator::difference_type	_offset;
			Packet						_packet;
		};
		typedef std::vector<SChunk> TVChunks;

	public:
		Message();
		virtual ~Message();

		Iterator begin();
		Iterator end();
		Iterator endInfinity();

		TVChunks &chunks();
		const Iterator::difference_type &size() const;
		virtual bool obtainMoreChunks() =0;

	protected:
		TVChunks _chunks;
		Iterator::difference_type _size;
		void pushChunk(const Packet &packet);

	};

	typedef boost::shared_ptr<Message> MessagePtr;
}}

#endif
