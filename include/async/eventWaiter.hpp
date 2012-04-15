#ifndef _ASYNC_EVENTWAITER_HPP_
#define _ASYNC_EVENTWAITER_HPP_

#include "async/event.hpp"

//////////////////////////////////////////////////////////////////////////
namespace async
{
	class Operand;
	typedef boost::shared_ptr<Operand> OperandPtr;
	
	template <class CustomEvent=Event>
	class EventWaiter
	{
	public:

		EventWaiter();
		EventWaiter(const CustomEvent &);
		EventWaiter(const CustomEvent &, const CustomEvent &);
		EventWaiter(const CustomEvent &, const CustomEvent &, const CustomEvent &);
		EventWaiter(const CustomEvent &, const CustomEvent &, const CustomEvent &, const CustomEvent &);
		EventWaiter(const CustomEvent &, const CustomEvent &, const CustomEvent &, const CustomEvent &, const CustomEvent &);
		EventWaiter(const CustomEvent &, const CustomEvent &, const CustomEvent &, const CustomEvent &, const CustomEvent &, const CustomEvent &);
		EventWaiter(const CustomEvent &, const CustomEvent &, const CustomEvent &, const CustomEvent &, const CustomEvent &, const CustomEvent &, const CustomEvent &);
		EventWaiter(const CustomEvent &, const CustomEvent &, const CustomEvent &, const CustomEvent &, const CustomEvent &, const CustomEvent &, const CustomEvent &, const CustomEvent &);
		EventWaiter(const CustomEvent &, const CustomEvent &, const CustomEvent &, const CustomEvent &, const CustomEvent &, const CustomEvent &, const CustomEvent &, const CustomEvent &, const CustomEvent &);
		EventWaiter(const CustomEvent &, const CustomEvent &, const CustomEvent &, const CustomEvent &, const CustomEvent &, const CustomEvent &, const CustomEvent &, const CustomEvent &, const CustomEvent &, const CustomEvent &);

		~EventWaiter();

		EventWaiter &operator <<(const CustomEvent &event);

		bool wait();
		operator bool();
		CustomEvent &current();
		operator CustomEvent &();
		size_t currentIndex();

	private:
		struct CustomEventHolder
		{
			CustomEvent	_event;
			size_t		_originalIndex;

			CustomEventHolder(const CustomEvent &event, size_t originalIndex)
				: _event(event)
				, _originalIndex(originalIndex)
			{}
		};
		std::deque<CustomEventHolder>	_customs;
		std::vector<Event>				_events;
		size_t							_current;
		static const size_t				_wrongIndex = (size_t)-1;
	};

	//////////////////////////////////////////////////////////////////////////
	template <class CustomEvent>
	EventWaiter<CustomEvent>::EventWaiter()
		: _current(_wrongIndex)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	template <class CustomEvent>
	EventWaiter<CustomEvent>::EventWaiter(const CustomEvent &event1)
		: _current(_wrongIndex)
	{
		(*this)<<event1;
	}

	//////////////////////////////////////////////////////////////////////////
	template <class CustomEvent>
	EventWaiter<CustomEvent>::EventWaiter(const CustomEvent &event1, const CustomEvent &event2)
		: _current(_wrongIndex)
	{
		(*this)<<event1<<event2;
	}

	//////////////////////////////////////////////////////////////////////////
	template <class CustomEvent>
	EventWaiter<CustomEvent>::EventWaiter(const CustomEvent &event1, const CustomEvent &event2, const CustomEvent &event3)
		: _current(_wrongIndex)
	{
		(*this)<<event1<<event2<<event3;
	}

	//////////////////////////////////////////////////////////////////////////
	template <class CustomEvent>
	EventWaiter<CustomEvent>::EventWaiter(const CustomEvent &event1, const CustomEvent &event2, const CustomEvent &event3, const CustomEvent &event4)
		: _current(_wrongIndex)
	{
		(*this)<<event1<<event2<<event3<<event4;
	}

	//////////////////////////////////////////////////////////////////////////
	template <class CustomEvent>
	EventWaiter<CustomEvent>::EventWaiter(const CustomEvent &event1, const CustomEvent &event2, const CustomEvent &event3, const CustomEvent &event4, const CustomEvent &event5)
		: _current(_wrongIndex)
	{
		(*this)<<event1<<event2<<event3<<event4<<event5;
	}

	//////////////////////////////////////////////////////////////////////////
	template <class CustomEvent>
	EventWaiter<CustomEvent>::EventWaiter(const CustomEvent &event1, const CustomEvent &event2, const CustomEvent &event3, const CustomEvent &event4, const CustomEvent &event5, const CustomEvent &event6)
		: _current(_wrongIndex)
	{
		(*this)<<event1<<event2<<event3<<event4<<event5<<event6;
	}

