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

	private:
		int pageId(boost::int64_t id);

	};
}}
#endif
