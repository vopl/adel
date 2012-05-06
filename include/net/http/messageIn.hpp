#ifndef _NET_HTTP_MESSAGEIN_HPP_
#define _NET_HTTP_MESSAGEIN_HPP_

#include "net/http/headerName.hpp"

namespace net { namespace http
{
	namespace impl
	{
		class MessageIn;
		typedef boost::shared_ptr<MessageIn> MessageInPtr;

		class MessageInBuffer;
		typedef boost::shared_ptr<MessageInBuffer> MessageInBufferPtr;
}

	///////////////////////////////////////////////////////////////
	class MessageIn
	{
	protected:
		typedef impl::MessageInPtr ImplPtr;
		ImplPtr	_impl;

	public:
		class Iterator
			: public boost::iterator_facade<Iterator, char, boost::random_access_traversal_tag>
		{
		public:
			typedef size_t size_type;

		public:
			Iterator();
			Iterator(const Iterator &i);
			~Iterator();

			size_type absolutePosition() const;

		private:
			friend class boost::iterator_core_access;

			reference dereference() const;
			bool equal(const Iterator &i) const;
			void increment();
			void decrement();
			void advance(difference_type n);
			difference_type distance_to(const Iterator &i) const;

		private:
			friend class impl::MessageIn;
			Iterator(const impl::MessageInBufferPtr buffer, char *position);

		private:
			impl::MessageInBufferPtr _buffer;
			char					*_position;
		};
		typedef boost::iterator_range<Iterator> Segment;

	protected:
		MessageIn();

	public:
		~MessageIn();

		bool isConnected() const;

		bool readRequestLine();
		bool readHeaders();
		bool readBody();


		//requestLine, responseLine
		const Segment *firstLine() const;

		//headers
		const Segment *header(const HeaderName &name) const;
		const Segment *header(size_t key) const;
		const Segment *header(const std::string &name) const;
		const Segment *header(const char *name, size_t nameSize) const;
		const Segment *header(const char *namez) const;

		const Segment *body() const;

		bool flush();
	};
}}

#endif