	//////////////////////////////////////////////////////////////////////////
	template <class CustomEvent>
	EventWaiter<CustomEvent>::EventWaiter(const CustomEvent &event1, const CustomEvent &event2, const CustomEvent &event3, const CustomEvent &event4, const CustomEvent &event5, const CustomEvent &event6, const CustomEvent &event7)
		: _current(_wrongIndex)
	{
		(*this)<<event1<<event2<<event3<<event4<<event5<<event6<<event7;
	}

	//////////////////////////////////////////////////////////////////////////
	template <class CustomEvent>
	EventWaiter<CustomEvent>::EventWaiter(const CustomEvent &event1, const CustomEvent &event2, const CustomEvent &event3, const CustomEvent &event4, const CustomEvent &event5, const CustomEvent &event6, const CustomEvent &event7, const CustomEvent &event8)
		: _current(_wrongIndex)
	{
		(*this)<<event1<<event2<<event3<<event4<<event5<<event6<<event7<<event8;
	}

	//////////////////////////////////////////////////////////////////////////
	template <class CustomEvent>
	EventWaiter<CustomEvent>::EventWaiter(const CustomEvent &event1, const CustomEvent &event2, const CustomEvent &event3, const CustomEvent &event4, const CustomEvent &event5, const CustomEvent &event6, const CustomEvent &event7, const CustomEvent &event8, const CustomEvent &event9)
		: _current(_wrongIndex)
	{
		(*this)<<event1<<event2<<event3<<event4<<event5<<event6<<event7<<event8<<event9;
	}

	//////////////////////////////////////////////////////////////////////////
	template <class CustomEvent>
	EventWaiter<CustomEvent>::EventWaiter(const CustomEvent &event1, const CustomEvent &event2, const CustomEvent &event3, const CustomEvent &event4, const CustomEvent &event5, const CustomEvent &event6, const CustomEvent &event7, const CustomEvent &event8, const CustomEvent &event9, const CustomEvent &event10)
		: _current(_wrongIndex)
	{
		(*this)<<event1<<event2<<event3<<event4<<event5<<event6<<event7<<event8<<event9<<event10;
	}

	//////////////////////////////////////////////////////////////////////////
	template <class CustomEvent>
	EventWaiter<CustomEvent>::~EventWaiter()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	template <class CustomEvent>
	EventWaiter<CustomEvent> &EventWaiter<CustomEvent>::operator <<(const CustomEvent &event)
	{
		_customs.push_back(CustomEventHolder(event, _customs.size()));
		_events.push_back((const Event&)event);
		return *this;
	}


	//////////////////////////////////////////////////////////////////////////
	template <class CustomEvent>
	bool EventWaiter<CustomEvent>::wait()
	{
		if(_wrongIndex == _current)
		{
			_current = 0;
		}
		else
		{
			_current++;
		}

		if(_current >= _events.size())
		{
			return false;
		}

		Event *begin = &_events[0]+_current;
		Event *ready =
			Event::waitAny(begin, &_events.back()+1);

		if(begin != ready)
		{
			size_t readyIdx = ready - begin + _current;
			std::swap(*begin, *ready);
			std::swap(_customs[_current], _customs[readyIdx]);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	template <class CustomEvent>
	EventWaiter<CustomEvent>::operator bool()
	{
		return wait();
	}


	//////////////////////////////////////////////////////////////////////////
	template <class CustomEvent>
	CustomEvent &EventWaiter<CustomEvent>::current()
	{
		if(_wrongIndex==_current)
		{
			assert("need call 'wait' first");
			throw exception("need call 'wait' first");
			static CustomEvent stub;
			return stub;
		}
		if(_current > _events.size() || _wrongIndex==_current)
		{
			assert("no more event in waiter");
			throw exception("no more event in waiter");
			static CustomEvent stub;
			return stub;
		}

		return _customs[_current]._event;
	}

	//////////////////////////////////////////////////////////////////////////
	template <class CustomEvent>
	EventWaiter<CustomEvent>::operator CustomEvent&()
	{
		return current();
	}

	//////////////////////////////////////////////////////////////////////////
	template <class CustomEvent>
	size_t EventWaiter<CustomEvent>::currentIndex()
	{
		if(_wrongIndex==_current)
		{
			assert("need call 'wait' first");
			throw exception("need call 'wait' first");
			return _wrongIndex;
		}
		if(_current > _events.size() || _wrongIndex==_current)
		{
			assert("no more event in waiter");
			throw exception("no more event in waiter");
			return _wrongIndex;
		}

		return _customs[_current]._originalIndex;
	}

}

#endif
