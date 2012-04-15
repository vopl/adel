#ifndef _ASYNC_EVENT_HPP_
#define _ASYNC_EVENT_HPP_

#include <boost/shared_ptr.hpp>
#include <deque>

//////////////////////////////////////////////////////////////////////////
namespace async
{
	namespace impl
	{
		class Event;
		typedef boost::shared_ptr<Event> EventPtr;
	}

	class Event
	{
		friend class impl::Event;
		impl::EventPtr	_impl;

	public:
		Event(bool autoReset = false);

		void set();
		void reset();
		bool isSet();
		void wait();

	public:

		static Event *waitAny(Event *begin, Event *end);
	};
}

#endif
