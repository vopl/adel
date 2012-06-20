#include "pch.hpp"
#include "scom/impl/pageRuleApplyer.hpp"
#include "scom/log.hpp"
#include "htmlcxx/html/Uri.h"

namespace scom { namespace impl
{

	/////////////////////////////////////////////////////////////////////////////////////
	PageRuleApplyer::PageRuleApplyer(boost::int64_t instanceId, const boost::posix_time::ptime &accessTime)
		: _instanceId(instanceId)
		, _accessTime(accessTime)
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

						const std::string &hostname = uri.hostname();
						std::string::size_type prevPos = 0;
						std::string::size_type pos = hostname.find('/');
						while(std::string::npos != prevPos)
						{
							r._domain.push_back(
								hostname.substr(prevPos, std::string::npos==pos?std::string::npos:pos - prevPos)
							);
							prevPos = pos;
							pos = hostname.find('/', pos);
						}
					}
					else
					{
						WLOG("domain page rule has bad value: "<<row[2].to<std::string>());
					}
				}
				break;
			case PageRule::ek_regex:
				break;
			case PageRule::ek_path:
				break;
			case PageRule::ek_reference:
				break;
			default:
				assert(!"unknown page rule kind");
				ELOG("unknown page rule kind: "<<(kindAndAccess & PageRule::ek_mask));
			}
		}
		return amount;
	}

	

}}
