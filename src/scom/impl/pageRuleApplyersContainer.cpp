#include "pch.hpp"
#include "scom/impl/pageRuleApplyersContainer.hpp"

#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace scom { namespace impl
{

	////////////////////////////////////////////////////////////////////
	PageRuleApplyersContainer::PageRuleApplyersContainer()
		: _cacheTimeout(boost::posix_time::minutes(10))
	{
	}

	////////////////////////////////////////////////////////////////////
	PageRuleApplyersContainer::~PageRuleApplyersContainer()
	{
		stop();
	}

	////////////////////////////////////////////////////////////////////
	void PageRuleApplyersContainer::start(
		const pgc::Db &db,
		const boost::posix_time::time_duration &cacheTimeout)
	{
		async::Mutex::ScopedLock sl(_mtx);

		_db = db;
		_cacheTimeout = cacheTimeout;
		_instances.clear();
	}

	////////////////////////////////////////////////////////////////////
	void PageRuleApplyersContainer::stop()
	{
		async::Mutex::ScopedLock sl(_mtx);

		_db.reset();
		_cacheTimeout = boost::posix_time::minutes(10);
		_instances.clear();
	}

	////////////////////////////////////////////////////////////////////
	bool PageRuleApplyersContainer::update(boost::int64_t instanceId)
	{
		async::Mutex::ScopedLock sl(_mtx);

		PageRuleApplyerPtr prap;

		TInstances::nth_index<0>::type &idIndex = _instances.get<0>();
		TInstances::nth_index<0>::type::iterator iter = idIndex.find(instanceId);
		if(idIndex.end() != iter)
		{
			idIndex.modify(iter, ChangeAccessTime(posix_time::second_clock::local_time()));
			prap = *iter;
		}
		else
		{
			prap.reset(new PageRuleApplyer(instanceId, posix_time::second_clock::local_time()));
			idIndex.insert(prap);

			if(!loadRules(prap))
			{
				return false;
			}
		}
		if(!loadPages(prap))
		{
			return false;
		}

		prap->update();

		if(!storePages(prap))
		{
			return false;
		}
		return true;
	}

	////////////////////////////////////////////////////////////////////
	bool PageRuleApplyersContainer::drop(boost::int64_t instanceId)
	{
		async::Mutex::ScopedLock sl(_mtx);
		_instances.get<0>().erase(instanceId);
		return false;
	}

	////////////////////////////////////////////////////////////////////
	bool PageRuleApplyersContainer::dropOlds()
	{
		async::Mutex::ScopedLock sl(_mtx);

		//удаление по таймауту безактивности
		posix_time::ptime boundATime = posix_time::second_clock::local_time() - _cacheTimeout;
		TInstances::nth_index<1>::type &atimeIndex = _instances.get<1>();
		while(!atimeIndex.empty() && boundATime > (*atimeIndex.begin())->accessTime())
		{
			atimeIndex.erase(atimeIndex.begin());
		}
		return false;
	}

	////////////////////////////////////////////////////////////////////
	bool PageRuleApplyersContainer::loadRules(const PageRuleApplyerPtr &prap)
	{
		assert(0);
		return false;
	}

	////////////////////////////////////////////////////////////////////
	bool PageRuleApplyersContainer::loadPages(const PageRuleApplyerPtr &prap)
	{
		assert(0);
		return false;
	}

	////////////////////////////////////////////////////////////////////
	bool PageRuleApplyersContainer::storePages(const PageRuleApplyerPtr &prap)
	{
		assert(0);
		return false;
	}

}}
