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
		, _maxLoadedPageRefId(0)
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

			int mustMatchMin = ((int)pattern.size())-1 + patternMin;
			int mustMatchMax = ((int)pattern.size()) + patternMax;

			if(mustMatchMin < 0)
			{
				mustMatchMin = 0;
			}

			if(mustMatchMax > pattern.size())
			{
				mustMatchMax = (int)pattern.size();
			}

			if(mustMatchMax <= mustMatchMin)
			{
				return true;
			}

			if(mustMatchMax > sample.size())
			{
				mustMatchMax = (int)sample.size();
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
		//бросить счетчики применения
		BOOST_FOREACH(RuleRegex &r, _rulesRegex)
		{
			r._amountApplyed = 0;
		}
		BOOST_FOREACH(RuleDomain &r, _rulesDomain)
		{
			r._amountApplyed = 0;
		}
		BOOST_FOREACH(RulePath &r, _rulesPath)
		{
			r._amountApplyed = 0;
		}
		BOOST_FOREACH(RuleReference &r, _rulesReference)
		{
			r._amountApplyed = 0;
		}

		//перебрать все страницы, применить к ним простые правила
		for(size_t pageIdx(0); pageIdx<_pages.size(); pageIdx++)
		{
			Page &p = _pages[pageIdx];
			p._accessRefs = 0;
			p._accessSimple = 0;

			//regex
			{
				for(size_t ruleIdx(0); ruleIdx<_rulesRegex.size(); ruleIdx++)
				{
					RuleRegex &r = _rulesRegex[ruleIdx];
					if(r._amountApplyed >= r._amount)
					{
						continue;
					}
					if(boost::regex_match(p._uriStr, r._regex))
					{
						p._accessSimple |= r._access;
						r._amountApplyed++;
					}
				}
			}

			//domain
			{
				for(size_t ruleIdx(0); ruleIdx<_rulesDomain.size(); ruleIdx++)
				{
					RuleDomain &r = _rulesDomain[ruleIdx];
					if(r._amountApplyed >= r._amount)
					{
						continue;
					}

					std::vector<std::string> sample;
					boost::split(sample, p._uri.hostname(), boost::algorithm::is_any_of("."));
					std::reverse(sample.begin(), sample.end());

					if(levelMatched(r._domain, r._levelMin, r._levelMax, sample))
					{
						p._accessSimple |= r._access;
						r._amountApplyed++;
					}
				}
			}

			//path
			{
				for(size_t ruleIdx(0); ruleIdx<_rulesPath.size(); ruleIdx++)
				{
					RulePath &r = _rulesPath[ruleIdx];
					if(r._amountApplyed >= r._amount)
					{
						continue;
					}

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
						p._accessSimple |= r._access;
						r._amountApplyed++;
					}
				}
			}
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

				updateReferences(r);
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

			//id, instance_id, value, kind_and_access, kind_and_access_min, kind_and_access_max, amount

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
						r._amount = row[6];
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
	boost::int64_t PageRuleApplyer::maxLoadedPageRefId()
	{
		return _maxLoadedPageRefId;
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
			_pages.push_back(Page());
			Page &p = _pages.back();

			p._id = row[0];
			p._uriStr = row[1].to<std::string>();
			p._uri = htmlcxx::Uri(p._uriStr);

			utils::Variant accessv = row[2];
			p._access = accessv.isNull()?-1:accessv.to<int>();
			p._accessSimple = 0;
			p._accessRefs = 0;
			p._levelInCurrentUpdate = INT_MAX;

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
			_maxLoadedPageId = std::max(_maxLoadedPageId, p._id);

			amount++;
		}

		for(size_t i(0); i<pgrReferences.rows(); i++)
		{
			utils::Variant row;
			bool b = pgrReferences.fetchRowList(row, i);
			assert(b);

			const utils::Variant::DequeVariant &rowv = row.as<utils::Variant::DequeVariant>();
			//id, ref_page_ids, refId
			boost::int64_t srcId = rowv[0].as<boost::int64_t>();
			const utils::Variant::VectorChar &dstIds = rowv[1].as<utils::Variant::VectorChar>();
			boost::int64_t refId = rowv[2].as<boost::int64_t>();

			assert(_pageId2Idx.end() != _pageId2Idx.find(srcId));
			assert(_pageId2Idx[srcId] < _pages.size());

			for(size_t i(0); i<dstIds.size(); i+=8)
			{
				const boost::int64_t &dstId = *(const boost::int64_t *)&dstIds[i];
				assert(_pageId2Idx.end() != _pageId2Idx.find(dstId));
				assert(_pageId2Idx[dstId] < _pages.size());

				_pages[_pageId2Idx[srcId]]._refereces.push_back(_pageId2Idx[dstId]);
			}
			_maxLoadedPageRefId = std::max(_maxLoadedPageRefId, refId);

		}
		return amount;
	}

	//////////////////////////////////////////////////////////////////////////
	void PageRuleApplyer::storePages(std::vector<utils::Variant> &rows)
	{
		for(size_t i(0); i<_pages.size(); i++)
		{
			Page &p = _pages[i];
			int access = p._accessSimple | p._accessRefs;

			if(access != p._access)
			{
				p._access = access;
				rows.push_back(utils::MVA(p._id, p._access));
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void PageRuleApplyer::updateReferences(RuleReference &r)
	{
		assert(r._levelMin <= r._levelMax);
		if(0 > r._levelMax)
		{
			return;
		}
		std::deque<UpdateReferencesFrame> buffer;
		buffer.push_back(UpdateReferencesFrame(&_pages[r._sourcePageIdx], 0));

		while(!buffer.empty())
		{
			std::deque<UpdateReferencesFrame> nextBuffer;
			for(size_t i(0); i<buffer.size(); i++)
			{
				UpdateReferencesFrame &f = buffer[i];

				//сам
				if(f._page->_levelInCurrentUpdate > f._level)
				{
					//но уровень понизился, надо обработать
					f._page->_levelInCurrentUpdate = f._level;
				}
				else
				{
					//уже обработан со всеми дочерними и уровень не понизился, пропустить такой узел
					continue;
				}

				//дочерние в буфер, на следующий шаг
				//if(f._level < r._levelMax)
				{
					for(size_t i(0); i<f._page->_refereces.size(); i++)
					{
						assert(_pages.size() > f._page->_refereces[i]);
						Page &child = _pages[f._page->_refereces[i]];
						nextBuffer.push_back(UpdateReferencesFrame(&child, f._level+1));
					}
				}
			}
			nextBuffer.swap(buffer);
		}

		//плохо если страниц много а выбираемая область маленькая
		for(size_t i(0); i<_pages.size(); i++)
		{
			if(r._amountApplyed >= r._amount)
			{
				continue;
			}

			Page &p = _pages[i];
			if(
				p._levelInCurrentUpdate >= r._levelMin &&
				p._levelInCurrentUpdate <= r._levelMax &&
				true)
			{
				p._accessRefs |= r._access;
				r._amountApplyed++;
			}
			p._levelInCurrentUpdate = INT_MAX;
		}
	}

}}
