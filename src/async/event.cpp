#include "pch.hpp"
#include "async/event.hpp"
#include "async/impl/event.hpp"

namespace async
{
	//////////////////////////////////////////////////////////////////////////
	Event::Event(bool autoReset)
		: _impl(new impl::Event(autoReset))
	{
	}


	//////////////////////////////////////////////////////////////////////////
	void Event::set()
	{
		return _impl->set();
	}

	//////////////////////////////////////////////////////////////////////////
	void Event::reset()
	{
		return _impl->reset();
	}

	//////////////////////////////////////////////////////////////////////////
	bool Event::isSet()
	{
		return _impl->isSet();
	}

	//////////////////////////////////////////////////////////////////////////
	void Event::wait()
	{
		return _impl->wait();
	}

	//////////////////////////////////////////////////////////////////////////
	void Event::swap(Event &with)
	{
		_impl.swap(with._impl);
	}

	//////////////////////////////////////////////////////////////////////////
	Event *Event::waitAny(Event *begin, Event *end)
	{
		return impl::Event::waitAny(begin, end);
	}



}

