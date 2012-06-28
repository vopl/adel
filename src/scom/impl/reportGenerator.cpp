#include "pch.hpp"
#include "scom/impl/reportGenerator.hpp"
#include "scom/log.hpp"

#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>

namespace scom { namespace impl
{

	///////////////////////////////////////////////////
	ReportGenerator::ReportGenerator(const std::string &tmpDir, Hunspell *hunspell)
		: _isOk(true)
		, _hunspell(hunspell)
	{
#ifdef _MSC_VER
		char fnameBuf[L_tmpnam];
		if(tmpnam_s(fnameBuf))
		{
			ELOG("failed to create temporary file for report sqlite database: "<<strerror(errno));
			_isOk = false;
			return;
		}
#else
		boost::filesystem::path p(tmpDir);
		p = boost::filesystem::absolute(p);
		boost::system::error_code ec;
		p = boost::filesystem::canonical(p, ec);
		if(ec)
		{
			ELOG("unable to canonicalize temporary directory path: "<<ec);
			_isOk = false;
			return;
		}
		p /= "rdb_XXXXXX.sqlite";
		std::string str = p.string();
		std::vector<char> fnameBuf(str.begin(), str.end());
		fnameBuf.push_back(0);
		int fd = mkstemps(&fnameBuf[0], 7);
		if(0 > fd)
		{
			ELOG("failed to create temporary file for report sqlite database: "<<strerror(errno));
			_isOk = false;
			return;
		}
		close(fd);
		_dbFileName.assign(fnameBuf.begin(), fnameBuf.end());
#endif

//#define LITE(action, ...) {int res = (__VA_ARGS__); if(SQLITE_OK != res && SQLITE_DONE != res && SQLITE_ROW != res){ELOG("sqlite call failed: "<<sqlite3_errmsg(_db)<<" ("<<__LINE__<<")");_isOk=false;action;}}

		try
		{
			_db.open(_dbFileName);
			_db<<"CREATE TABLE page(id INT4 PRIMARY KEY,uri VARCHAR,volume INT4)";

			_db<<"CREATE TABLE page_ref_page(src_page_id INT4, dst_page_id int4, PRIMARY KEY(src_page_id,dst_page_id) )";

			_db<<"CREATE TABLE phrase1(id INT4 PRIMARY KEY,src1 VARCHAR,page_ids BLOB)";
			_db<<"CREATE TABLE phrase2(id INT4 PRIMARY KEY,src1 VARCHAR,src2 VARCHAR,page_ids BLOB)";
			_db<<"CREATE TABLE phrase3(id INT4 PRIMARY KEY,src1 VARCHAR,src2 VARCHAR,src3 VARCHAR,page_ids BLOB)";

			_db<<"CREATE TABLE page_phrase_page(page1_id INT4, page2_id INT4,intersect1_volume INT4,intersect2_volume INT4,intersect3_volume INT4, PRIMARY KEY(page1_id,page2_id))";
		}
		catch(sqlitepp::exception &e)
		{
			ELOG("sqlitepp exception: "<<e.what());
			_isOk = false;
		}


	}

	///////////////////////////////////////////////////
	ReportGenerator::~ReportGenerator()
	{
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

		//сформировать заготовки страниц
		{
			sqlitepp::statement stm(_db, "INSERT INTO page (id) VALUES(?)");
			stm.prepare();

			for(int i(0); i<_pageIds.size(); i++)
			{
				stm.use_value(1, i);
				stm.exec();
			}
		}

		//сформировать заготовки связей страниц по фразам
		{
			sqlitepp::statement stm(_db, "INSERT INTO page_phrase_page (page1_id, page2_id,intersect1_volume,intersect2_volume,intersect3_volume) VALUES(?,?,0,0,0)");
			stm.prepare();

			for(int i(0); i<_pageIds.size(); i++)
			{
				for(int j(0); j<i; j++)
				{
					stm.use_value(1, i);
					stm.use_value(2, j);
					stm.exec();
				}
			}
		}
		return _isOk;
	}

	///////////////////////////////////////////////////
	bool ReportGenerator::setPagesContent(const utils::Variant &rows)
	{
		//перебрать строки, обновить в базе урлы и ссылки
		sqlitepp::statement stm(_db, "UPDATE page SET uri=?, volume=? WHERE id=?");
		stm.prepare();

		sqlitepp::statement stm2(_db, "INSERT OR IGNORE INTO page_ref_page (src_page_id,dst_page_id) VALUES(?,?)");
		stm2.prepare();

		BOOST_FOREACH(const utils::Variant &row, rows.as<utils::Variant::DequeVariant>())
		{
			//id, uri, ref_page_ids, text
			const utils::Variant::DequeVariant &rowv = row.as<utils::Variant::DequeVariant>();
			int srcId = pageId(rowv[0].as<boost::int64_t>());
			if(srcId >= _pageIds.size())
			{
				continue;
			}
			const std::string &uri = rowv[1].as<std::string>();
			const std::string *text = rowv[3].isNull()?NULL:&rowv[3].as<std::string>();

			//индексировать слова тут

			stm.use_value(1, uri, false);
			stm.use_value(2, text?(int)text->size():0);
			stm.use_value(3, srcId);
			stm.exec();

			if(!rowv[2].isNull())
			{
				const utils::Variant::VectorChar &refIds = rowv[2].as<utils::Variant::VectorChar>();

				for(size_t i(0); i<refIds.size(); i+=8)
				{
					boost::int64_t &i64 = *(boost::int64_t*)&refIds[i];
					int dstId = pageId(i64);
					if(dstId >= _pageIds.size())
					{
						continue;
					}

					stm2.use_value(1, srcId);
					stm2.use_value(2, dstId);
					stm2.exec();
				}
			}
		}

		return _isOk;
	}

	/////////////////////////////////////////////////////////////////////////////////
	int ReportGenerator::pageId(boost::int64_t id)
	{
		return (int)(std::lower_bound(_pageIds.begin(), _pageIds.end(), id) - _pageIds.begin());
	}

}}
