#ifndef _NET_MESSAGE_HPP_
#define _NET_MESSAGE_HPP_

# include <boost/iterator/iterator_facade.hpp>
# include <boost/range.hpp>
#include "net/packet.hpp"

namespace net
{
	namespace impl
	{
		class Message;
		typedef boost::shared_ptr<Message> MessagePtr;
	}
	///////////////////////////////////////////////////////////////
	class Message
	{
	protected:
		typedef impl::MessagePtr ImplPtr;
		ImplPtr	_impl;

	public:
		class Iterator
			: public boost::iterator_facade<Iterator, char, boost::random_access_traversal_tag>
		{
		public:
			Iterator();
			Iterator(const Iterator &i);
			~Iterator();

			typedef size_t size_type;

			static const difference_type _badOffset = -1;


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
			friend class impl::Message;
			Iterator(impl::Message *message, difference_type chunkIndex, difference_type offsetInChunk);
			impl::Message		*_message;
			size_type			_chunkIndex;
			size_type			_offsetInChunk;
		};

	public:
		typedef boost::iterator_range<Iterator> Segment;

	public:
		Message();
		~Message();

		Iterator begin();
		Iterator end();
		Iterator endInfinity();
	};
}

#endif
