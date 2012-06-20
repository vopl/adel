#ifndef _SCOM_IMPL_PAGERULEAPPLYER_HPP_
#define _SCOM_IMPL_PAGERULEAPPLYER_HPP_

#include "pgc/result.hpp"
#include "scom/service.hpp"

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>

#include <boost/regex.hpp>

#include <vector>
#include <string>

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
		
		size_t loadRules(pgc::Result pgr);

	private:
		boost::int64_t				_instanceId;
		boost::posix_time::ptime	_accessTime;
		
		struct Rule
		{
			int					_access;//PageRule::EAccess bits
			int					_amount;
		};
		
		struct RuleRegex
			: Rule
		{
			boost::regex	_regex;
		};
		
		struct RuleDomain
			: Rule
		{
			std::vector<std::string>	_domain;
			int				_levelMin;
			int				_levelMax;
		};
		
		struct RulePath
			: Rule
		{
			std::string					_host;
			std::vector<std::string>	_path;
			int							_levelMin;
			int							_levelMax;
		};


		struct RuleReference
			: Rule
		{
			std::string		_source;
			int				_levelMin;
			int				_levelMax;
		};

		std::vector<RuleRegex>		_rulesRegex;
		std::vector<RuleDomain>		_rulesDomain;
		std::vector<RulePath>		_rulesPath;
		std::vector<RuleReference>	_rulesReference;

	};
	typedef boost::shared_ptr<PageRuleApplyer> PageRuleApplyerPtr;
}}
#endif
