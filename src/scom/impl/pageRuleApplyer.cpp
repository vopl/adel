#include "pch.hpp"
#include "scom/impl/pageRuleApplyer.hpp"
#include "scom/log.hpp"
#include "htmlcxx/html/Uri.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

namespace scom { namespace impl
{

	/////////////////////////////////////////////////////////////////////////////////////
	PageRuleApplyer::PageRuleApplyer(boost::int64_t instanceId, const boost::posix_time::ptime &accessTime)
		: _instanceId(instanceId)
		, _accessTime(accessTime)
		, _maxLoadedPageId(0)
	{
	}

	/////////////////////////////////////////////////////////////////////////////////////
	PageRuleApplyer::~PageRuleApplyer()
	{

	}

	/////////////////////////////////////////////////////////////////////////////////////
	boost::int64_t PageRuleApplyer::instanceId() const
	{
		return _instanceId;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	boost::posix_time::ptime PageRuleApplyer::accessTime() const
	{
		return _accessTime;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	void PageRuleApplyer::accessTime(const boost::posix_time::ptime &accessTime)
	{
		_accessTime = accessTime;
	}


	/////////////////////////////////////////////////////////////////////////////////////
	void PageRuleApplyer::update()
	{
		assert(0);
	}

	//////////////////////////////////////////////////////////////////////////
	size_t PageRuleApplyer::loadRules(pgc::Result pgr)
	{
		_rulesRegex.clear();
		_rulesDomain.clear();
		_rulesPath.clear();
		_rulesReference.clear();

		size_t amount = 0;
		for(size_t i(0); i<pgr.rows(); i++)
		{
			utils::Variant row;
			bool b = pgr.fetchRowList(row, i);
			assert(b);

			//id, instance_id, value, kind_and_access, kind_and_access_min, kind_and_access_max, max_amount

			int kindAndAccess = row[3];

			switch(kindAndAccess & PageRule::ek_mask)
			{
			case PageRule::ek_domain:
				{
					htmlcxx::Uri uri("http://"+row[2].to<std::string>()+"/");
					if(uri.isOk())
					{
						_rulesDomain.push_back(RuleDomain());
						RuleDomain &r = _rulesDomain.back();
						r._access = kindAndAccess & PageRule::ea_mask;
						r._amount = row[6];
						r._levelMin = row[4];
						r._levelMax = row[5];

						boost::split(r._domain, uri.hostname(), boost::algorithm::is_any_of("."));
						std::reverse(r._domain.begin(), r._domain.end());
						amount++;
					}
					else
					{
						WLOG("domain page rule has bad value: "<<row[2].to<std::string>());
					}
				}
				break;
			case PageRule::ek_regex:
				{
					try
					{
						boost::regex regex(row[2].to<std::string>());

						_rulesRegex.push_back(RuleRegex());
						RuleRegex &r = _rulesRegex.back();
						r._access = kindAndAccess & PageRule::ea_mask;
						r._amount = row[6];
						r._regex = regex;
						amount++;
					}
					catch(...)
					{
						WLOG("regex page rule has bad value: "<<row[2].to<std::string>());
					}
				}
				break;
			case PageRule::ek_path:
				{
					htmlcxx::Uri uri(row[2].to<std::string>());
					if(uri.isOk())
					{
						_rulesPath.push_back(RulePath());
						RulePath &r = _rulesPath.back();
						r._access = kindAndAccess & PageRule::ea_mask;
						r._amount = row[6];
						r._host = uri.hostnameWithPort();
						r._levelMin = row[4];
						r._levelMax = row[5];

						boost::split(r._path, uri.path(), boost::algorithm::is_any_of("/"));

						//корень всегда пустой
						if(!r._path.empty())
						{
							if(r._path[0].empty())
							{
								r._path.erase(r._path.begin());
							}
						}

						//последний нод всегда файл, если путь оканчивается на / то он пустой. В любом случае его надо удалить
						if(!r._path.empty())
						{
							r._path.erase(r._path.begin()+r._path.size()-1);
						}
						amount++;
					}
					else
					{
						WLOG("path page rule has bad value: "<<row[2].to<std::string>());
					}
				}
				break;
			case PageRule::ek_reference:
				{
					htmlcxx::Uri uri(row[2].to<std::string>());
					if(uri.isOk())
					{
						_rulesReference.push_back(RuleReference());
						RuleReference &r = _rulesReference.back();
						r._access = kindAndAccess & PageRule::ea_mask;
						r._amount = row[6];
						r._source = uri.unparse(htmlcxx::Uri::REMOVE_FRAGMENT);
						r._levelMin = row[4];
						r._levelMax = row[5];
						amount++;
					}
					else
					{
						WLOG("path page rule has bad value: "<<row[2].to<std::string>());
					}
				}
				break;
			default:
				assert(!"unknown page rule kind");
				ELOG("unknown page rule kind: "<<(kindAndAccess & PageRule::ek_mask));
			}
		}
		return amount;
	}

	//////////////////////////////////////////////////////////////////////////
	boost::int64_t PageRuleApplyer::maxLoadedPageId()
	{
		return _maxLoadedPageId;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t PageRuleApplyer::loadPages(pgc::Result pgr)
	{
		size_t amount = 0;

		for(size_t i(0); i<pgr.rows(); i++)
		{
			utils::Variant row;
			bool b = pgr.fetchRowList(row, i);
			assert(b);

			//id, uri, is_allowed
			_maxLoadedPageId = std::max(_maxLoadedPageId, row[0].to<boost::int64_t>());

			_pages.push_back(Page());
			Page &p = _pages.back();
			p._uriStr = row[1].to<std::string>();
			p._uri = htmlcxx::Uri(p._uriStr);

			utils::Variant isAllowed = row[2];
			p._isAllowed = isAllowed.isNull()?false:isAllowed.to<bool>();

			amount++;
		}
		return amount;
	}


}}
