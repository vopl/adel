#ifndef _SCOM_IMPL_REPORTGENERATOR_HPP_
#define _SCOM_IMPL_REPORTGENERATOR_HPP_

#include "utils/variant.hpp"
#include "sqlitepp/sqlitepp.hpp"

#ifdef near
#	undef near
#endif
#include <hunspell.hxx>

#include <boost/pool/pool_alloc.hpp>
#include <boost/unordered_map.hpp>

namespace scom { namespace impl
{
	class ReportGenerator
	{
	public:
		ReportGenerator(const std::string &tmpDir, Hunspell *hunspell);
		~ReportGenerator();

		bool isOk() const;
		bool addPageIds(const utils::Variant &ids);
		bool fixPageIds();

		bool setPagesContent(const utils::Variant &rows);

		struct CrossCounter
		{
			boost::int32_t _all;
			boost::int32_t _gt1c;
			boost::int32_t _gt2c;
			boost::int32_t _gt1m1;
			boost::int32_t _gt2m2;

			CrossCounter()
				: _all(0)
				, _gt1c(0)
				, _gt2c(0)
				, _gt1m1(0)
				, _gt2m2(0)
			{}
		};

		std::vector<CrossCounter> _crossCounters1;
		std::vector<CrossCounter> _crossCounters2;
		std::vector<CrossCounter> _crossCounters3;

		////////////////////////
		//половина матрицы пересечения страниц
		boost::int32_t crossIdx(boost::int32_t page1Id, boost::int32_t page2Id);

		bool evalPhraseWeights();

	private:
		bool							_isOk;
		Hunspell						*_hunspell;
		std::string						_dbFileName;
		sqlitepp::session				_db;
		std::deque<boost::int64_t>		_pageIds;

		template <size_t size>
		struct PhraseEntry
		{
			boost::int32_t	_pageId;
			boost::int32_t	_words[size];
			boost::int32_t	_amount;

			bool operator<(const PhraseEntry<size> &with) const;
			bool equalWords(const PhraseEntry<size> &with) const;
		};

		std::deque<PhraseEntry<1> > _phrases1;
		std::deque<PhraseEntry<2> > _phrases2;
		std::deque<PhraseEntry<3> > _phrases3;

	private:
		boost::int32_t pageId(boost::int64_t id);
		boost::int32_t pushPageText(boost::int32_t pageId, const std::string &text);
		void stem(std::deque<boost::int32_t> &compressedWords, const std::vector<boost::int32_t> &wordChars);
		void fillPhrases(boost::int32_t pageId, const std::deque<boost::int32_t> &compressedWords);

		template <size_t size>
		void fillPhrases(std::deque<PhraseEntry<size> > &phrases, boost::int32_t pageId, const std::deque<boost::int32_t> &compressedWords);

		template <size_t size>
		void compactPhrases(std::deque<PhraseEntry<size> > &dst, std::deque<PhraseEntry<size> > &phrases);

		template <size_t size>
		bool evalPhraseWeights(std::vector<CrossCounter> &crossCounters, std::deque<PhraseEntry<size> > &phrases);
	};





	///////////////////////////////////////////////////////////////
	inline boost::int32_t ReportGenerator::crossIdx(boost::int32_t page1Id, boost::int32_t page2Id)
	{
		assert(page1Id > 0);
		assert(page2Id <= _pageIds.size()+1);
		assert(page1Id < page2Id);

		page1Id -= 1;
		page2Id -= 1;

		return (page2Id*(page2Id-1))/2 + page1Id;
	}

	///////////////////////////////////////////////////////////////
	template <size_t size>
	bool ReportGenerator::PhraseEntry<size>::operator<(const PhraseEntry<size> &with) const
	{
		for(size_t i(0); i<size; i++)
		{
			if(_words[i] < with._words[i])
			{
				return true;
			}
			else if(_words[i] > with._words[i])
			{
				return false;
			}
		}
		return _pageId < with._pageId;
	}
	///////////////////////////////////////////////////////////////
	template <size_t size>
	bool ReportGenerator::PhraseEntry<size>::equalWords(const PhraseEntry<size> &with) const
	{
		for(size_t i(0); i<size; i++)
		{
			if(_words[i] != with._words[i])
			{
				return false;
			}
		}
		return true;
	}


	///////////////////////////////////////////////////////////////////
	template <size_t size>
	void ReportGenerator::compactPhrases(std::deque<PhraseEntry<size> > &dst, std::deque<PhraseEntry<size> > &phrases)
	{
		////////////////////////
		std::sort(phrases.begin(), phrases.end());
		typedef std::deque<PhraseEntry<size> > TPhrases;
		typename TPhrases::const_iterator beginRangeIter, endRangeIter, end;
		beginRangeIter = phrases.begin();
		end = phrases.end();

		//перебирать диапазоны идентичных фраз
		while(end != beginRangeIter)
		{
			endRangeIter = beginRangeIter;
			boost::int32_t amount = 0;
			do
			{
				amount += endRangeIter->_amount;
				endRangeIter++;
			}
			while(endRangeIter != end && endRangeIter->equalWords(*beginRangeIter) && endRangeIter->_pageId==beginRangeIter->_pageId);

			PhraseEntry<size> p = *beginRangeIter;
			p._amount = amount;
			dst.push_back(p);

			beginRangeIter = endRangeIter;
		}
	}

