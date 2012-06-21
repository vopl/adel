#ifndef _SCOM_IMPL_PAGERULEAPPLYER_HPP_
#define _SCOM_IMPL_PAGERULEAPPLYER_HPP_

#include "pgc/result.hpp"
#include "scom/service.hpp"
#include "htmlcxx/html/Uri.h"

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
		boost::int64_t maxLoadedPageId();
		size_t loadPages(pgc::Result pgrPages, pgc::Result pgrReferences);

		void storePages(std::vector<utils::Variant> &rows);


	private:
		boost::int64_t				_instanceId;
		boost::posix_time::ptime	_accessTime;
		
		struct Rule
		{
			int					_access;//PageRule::EAccess bits
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
			size_t			_sourcePageIdx;
		};

		std::vector<RuleRegex>		_rulesRegex;
		std::vector<RuleDomain>		_rulesDomain;
		std::vector<RulePath>		_rulesPath;
		std::vector<RuleReference>	_rulesReference;

		struct Page
		{
			boost::int64_t		_id;
			std::string			_uriStr;
			htmlcxx::Uri		_uri;
			//формируется при обновлении простых правил
			int					_accessSimple;//PageRule::EAccess bits
			//формируется при обновлении правил ссылочности
			int					_accessRefs;//PageRule::EAccess bits
			//исходный берется при добавлении страницы, итоговый формируется при сохранении таблицы
			int					_access;//PageRule::EAccess bits

			//индексы страниц, ссылки на которые есть в даной странице
			std::deque<size_t>	_refereces;

			//поколение обхода при просчете ссылочности, формируется заново и используется при каждом просчете
			size_t				_updateReferencesMarker;
		};
		std::deque<Page>	_pages;
		boost::int64_t		_maxLoadedPageId;
		typedef std::map<boost::int64_t, size_t> TPageId2Idx;
		TPageId2Idx _pageId2Idx;

	private:
			struct UpdateReferencesFrame
			{
				Page	*_page;
				int		_level;
				UpdateReferencesFrame(Page *page, int level)
					: _page(page)
					, _level(level)
				{
				}
			};

		void updateReferences(const RuleReference &r, size_t updateReferencesMarker);
	};
	typedef boost::shared_ptr<PageRuleApplyer> PageRuleApplyerPtr;
}}
#endif
