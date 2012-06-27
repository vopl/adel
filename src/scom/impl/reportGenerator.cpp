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

		LITE(return, sqlite3_exec(_db, "CREATE TABLE page(id INT4 PRIMARY KEY,uri VARCHAR,ref_ids BLOB)", NULL, NULL, NULL));

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
		assert(0);
		//перебрать строки, обновить в базе урлы и ссылки
		return _isOk;
	}

}}
