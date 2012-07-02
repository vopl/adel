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
#define CROSSIDX(page1Idx, page2Idx) ((page2Idx*(page2Idx-1))/2+page1Idx)
		std::vector<boost::int32_t> crossCounters(CROSSIDX(0, pagesAmount));

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
						//там треугольная матрица, ид должен быть упорядочены
						std::swap(page1Id, page2Id);
					}
					localCross[std::make_pair(page1Id, page2Id)]++;
				}
			}
			BOOST_FOREACH(const TLocalCross::value_type &c, localCross)
			{
				size_t cidx = CROSSIDX(c.first.first, c.first.second);
				assert(cidx < crossCounters.size());
				crossCounters[cidx] += 1;
			}
			//std::cout<<"-------- progress "<<end-beginRangeIter<<", "<<endRangeIter-beginRangeIter<<std::endl;
			beginRangeIter = endRangeIter;
		}

		phrases.clear();

		//вылить в базу накопленные веса
		{
			char sql[128];
			sprintf(sql, "UPDATE page_phrase_page SET intersect%d_volume=? WHERE page1_id=? AND page2_id=?", size, size);
			sqlitepp::statement stm(_db, sql);
			stm.prepare();
			//sqlitepp::transaction tr(_db);

			for(boost::int32_t page2Idx(0); page2Idx<_pageIds.size(); page2Idx++)
			{
				for(boost::int32_t page1Idx(0); page1Idx<page2Idx; page1Idx++)
				{
					size_t cidx = CROSSIDX(page1Idx, page2Idx);
					assert(cidx < crossCounters.size());
					stm.use_value(1, crossCounters[cidx]);
					stm.use_value(2, page1Idx);//page1_id < page2_id
					stm.use_value(3, page2Idx);
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
