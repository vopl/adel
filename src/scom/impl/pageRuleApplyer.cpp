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
	namespace
	{
		bool levelMatched(
			const std::vector<std::string> &pattern,
			int patternMin,
			int patternMax,
			const std::vector<std::string> &sample)
		{
			if(pattern.empty())
			{
				return true;
			}

			int mustMatchMin = pattern.size()-1 + patternMin;
			int mustMatchMax = pattern.size() + patternMax;

			if(mustMatchMin < 0)
			{
				mustMatchMin = 0;
			}

			if(mustMatchMax > pattern.size())
			{
				mustMatchMax = pattern.size();
			}

			if(mustMatchMax <= mustMatchMin)
			{
				return true;
			}

			if(mustMatchMax > sample.size())
			{
				mustMatchMax = sample.size();
			}

			for(int i(mustMatchMin); i<mustMatchMax; i++)
			{
				if(pattern[i] != sample[i])
				{
					return false;
				}
			}

			return true;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	void PageRuleApplyer::update()
	{
		//перебрать все страницы, применить к ним простые правила
		for(size_t pageIdx(0); pageIdx<_pages.size(); pageIdx++)
		{
			Page &p = _pages[pageIdx];
			p._access = 0;
			p._updateReferencesMarker = 0;

			//regex
			{
				for(size_t ruleIdx(0); ruleIdx<_rulesRegex.size(); ruleIdx++)
				{
					RuleRegex &r = _rulesRegex[ruleIdx];
					if(boost::regex_match(p._uriStr, r._regex))
					{
						p._access |= r._access;
						if(p._access & PageRule::ea_ignore)
						{
							break;
						}
					}
				}
			}
			if(p._access & PageRule::ea_ignore)
			{
				continue;
			}

			//domain
			{
				for(size_t ruleIdx(0); ruleIdx<_rulesDomain.size(); ruleIdx++)
				{
					RuleDomain &r = _rulesDomain[ruleIdx];

					std::vector<std::string> sample;
					boost::split(sample, p._uri.hostname(), boost::algorithm::is_any_of("."));
					std::reverse(sample.begin(), sample.end());

					if(levelMatched(r._domain, r._levelMin, r._levelMax, sample))
					{
						p._access |= r._access;
						if(p._access & PageRule::ea_ignore)
						{
							break;
						}
					}
				}
			}
			if(p._access & PageRule::ea_ignore)
			{
				continue;
			}

			//path
			{
				for(size_t ruleIdx(0); ruleIdx<_rulesPath.size(); ruleIdx++)
				{
					RulePath &r = _rulesPath[ruleIdx];

					if(p._uri.hostnameWithPort() != r._host)
					{
						continue;
					}

					std::vector<std::string> sample;
					boost::split(sample, p._uri.path(), boost::algorithm::is_any_of("/"));
					//последний нод всегда файл, если путь оканчивается на / то он пустой. В любом случае его надо удалить
					if(!sample.empty())
					{
						sample.pop_back();
					}

					if(levelMatched(r._path, r._levelMin, r._levelMax, sample))
					{
						p._access |= r._access;
						if(p._access & PageRule::ea_ignore)
						{
							break;
						}
					}
				}
			}
			/*if(p._access & PageRule::ea_ignore)
			{
				continue;
			}
			*/
		}

		//refs
		{
			for(size_t ruleIdx(0); ruleIdx<_rulesReference.size(); ruleIdx++)
			{
				RuleReference &r = _rulesReference[ruleIdx];

				assert((size_t)-1 != r._sourcePageIdx);

				if((size_t)-1 == r._sourcePageIdx)
				{
					continue;
				}

				updateReferences(r, ruleIdx+1);
			}
		}
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

			//id, instance_id, value, kind_and_access, kind_and_access_min, kind_and_access_max

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
						r._host = uri.hostnameWithPort();
						r._levelMin = row[4];
						r._levelMax = row[5];

						boost::split(r._path, uri.path(), boost::algorithm::is_any_of("/"));

						//последний нод всегда файл, если путь оканчивается на / то он пустой. В любом случае его надо удалить
						if(!r._path.empty())
						{
							r._path.pop_back();
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
						r._source = uri.unparse(htmlcxx::Uri::REMOVE_FRAGMENT);
						r._levelMin = row[4];
						r._levelMax = row[5];
						r._sourcePageIdx = (size_t)-1;
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
	size_t PageRuleApplyer::loadPages(pgc::Result pgrPages, pgc::Result pgrReferences)
	{
		size_t amount = 0;

		for(size_t i(0); i<pgrPages.rows(); i++)
		{
			utils::Variant row;
			bool b = pgrPages.fetchRowList(row, i);
			assert(b);

			//id, uri, access
			_maxLoadedPageId = std::max(_maxLoadedPageId, row[0].to<boost::int64_t>());

			_pages.push_back(Page());
			Page &p = _pages.back();
			p._uriStr = row[1].to<std::string>();
			p._uri = htmlcxx::Uri(p._uriStr);
			p._access = row[2];

			//проиндексировать id
			assert(_pageId2Idx.end() == _pageId2Idx.find(row[0]));//такая страница раньше не встречалась
			_pageId2Idx[row[0]] = _pages.size()-1;

			//добавить в ссылочный руль как источник
			for(size_t ruleIdx(0); ruleIdx<_rulesReference.size(); ruleIdx++)
			{
				RuleReference &r = _rulesReference[ruleIdx];
				if((size_t)-1 == r._sourcePageIdx && p._uriStr == r._source)
				{
					r._sourcePageIdx = _pages.size()-1;
				}
			}

			amount++;
		}

		for(size_t i(0); i<pgrReferences.rows(); i++)
		{
			utils::Variant row;
			bool b = pgrReferences.fetchRowList(row, i);
			assert(b);

			//src_page_id, dst_page_id
			boost::int64_t srcId = row[0];
			boost::int64_t dstId = row[1];

			assert(_pageId2Idx.end() != _pageId2Idx.find(srcId));
			assert(_pageId2Idx.end() != _pageId2Idx.find(dstId));

			assert(_pageId2Idx[srcId] < _pages.size());
			assert(_pageId2Idx[dstId] < _pages.size());

			_pages[_pageId2Idx[srcId]]._refereces.push_back(_pageId2Idx[dstId]);
		}
		return amount;
	}


	//////////////////////////////////////////////////////////////////////////
	void PageRuleApplyer::updateReferences(const RuleReference &r, size_t updateReferencesMarker)
	{
		std::deque<UpdateReferencesFrame> buffer;
		buffer.push_back(UpdateReferencesFrame(&_pages[r._sourcePageIdx], 0));

		while(!buffer.empty())
		{
			std::deque<UpdateReferencesFrame> nextBuffer;
			for(size_t i(0); i<buffer.size(); i++)
			{
				UpdateReferencesFrame &f = buffer[i];
				if(f._page->_updateReferencesMarker == updateReferencesMarker)
				{
					//уже обработан со всеми дочерними
					continue;
				}

				//дочерние
				for(size_t i(0); i<f._page->_refereces.size(); i++)
				{
					assert(_pages.size() > f._page->_refereces[i]);
					Page &child = _pages[f._page->_refereces[i]];
					if(child._refereces.empty())
					{
						//сам
						child._updateReferencesMarker = updateReferencesMarker;
						if(f._level+1 >= r._levelMin && f._level+1 <= r._levelMax)
						{
							child._access |= r._access;
						}
					}
					else
					{
						//в буфер, на следующий шаг
						nextBuffer.push_back(UpdateReferencesFrame(&child, f._level+1));
					}

				}

				//сам
				f._page->_updateReferencesMarker = updateReferencesMarker;
				if(f._level >= r._levelMin && f._level <= r._levelMax)
				{
					f._page->_access |= r._access;
				}
			}
			nextBuffer.swap(buffer);
		}
	}

}}
