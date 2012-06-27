#include "pch.hpp"
#include "scom/impl/reportGenerator.hpp"
#include "scom/log.hpp"

#include <boost/foreach.hpp>

namespace scom { namespace impl
{

	///////////////////////////////////////////////////
	ReportGenerator::ReportGenerator(Hunspell *hunspell)
		: _isOk(true)
		, _hunspell(hunspell)
		, _db(NULL)
	{
		char fnameTemplate[L_tmpnam];
		if(!tmpnam_r(fnameTemplate))
		{
			ELOG("failed to create temporary file for report sqlite database: "<<strerror(errno));
			_isOk = false;
			return;
		}

#define LITE(action, ...) {int res = (__VA_ARGS__); if(SQLITE_OK != res && SQLITE_DONE != res && SQLITE_ROW != res){ELOG("sqlite call failed: "<<sqlite3_errmsg(_db)<<" ("<<__LINE__<<")");_isOk=false;action;}}

		LITE(return, sqlite3_open(fnameTemplate, &_db));

		LITE(return, sqlite3_exec(_db, "CREATE TABLE page(id INT4 PRIMARY KEY,uri VARCHAR)", NULL, NULL, NULL));

		LITE(return, sqlite3_exec(_db, "CREATE TABLE page_ref_page(src_page_id INT4, dst_page_id int4)", NULL, NULL, NULL));

		LITE(return, sqlite3_exec(_db, "CREATE TABLE phrase1(id INT4 PRIMARY KEY,src1 VARCHAR,page_ids BLOB)", NULL, NULL, NULL));
		LITE(return, sqlite3_exec(_db, "CREATE TABLE phrase2(id INT4 PRIMARY KEY,src1 VARCHAR,src2 VARCHAR,page_ids BLOB)", NULL, NULL, NULL));
		LITE(return, sqlite3_exec(_db, "CREATE TABLE phrase3(id INT4 PRIMARY KEY,src1 VARCHAR,src2 VARCHAR,src3 VARCHAR,page_ids BLOB)", NULL, NULL, NULL));
	}

	///////////////////////////////////////////////////
	ReportGenerator::~ReportGenerator()
	{
		if(_db)
		{
			sqlite3_close(_db);
		}

	}

	///////////////////////////////////////////////////
	bool ReportGenerator::isOk() const
	{
		return _isOk;
	}

	///////////////////////////////////////////////////
	bool ReportGenerator::addPageIds(const utils::Variant &ids)
	{
		BOOST_FOREACH(const utils::Variant &v, ids.as<utils::Variant::DequeVariant>())
		{
			_pageIds.push_back(v.to<boost::int64_t>());
		}
		return _isOk;
	}

	///////////////////////////////////////////////////
	bool ReportGenerator::fixPageIds()
	{
		std::sort(_pageIds.begin(), _pageIds.end());
		//вылить в базу
		sqlite3_stmt *stm;
		LITE(return false, sqlite3_prepare(_db, "INSERT INTO page (id) VALUES(?)", -1, &stm, NULL));

		for(int i(0); i<_pageIds.size(); i++)
		{
			LITE(sqlite3_finalize(stm);return false, sqlite3_bind_int(stm, 1, i));
			LITE(sqlite3_finalize(stm);return false, sqlite3_step(stm));
			LITE(sqlite3_finalize(stm);return false, sqlite3_reset(stm));
		}
		sqlite3_finalize(stm);
		return _isOk;
	}

	///////////////////////////////////////////////////
	bool ReportGenerator::setPagesContent(const utils::Variant &rows)
	{
		//перебрать строки, обновить в базе урлы и ссылки
		sqlite3_stmt *stm;
		LITE(return false, sqlite3_prepare(_db, "UPDATE page SET uri=? WHERE id=?", -1, &stm, NULL));

		sqlite3_stmt *stm2;
		LITE(sqlite3_finalize(stm);return false, sqlite3_prepare(_db, "INSERT INTO page_ref_page (src_page_id,dst_page_id) VALUES(?,?)", -1, &stm2, NULL));

		BOOST_FOREACH(const utils::Variant &row, rows.as<utils::Variant::DequeVariant>())
		{
			//id, uri, ref_page_ids, text
			const utils::Variant::DequeVariant &rowv = row.as<utils::Variant::DequeVariant>();
			int srcId = pageId(rowv[0]);
			const std::string &uri = rowv[1].as<std::string>();

			LITE(sqlite3_finalize(stm);sqlite3_finalize(stm2);return false, sqlite3_bind_text(stm, 1, uri.data(), uri.size(), NULL));
			LITE(sqlite3_finalize(stm);sqlite3_finalize(stm2);return false, sqlite3_bind_int(stm, 2, srcId));
			LITE(sqlite3_finalize(stm);sqlite3_finalize(stm2);return false, sqlite3_step(stm));
			LITE(sqlite3_finalize(stm);sqlite3_finalize(stm2);return false, sqlite3_reset(stm));

			if(!rowv[2].isNull())
			{
				const utils::Variant::VectorChar &refIds = rowv[2].as<utils::Variant::VectorChar>();

				for(size_t i(0); i<refIds.size(); i+=8)
				{
					boost::int64_t &i64 = *(boost::int64_t*)&refIds[i];
					int dstId = pageId(i64);

					LITE(sqlite3_finalize(stm);sqlite3_finalize(stm2);return false, sqlite3_bind_int(stm2, 1, srcId));
					LITE(sqlite3_finalize(stm);sqlite3_finalize(stm2);return false, sqlite3_bind_int(stm2, 2, dstId));
					LITE(sqlite3_finalize(stm);sqlite3_finalize(stm2);return false, sqlite3_step(stm2));
					LITE(sqlite3_finalize(stm);sqlite3_finalize(stm2);return false, sqlite3_reset(stm2));
				}
			}
		}

		sqlite3_finalize(stm);
		sqlite3_finalize(stm2);

		//индексировать слова
		return _isOk;
	}

	/////////////////////////////////////////////////////////////////////////////////
	int ReportGenerator::pageId(boost::int64_t id)
	{
		return (int)(std::lower_bound(_pageIds.begin(), _pageIds.end(), id) - _pageIds.begin());
	}

}}
