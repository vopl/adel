#ifndef _NET_HTTP_INPUTMESSAGE_HPP_
#define _NET_HTTP_INPUTMESSAGE_HPP_

#include "net/http/headerName.hpp"

namespace net { namespace http
{
	namespace impl
	{
		class InputMessage;
		typedef boost::shared_ptr<InputMessage> InputMessagePtr;

		class InputMessageBuffer;
	}

	///////////////////////////////////////////////////////////////
	class InputMessage
	{
	protected:
		typedef impl::InputMessagePtr ImplPtr;
		ImplPtr	_impl;

	public:
		class Iterator
			: public boost::iterator_facade<Iterator, const char, boost::random_access_traversal_tag>
		{
		public:
			typedef size_t size_type;

		public:
			Iterator(const Iterator &i);
			~Iterator();

			Iterator &operator=(const Iterator &i);
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
			friend class impl::InputMessage;
			Iterator();//endInfinity
			Iterator(impl::InputMessageBuffer *buffer, const char *position);//normal
		private:
			impl::InputMessageBuffer	*_buffer;
			const char				*_position;
		};
		typedef boost::iterator_range<Iterator> Segment;

	protected:
		InputMessage();

	public:
		~InputMessage();

		bool isConnected() const;

		bool readRequestLine();
		bool readHeaders();
		bool readBody();


		//requestLine, responseLine
		const Segment &firstLine() const;

		//headers
		const Segment &headers() const;
		const Segment *header(const HeaderName &name) const;
		const Segment *header(size_t key) const;
		const Segment *header(const std::string &name) const;
		const Segment *header(const char *name, size_t nameSize) const;
		const Segment *header(const char *namez) const;

		const Segment &body() const;
	};
}}

#endif
