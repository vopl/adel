#ifndef _ASYNC_FUTUREWAITER_HPP_
#define _ASYNC_FUTUREWAITER_HPP_

#include "async/future.hpp"
#include "async/eventWaiter.hpp"

//////////////////////////////////////////////////////////////////////////
namespace async
{
	//////////////////////////////////////////////////////////////////////////
	template <class Data>
	class FutureWaiter
		: public EventWaiter<Future<Data> >
	{
		typedef EventWaiter<Future<Data> > Base;
	public:
		FutureWaiter();
		FutureWaiter(const Future<Data> &);
		FutureWaiter(const Future<Data> &, const Future<Data> &);
		FutureWaiter(const Future<Data> &, const Future<Data> &, const Future<Data> &);
		FutureWaiter(const Future<Data> &, const Future<Data> &, const Future<Data> &, const Future<Data> &);
		FutureWaiter(const Future<Data> &, const Future<Data> &, const Future<Data> &, const Future<Data> &, const Future<Data> &);
		FutureWaiter(const Future<Data> &, const Future<Data> &, const Future<Data> &, const Future<Data> &, const Future<Data> &, const Future<Data> &);
		FutureWaiter(const Future<Data> &, const Future<Data> &, const Future<Data> &, const Future<Data> &, const Future<Data> &, const Future<Data> &, const Future<Data> &);
		FutureWaiter(const Future<Data> &, const Future<Data> &, const Future<Data> &, const Future<Data> &, const Future<Data> &, const Future<Data> &, const Future<Data> &, const Future<Data> &);
		FutureWaiter(const Future<Data> &, const Future<Data> &, const Future<Data> &, const Future<Data> &, const Future<Data> &, const Future<Data> &, const Future<Data> &, const Future<Data> &, const Future<Data> &);
		FutureWaiter(const Future<Data> &, const Future<Data> &, const Future<Data> &, const Future<Data> &, const Future<Data> &, const Future<Data> &, const Future<Data> &, const Future<Data> &, const Future<Data> &, const Future<Data> &);

		operator Data &();

	};

	//////////////////////////////////////////////////////////////////////////
	template <class Data>
	FutureWaiter<Data>::FutureWaiter()
		: Base()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	template <class Data>
	FutureWaiter<Data>::FutureWaiter(const Future<Data> &event1)
		: Base(event1)
	{
	}
	//////////////////////////////////////////////////////////////////////////
	template <class Data>
	FutureWaiter<Data>::FutureWaiter(const Future<Data> &event1, const Future<Data> &event2)
		: Base(event1, event2)
	{
	}
	//////////////////////////////////////////////////////////////////////////
	template <class Data>
	FutureWaiter<Data>::FutureWaiter(const Future<Data> &event1, const Future<Data> &event2, const Future<Data> &event3)
		: Base(event1, event2, event3)
	{
	}
	//////////////////////////////////////////////////////////////////////////
	template <class Data>
	FutureWaiter<Data>::FutureWaiter(const Future<Data> &event1, const Future<Data> &event2, const Future<Data> &event3, const Future<Data> &event4)
		: Base(event1, event2, event3, event4)
	{
	}
	//////////////////////////////////////////////////////////////////////////
	template <class Data>
	FutureWaiter<Data>::FutureWaiter(const Future<Data> &event1, const Future<Data> &event2, const Future<Data> &event3, const Future<Data> &event4, const Future<Data> &event5)
		: Base(event1, event2, event3, event4, event5)
	{
	}
	//////////////////////////////////////////////////////////////////////////
	template <class Data>
	FutureWaiter<Data>::FutureWaiter(const Future<Data> &event1, const Future<Data> &event2, const Future<Data> &event3, const Future<Data> &event4, const Future<Data> &event5, const Future<Data> &event6)
		: Base(event1, event2, event3, event4, event5, event6)
	{
	}
	//////////////////////////////////////////////////////////////////////////
	template <class Data>
	FutureWaiter<Data>::FutureWaiter(const Future<Data> &event1, const Future<Data> &event2, const Future<Data> &event3, const Future<Data> &event4, const Future<Data> &event5, const Future<Data> &event6, const Future<Data> &event7)
		: Base(event1, event2, event3, event4, event5, event6, event7)
	{
	}
	//////////////////////////////////////////////////////////////////////////
	template <class Data>
	FutureWaiter<Data>::FutureWaiter(const Future<Data> &event1, const Future<Data> &event2, const Future<Data> &event3, const Future<Data> &event4, const Future<Data> &event5, const Future<Data> &event6, const Future<Data> &event7, const Future<Data> &event8)
		: Base(event1, event2, event3, event4, event5, event6, event7, event8)
	{
	}
	//////////////////////////////////////////////////////////////////////////
	template <class Data>
	FutureWaiter<Data>::FutureWaiter(const Future<Data> &event1, const Future<Data> &event2, const Future<Data> &event3, const Future<Data> &event4, const Future<Data> &event5, const Future<Data> &event6, const Future<Data> &event7, const Future<Data> &event8, const Future<Data> &event9)
		: Base(event1, event2, event3, event4, event5, event6, event7, event8, event9)
	{
	}
	//////////////////////////////////////////////////////////////////////////
	template <class Data>
	FutureWaiter<Data>::FutureWaiter(const Future<Data> &event1, const Future<Data> &event2, const Future<Data> &event3, const Future<Data> &event4, const Future<Data> &event5, const Future<Data> &event6, const Future<Data> &event7, const Future<Data> &event8, const Future<Data> &event9, const Future<Data> &event10)
		: Base(event1, event2, event3, event4, event5, event6, event7, event8, event9, event10)
	{
	}



	//////////////////////////////////////////////////////////////////////////
	template <class Data>
	FutureWaiter<Data>::operator Data &()
	{
		return Base::current().data();
	}

}

#endif
