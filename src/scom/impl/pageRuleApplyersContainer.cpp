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

		PageRuleApplyerPtr pra;

		TInstances::nth_index<1>::type &idIndex = _instances.get<1>();
		TInstances::nth_index<1>::type::iterator iter = idIndex.find(instanceId);
		if(idIndex.end() != iter)
		{
			idIndex.modify(iter, ChangeAccessTime(posix_time::microsec_clock::local_time()));
			pra = *iter;
		}
		else
		{
			pra.reset(new PageRuleApplyer(instanceId, posix_time::microsec_clock::local_time()));
			idIndex.insert(pra);
		}

	}

	////////////////////////////////////////////////////////////////////
	bool PageRuleApplyersContainer::drop(boost::int64_t instanceId)
	{
		async::Mutex::ScopedLock sl(_mtx);

		assert(0);
	}

	////////////////////////////////////////////////////////////////////
	bool PageRuleApplyersContainer::dropOlds()
	{
		async::Mutex::ScopedLock sl(_mtx);

		assert(0);
	}

}}
