#ifndef _SCOM_IMPL_REPORTGENERATOR_HPP_
#define _SCOM_IMPL_REPORTGENERATOR_HPP_

#include "utils/variant.hpp"

#ifdef near
#	undef near
#endif
#include <hunspell.hxx>

#include <sqlite3.h>

namespace scom { namespace impl
{
	class ReportGenerator
	{
	public:
		ReportGenerator(Hunspell *hunspell);
		~ReportGenerator();

		bool isOk() const;
		bool addPageIds(const utils::Variant &ids);
		bool fixPageIds();

		bool setPagesContent(const utils::Variant &rows);

	private:
		bool							_isOk;
		Hunspell						*_hunspell;
		sqlite3							*_db;
		std::deque<boost::int64_t>		_pageIds;

	};
}}
#endif