	///////////////////////////////////////////////////////////////////
	namespace {
		struct hashForCross
		{
			size_t operator()(const std::pair<boost::int32_t, boost::int32_t> &key) const
			{
				//половину на один половину на другой
				return (((size_t)key.first) << (sizeof(size_t)*8/2)) | (size_t)key.second;
			}
		};
	}

	///////////////////////////////////////////////////////////////////
	template <size_t size>
	bool ReportGenerator::evalPhraseWeights(std::vector<CrossCounter> &crossCounters, std::deque<PhraseEntry<size> > &phrases)
	{

		crossCounters.resize(crossIdx(1, _pageIds.size()+1));
		assert(!crossCounters.empty());
		memset(&crossCounters[0], 0, crossCounters.size()*sizeof(CrossCounter));

		////////////////////////
		std::sort(phrases.begin(), phrases.end());
		typedef std::deque<PhraseEntry<size> > TPhrases;
		typename TPhrases::const_iterator beginRangeIter, endRangeIter, end, crossIter1, crossIter2;
		beginRangeIter = phrases.begin();
		end = phrases.end();

		//перебирать диапазоны идентичных фраз
		while(end != beginRangeIter)
		{
			endRangeIter = beginRangeIter;

			do
			{
				endRangeIter++;
			}
			while(endRangeIter != end && endRangeIter->equalWords(*beginRangeIter));

			//внутри одного диапазона наращивать кросс весов
			size_t rangeSize = endRangeIter-beginRangeIter;
			if(rangeSize > 1)
			{
				typename TPhrases::const_iterator preEndRangeIter = endRangeIter-1;
				
				/*
				//аллокатор на базе буст пул
				typedef std::map<
					std::pair<boost::int32_t, boost::int32_t>, 
					boost::int32_t,
					std::less<std::pair<boost::int32_t, boost::int32_t> >,
					boost::fast_pool_allocator<std::pair<boost::int32_t, boost::int32_t> >
				> TLocalCross;
				*/

				/*typedef std::map<
					std::pair<boost::int32_t, boost::int32_t>,
					boost::int32_t
				> TLocalCross;
				*/
				/*typedef __gnu_cxx::hash_map<
					std::pair<boost::int32_t, boost::int32_t>,
					boost::int32_t,
					hashForCross
				> TLocalCross;
				*/

				typedef boost::unordered_map<
					std::pair<boost::int32_t, boost::int32_t>, 
					boost::int32_t,
					hashForCross
				> TLocalCross;


				TLocalCross localCross;

				for(crossIter1 = beginRangeIter; crossIter1 != preEndRangeIter; crossIter1++)
				{
					for(crossIter2 = crossIter1+1; crossIter2 != endRangeIter; crossIter2++)
					{
						std::pair<boost::int32_t,boost::int32_t> key(crossIter1->_pageId, crossIter2->_pageId);
						if(key.first == key.second)
						{
							//сам с собой не надо
							continue;
						}
						if(key.first > key.second)
						{
							//там треугольная матрица, ид должены быть упорядочены
							std::swap(key.first, key.second);
						}
						localCross[key] += crossIter1->_amount + crossIter2->_amount;
					}
				}

				BOOST_FOREACH(const TLocalCross::value_type &c, localCross)
				{
					size_t cidx = crossIdx(c.first.first, c.first.second);
					assert(cidx < crossCounters.size());
					CrossCounter &cc = crossCounters[cidx];
					cc._all += c.second;

					if(c.second > 1)
					{
						cc._gt1c++;
						cc._gt1m1 += c.second-1;
					}
					if(c.second > 2)
					{
						cc._gt2c++;
						cc._gt2m2 += c.second-2;
					}
				}
			}

			if(rangeSize > 10)
			{
				std::cout<<"-------- progress "<<end-beginRangeIter<<", "<<rangeSize<<std::endl;
			}
			beginRangeIter = endRangeIter;
		}

		phrases.clear();

/*
		//вылить в базу накопленные веса
		{
			char sql[256];
			sprintf(sql, "UPDATE page_phrase_page SET "
				"intersect%d_all_volume=?, "
				"intersect%d_gt1c_volume=?, "
				"intersect%d_gt1m1_volume=?, "
				"intersect%d_gt2c_volume=?, "
				"intersect%d_gt2m2_volume=? "
				"WHERE page1_id=? AND page2_id=?", size, size, size, size, size);
			sqlitepp::statement stm(_db, sql);
			stm.prepare();

			for(boost::int32_t page2Id(1); page2Id<=_pageIds.size(); page2Id++)
			{
				for(boost::int32_t page1Id(1); page1Id<page2Id; page1Id++)
				{
					size_t cidx = CROSSIDX(page1Id, page2Id);
					assert(cidx < crossCounters.size());
					const CrossCounter &cc = crossCounters[cidx];
					stm.use_value(1, cc._all);
					stm.use_value(2, cc._gt1c);
					stm.use_value(3, cc._gt1m1);
					stm.use_value(4, cc._gt2c);
					stm.use_value(5, cc._gt2m2);
					stm.use_value(6, page1Id);//page1_id < page2_id
					stm.use_value(7, page2Id);
					stm.exec();
				}
			}
		}
*/

		std::cout<<"-------- size "<<size<<" complete"<<std::endl;
		/*if(3 == size)
		{
			exit(0);
		}*/

		return true;
	}

}}
#endif
