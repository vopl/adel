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
		};
		std::deque<PhraseEntry<1> > _phrases1;
		std::deque<PhraseEntry<2> > _phrases2;
		std::deque<PhraseEntry<3> > _phrases3;

	private:
		boost::int32_t pageId(boost::int64_t id);
		boost::int32_t pushPageText(boost::int32_t id, const std::string &text);
		void stem(std::deque<int32_t> &compressedWords, const std::vector<int32_t> &wordChars);

	};
}}
#endif
