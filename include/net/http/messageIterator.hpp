#ifndef _NET_HTTP_MESSAGEITERATOR_HPP_
#define _NET_HTTP_MESSAGEITERATOR_HPP_

# include <boost/iterator/iterator_facade.hpp>

namespace net { namespace http
{
	namespace impl
	{
		class Message;
		class MessageBuffer;
		typedef boost::shared_ptr<MessageBuffer> MessageBufferPtr;
	}

	///////////////////////////////////////////////////////////////
	class MessageIterator
		: public boost::iterator_facade<MessageIterator, char, boost::random_access_traversal_tag>
		//: public boost::iterator_facade<MessageIterator, char, boost::bidirectional_traversal_tag>
	{
	public:
		MessageIterator();
		MessageIterator(const MessageIterator &i);
		~MessageIterator();

		typedef size_t size_type;

	public:
		friend class boost::iterator_core_access;

		reference dereference() const;
		bool equal(const MessageIterator &i) const;
		void increment();
		void decrement();
		void advance(difference_type n);
		difference_type distance_to(const MessageIterator &i) const;
		impl::MessageBufferPtr	buffer();

		char *rawBufferFwd(size_type &size);
		char *rawBufferBwd(size_type &size);

	public:
		size_type absolutePosition() const;
		bool isEndInfinity() const;

	private:
		friend class impl::Message;
		MessageIterator(impl::MessageBufferPtr buffer, char *position);

	private:
		impl::MessageBufferPtr	_buffer;
		char					*_position;
	};
}}

#endif
