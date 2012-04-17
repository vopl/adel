#ifndef _NET_MESSAGE_HPP_
#define _NET_MESSAGE_HPP_

# include <boost/iterator/iterator_facade.hpp>
#include "net/packet.hpp"

namespace net
{
	class Message
	{
	public:
		class Iterator
			: public boost::iterator_facade<Iterator, char, boost::random_access_traversal_tag>
		{
		public:
			Iterator(const Iterator &i);
			~Iterator();

			typedef size_t size_type;

		private:
			friend class boost::iterator_core_access;

			reference dereference() const;
			bool equal(const Iterator &i) const;
			void increment();
			void decrement();
			void advance(difference_type n);
			difference_type distance_to(const Iterator &i) const;
			
			difference_type absolutePosition() const;

		private:
			friend class Message;
			Iterator(Message *message, difference_type chunkIndex, difference_type offsetInChunk);
			Message *			_message;
			size_type		_chunkIndex;
			size_type		_offsetInChunk;
		};

	public:
		struct Sequence
		{
			Iterator _begin;
			Iterator _end;
		};

	public:
		Message();
		~Message();

		Iterator begin();
		Iterator end();
		Iterator endInfinity();

	private:
		friend class Iterator;
		struct SChunk
		{
			Iterator::difference_type	_offset;
			Packet						_packet;
		};
		typedef std::vector<SChunk> TVChunks;
		TVChunks _chunks;
		Iterator::difference_type _size;


		bool obtainMoreChunks();
	};
}

#endif
