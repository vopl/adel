#ifndef _SCOM_IMPL_REPORTGENERATOR_HPP_
#define _SCOM_IMPL_REPORTGENERATOR_HPP_

#include "utils/variant.hpp"
#include "sqlitepp/sqlitepp.hpp"

#ifdef near
#	undef near
#endif
#include <hunspell.hxx>


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

			bool operator<(const PhraseEntry<size> &with) const;
			bool operator==(const PhraseEntry<size> &with) const;
			bool operator!=(const PhraseEntry<size> &with) const;
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
		bool evalPhraseWeights(std::deque<PhraseEntry<size> > &phrases);
	};






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
		return false;
	}
	///////////////////////////////////////////////////////////////
	template <size_t size>
	bool ReportGenerator::PhraseEntry<size>::operator==(const PhraseEntry<size> &with) const
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
	///////////////////////////////////////////////////////////////
	template <size_t size>
	bool ReportGenerator::PhraseEntry<size>::operator!=(const PhraseEntry<size> &with) const
	{
		return !operator==(with);
	}
	///////////////////////////////////////////////////////////////////
	template <size_t size>
	bool ReportGenerator::evalPhraseWeights(std::deque<PhraseEntry<size> > &phrases)
	{
		size_t pagesAmount = _pageIds.size();
		////////////////////////
		//половина матрицы пересечения страниц
#define CROSSIDX(page1Id, page2Id) (((page2Id-1)*(page2Id-2))/2+(page1Id-1))

		std::vector<CrossCounter> crossCounters(CROSSIDX(1, pagesAmount+1));

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
			while(*endRangeIter == *beginRangeIter && endRangeIter != end);

			//внутри одного диапазона наращивать кросс весов
			typedef std::map<std::pair<boost::int32_t, boost::int32_t>, boost::int32_t> TLocalCross;
			TLocalCross localCross;
			for(crossIter1 = beginRangeIter; crossIter1 != endRangeIter; crossIter1++)
			{
				for(crossIter2 = beginRangeIter; crossIter2 != endRangeIter; crossIter2++)
				{
					boost::int32_t page1Id = crossIter1->_pageId;
					boost::int32_t page2Id = crossIter2->_pageId;
					if(page1Id == page2Id)
					{
						//сам с собой не надо
						continue;
					}
					if(page1Id > page2Id)
					{
						//там треугольная матрица, ид должены быть упорядочены
						std::swap(page1Id, page2Id);
					}
					localCross[std::make_pair(page1Id, page2Id)]++;
				}
			}
			BOOST_FOREACH(const TLocalCross::value_type &c, localCross)
			{
				size_t cidx = CROSSIDX(c.first.first, c.first.second);
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
			//std::cout<<"-------- progress "<<end-beginRangeIter<<", "<<endRangeIter-beginRangeIter<<std::endl;
			beginRangeIter = endRangeIter;
		}

		phrases.clear();

		//вылить в базу накопленные веса
		{
			char sql[128];
			sprintf(sql, "UPDATE page_phrase_page SET "
				"intersect%d_all_volume=?, "
				"intersect%d_gt1c_volume=?, "
				"intersect%d_gt1m1_volume=?, "
				"intersect%d_gt2c_volume=?, "
				"intersect%d_gt2m2_volume=? "
				"WHERE page1_id=? AND page2_id=?", size, size, size, size, size);
			sqlitepp::statement stm(_db, sql);
			stm.prepare();
			//sqlitepp::transaction tr(_db);

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
			//tr.commit();
		}


		//std::cout<<"-------- size "<<size<<" complete"<<std::endl;
		/*if(3 == size)
		{
			exit(0);
		}*/

		return true;
	}

}}
#endif
