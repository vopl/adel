#ifndef _SCOM_IMPL_PAGERULEAPPLYER_HPP_
#define _SCOM_IMPL_PAGERULEAPPLYER_HPP_

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>

namespace scom { namespace impl
{
	class PageRuleApplyer
	{
	public:
		PageRuleApplyer(boost::int64_t instanceId, const boost::posix_time::ptime &accessTime);
		~PageRuleApplyer();

		boost::int64_t				instanceId() const;
		boost::posix_time::ptime	accessTime() const;
		void accessTime(const boost::posix_time::ptime &accessTime);

		void update();

	private:
		boost::int64_t				_instanceId;
		boost::posix_time::ptime	_accessTime;

	};
	typedef boost::shared_ptr<PageRuleApplyer> PageRuleApplyerPtr;
}}
#endif
