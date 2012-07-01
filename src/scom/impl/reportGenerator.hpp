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
		std::sort(phrases.begin(), phrases.end());
		typedef std::deque<PhraseEntry<size> > TPhrases;
		typename TPhrases::const_iterator beginRangeIter, endRangeIter, end, crossIter1, crossIter2;
		beginRangeIter = phrases.begin();
		end = phrases.end();

		char sql[128];
		sprintf(sql, "UPDATE page_phrase_page SET intersect%d_volume=intersect%d_volume+1 WHERE page1_id=? AND page2_id=?", size, size);
		sqlitepp::statement stm(_db, sql);
		stm.prepare();
		sqlitepp::transaction tr(_db);

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
					stm.use_value(1, page1Id);
					stm.use_value(2, page2Id);
					stm.exec();
				}
			}
			beginRangeIter = endRangeIter;
		}
		tr.commit();

		return true;
	}

}}
#endif
