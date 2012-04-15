#ifndef _ASYNC_IMPL_EVENT_HPP_
#define _ASYNC_IMPL_EVENT_HPP_

#include "async/event.hpp"
#include "async/impl/fiber.hpp"

#include <vector>

namespace async { namespace impl
{
	class Event
	{
		boost::mutex				_mtx;
		volatile bool				_isSet;
		bool						_autoReset;
		std::vector<FiberPtr>		_waiters;

		struct MultiNotifier
		{
			boost::mutex			_mtx;
			FiberPtr				_initiator;
			void *					_readyKey;

			bool notifyReady(void *key);
		};
		typedef boost::shared_ptr<MultiNotifier> MultiNotifierPtr;

		struct MultiNotifierMarked
		{
			MultiNotifierPtr _pmn;
			void *_key;

			MultiNotifierMarked(const MultiNotifierPtr &pmn, void *key);
		};
		std::vector<MultiNotifierMarked>	_mnWaiters;
	public:
		Event(bool autoReset);
		~Event();

		void set();
		void reset();
		bool isSet();
		void wait();

	public:
		static async::Event *waitAny(async::Event *begin, async::Event *end);

	private:
		bool mnWait(const MultiNotifierMarked &mn);
		void mnCancel(const MultiNotifierPtr &pmn);

	};
	typedef boost::shared_ptr<Event> EventPtr;
}}
#endif
