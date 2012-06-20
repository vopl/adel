#ifndef _SCOM_IMPL_PAGERULEAPPLYERSCONTAINER_HPP_
#define _SCOM_IMPL_PAGERULEAPPLYERSCONTAINER_HPP_

#include "scom/impl/pageRuleApplyer.hpp"
#include "pgc/connection.hpp"
#include "async/mutex.hpp"

#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/cstdint.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/mem_fun.hpp>

namespace scom { namespace impl
{

	using namespace boost;
	using namespace multi_index;

	class PageRuleApplyersContainer
	{
	public:
		PageRuleApplyersContainer();
		~PageRuleApplyersContainer();

		void start(
			const boost::posix_time::time_duration &cacheTimeout);
		void stop();

		bool update(pgc::Connection c, boost::int64_t instanceId);
		bool drop(boost::int64_t instanceId);
		bool dropOlds();

	private:
		async::Mutex						_mtx;
		boost::posix_time::time_duration	_cacheTimeout;

		//контейнер, индексирован по id экземпляра анализа, по времени последнего доступа
		typedef multi_index_container<
			PageRuleApplyerPtr,
			indexed_by<
				ordered_unique<
					const_mem_fun<
						PageRuleApplyer,
						boost::int64_t,
						&PageRuleApplyer::instanceId
					>
				>,
				ordered_non_unique<
					const_mem_fun<
						PageRuleApplyer,
						posix_time::ptime,
						&PageRuleApplyer::accessTime
					>
				>
			>
		> TInstances;

		//помогалка для модификации 'времени доступа' внутри мультииндекса
		struct ChangeAccessTime
		{
			ChangeAccessTime(const posix_time::ptime& accessTime):_accessTime(accessTime){}

			void operator()(PageRuleApplyerPtr &e)
			{
				e->accessTime(_accessTime);
			}

		private:
			posix_time::ptime _accessTime;
		};


		TInstances _instances;

	private:
		bool loadRules(pgc::Connection c, const PageRuleApplyerPtr &prap);
		bool loadPages(pgc::Connection c, const PageRuleApplyerPtr &prap);
		bool storePages(pgc::Connection c, const PageRuleApplyerPtr &prap);
	};
}}
#endif
