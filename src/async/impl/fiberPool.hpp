#ifndef _ASYNC_IMPL_FIBERPOOL_HPP_
#define _ASYNC_IMPL_FIBERPOOL_HPP_

#include "async/impl/fiber.hpp"

#include <set>

namespace async { namespace impl
{
	class FiberPool
	{
	public:
		//всего существующих фиберов (простой, работа, ожидание)
		//непосредственно рабочие нигде не учитываются
		size_t						_totalAmount;
		//фиберы без задачи
		std::set<FiberPtr>			_fibersIdle;
		//рабочие фиберы с задачей и готовые к исполнению
		std::deque<FiberPtr>		_fibersReady;
		boost::mutex				_mtxFibers;


		//очередь задач на которые не хватило фиберов
		std::deque<boost::function<void()> >
										_tasks;
		boost::mutex					_mtxTasks;

	public:
		FiberPool()
			: _totalAmount(0)
		{
		}
	};
	typedef boost::shared_ptr<FiberPool> FiberPoolPtr;
}}
#endif
