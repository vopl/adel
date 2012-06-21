#include "pch.hpp"
#include "async/freeFunctions.hpp"
#include "scom/impl/pageRuleApplyersContainer.hpp"
#include "scom/log.hpp"

#include <boost/date_time/posix_time/posix_time_types.hpp>

#define IF_PGRES_ERROR(action, ...) {pgc::Result r = __VA_ARGS__; if(pgc::ersError == r.status()) {ELOG(r.errorMsg()<<" ("<<__LINE__<<")");action;}}

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
		const boost::posix_time::time_duration &cacheTimeout)
	{
		async::Mutex::ScopedLock sl(_mtx);

		_cacheTimeout = cacheTimeout;
		_instances.clear();
	}

	////////////////////////////////////////////////////////////////////
	void PageRuleApplyersContainer::stop()
	{
		if(async::workerExists())
		{
			async::Mutex::ScopedLock sl(_mtx);

			_cacheTimeout = boost::posix_time::minutes(10);
			_instances.clear();
		}
		else
		{
			assert(_cacheTimeout == boost::posix_time::minutes(10));
			assert(_instances.empty());
		}
	}

	////////////////////////////////////////////////////////////////////
	bool PageRuleApplyersContainer::update(pgc::Connection c, boost::int64_t instanceId)
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

			if(!loadRules(c, prap))
			{
				return false;
			}
		}
		if(!loadPages(c, prap))
		{
			return false;
		}

		prap->update();

		if(!storePages(c, prap))
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
	bool PageRuleApplyersContainer::loadRules(pgc::Connection c, const PageRuleApplyerPtr &prap)
	{
		pgc::Result res = c.query(
			"SELECT "
			"id, instance_id, value, kind_and_access, kind_and_access_min, kind_and_access_max, max_amount "
			"FROM page_rule WHERE instance_id=$1", utils::Variant(prap->instanceId()));

		IF_PGRES_ERROR(
			return false,
			res);

		prap->loadRules(res);

		return true;
	}

	////////////////////////////////////////////////////////////////////
	bool PageRuleApplyersContainer::loadPages(pgc::Connection c, const PageRuleApplyerPtr &prap)
	{
		pgc::Result res = c.query(
			"SELECT "
			"id, uri, is_allowed "
			"FROM page WHERE instance_id=$1 AND id>$2", utils::MVA(prap->instanceId(), prap->maxLoadedPageId()));

		IF_PGRES_ERROR(
			return false,
			res);

		prap->loadPages(res);

		return true;
	}

	////////////////////////////////////////////////////////////////////
	bool PageRuleApplyersContainer::storePages(pgc::Connection c, const PageRuleApplyerPtr &prap)
	{
		assert(0);
		return false;
	}

}}
