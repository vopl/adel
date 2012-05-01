#ifndef _NET_HTTP_MESSAGE_HPP_
#define _NET_HTTP_MESSAGE_HPP_

#include "net/http/messageIterator.hpp"
#include <boost/range.hpp>
#include "net/packet.hpp"

namespace net { namespace http
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
		typedef MessageIterator Iterator;
		typedef boost::iterator_range<Iterator> Segment;

	public:
		Message();
		~Message();

		Iterator begin();
		Iterator end();
		Iterator endInfinity();
	};
}}

#endif
