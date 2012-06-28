#include "pch.hpp"
#include "scom/impl/reportGenerator.hpp"
#include "scom/log.hpp"
#include "utf8proc/utf8proc.h"

#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/crc.hpp>

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

			for(boost::int32_t i(0); i<_pageIds.size(); i++)
			{
				stm.use_value(1, i);
				stm.exec();
			}
		}

		//сформировать заготовки связей страниц по фразам
		{
			sqlitepp::statement stm(_db, "INSERT INTO page_phrase_page (page1_id, page2_id,intersect1_volume,intersect2_volume,intersect3_volume) VALUES(?,?,0,0,0)");
			stm.prepare();

			for(boost::int32_t i(0); i<_pageIds.size(); i++)
			{
				for(boost::int32_t j(0); j<i; j++)
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
			boost::int32_t srcId = pageId(rowv[0].as<boost::int64_t>());
			if(srcId >= _pageIds.size())
			{
				continue;
			}
			const std::string &uri = rowv[1].as<std::string>();
			const std::string *text = rowv[3].isNull()?NULL:&rowv[3].as<std::string>();

			stm.use_value(1, uri, false);
			stm.use_value(2, text?pushPageText(srcId, *text):(boost::int32_t)0);
			stm.use_value(3, srcId);
			stm.exec();

			if(!rowv[2].isNull())
			{
				const utils::Variant::VectorChar &refIds = rowv[2].as<utils::Variant::VectorChar>();

				for(size_t i(0); i<refIds.size(); i+=8)
				{
					boost::int64_t &i64 = *(boost::int64_t*)&refIds[i];
					boost::int32_t dstId = pageId(i64);
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
	boost::int32_t ReportGenerator::pageId(boost::int64_t id)
	{
		return (boost::int32_t)(std::lower_bound(_pageIds.begin(), _pageIds.end(), id) - _pageIds.begin());
	}

	/////////////////////////////////////////////////////////////////////////////////
	boost::int32_t ReportGenerator::pushPageText(boost::int32_t id, const std::string &text)
	{
		//нормализовать, разбить по словам
		size_t bufSize = text.size()*3+16;
		boost::shared_array<int32_t> buf(new int32_t[bufSize]);
		bufSize = utf8proc_decompose((const uint8_t *)text.data(), text.length(), buf.get(), bufSize, UTF8PROC_SKIPINVALID|UTF8PROC_STABLE|UTF8PROC_LUMP);

		enum EWordType
		{
			ewtNull,
			ewtLetters,
			ewtSymbols
		} ewt = ewtNull;
		std::vector<int32_t> wordChars;
		std::deque<int32_t> compressedWords;

		for(size_t i(0); i<bufSize; i++)
		{
			const utf8proc_property_t *prop = utf8proc_get_property(buf[i]);
			if(!prop)
			{
				assert(0);
				continue;
			}

			bool space = false;
			int32_t letter = 0;
			int32_t symbol = 0;
			switch(prop->category)
			{
			case UTF8PROC_CATEGORY_LU: //Letter, Uppercase
				letter = prop->lowercase_mapping;
				break;
			case UTF8PROC_CATEGORY_LL: //Letter, lowercase
				letter = buf[i];
				break;
			case UTF8PROC_CATEGORY_LT: //Letter, titlecase
				assert(!"check me");
				letter = prop->lowercase_mapping;
				break;
			case UTF8PROC_CATEGORY_LO: //Letter, other
			case UTF8PROC_CATEGORY_LM: //Letter, modifier
				break;
			case UTF8PROC_CATEGORY_ND: //Number, decimal digit
			case UTF8PROC_CATEGORY_NL: //Number, letter
			case UTF8PROC_CATEGORY_NO: //Number other
				symbol = 'N';
				break;
			case UTF8PROC_CATEGORY_SM: //Symbol, math
			case UTF8PROC_CATEGORY_SC: //Symbol, currency
			case UTF8PROC_CATEGORY_SK: //Symbol, modifier
			case UTF8PROC_CATEGORY_SO: //Symbol, other
				symbol = buf[i];
				break;

			case UTF8PROC_CATEGORY_PC: //Punctuation, connector
			case UTF8PROC_CATEGORY_PD: //Punctuation, dash
			case UTF8PROC_CATEGORY_PS: //Punctuation, open
			case UTF8PROC_CATEGORY_PE: //Punctuation, close
			case UTF8PROC_CATEGORY_PI: //Punctuation, initial quote
			case UTF8PROC_CATEGORY_PF: //Punctuation, final quote
			case UTF8PROC_CATEGORY_PO: //Punctuation, other
				symbol = buf[i];
				break;
			case UTF8PROC_CATEGORY_MN: //Mark, non-spacing
			case UTF8PROC_CATEGORY_MC: //Mark, spacing combining
			case UTF8PROC_CATEGORY_ME: //Mark, enclosing
				symbol = buf[i];
				break;
			case UTF8PROC_CATEGORY_ZS: //Separator, space
			case UTF8PROC_CATEGORY_ZL: //Seaprator, line
			case UTF8PROC_CATEGORY_ZP: //Seaparator, paragraph
			case UTF8PROC_CATEGORY_CC: //Other, control
			case UTF8PROC_CATEGORY_CF: //Other, format
			case UTF8PROC_CATEGORY_CS: //Other, surrogate
			case UTF8PROC_CATEGORY_CO: //Other, private use
			case UTF8PROC_CATEGORY_CN: //Other, not assigned
			default:
				space = true;
				break;
			}

			switch(ewt)
			{
			case ewtNull:
				if(letter)
				{
					ewt = ewtLetters;
					wordChars.push_back(letter);
				}
				else if(symbol)
				{
					ewt = ewtSymbols;
					wordChars.push_back(symbol);
				}
				break;

			case ewtLetters:
				if(letter)
				{
					wordChars.push_back(letter);
				}
				else if(symbol)
				{
					stem(compressedWords, wordChars);
					wordChars.clear();
					ewt = ewtSymbols;
					wordChars.push_back(symbol);
				}
				break;
			case ewtSymbols:
				if(letter)
				{
					stem(compressedWords, wordChars);
					wordChars.clear();
					ewt = ewtLetters;
					wordChars.push_back(letter);
				}
				else if(symbol)
				{
					wordChars.push_back(symbol);
				}
				break;
			default:
				assert(0);
				break;
			}
			if(space)
			{
				stem(compressedWords, wordChars);
				wordChars.clear();
				ewt = ewtNull;
			}

		}
		stem(compressedWords, wordChars);
		wordChars.clear();
		ewt = ewtNull;

		//заполнять фразы
		assert(0);

		return compressedWords.size();
	}

	/////////////////////////////////////////////////////////////////////////////////
	void ReportGenerator::stem(std::deque<boost::int32_t> &compressedWords, const std::vector<int32_t> &wordChars)
	{
		if(wordChars.empty())
		{
			return;
		}

		///////////////////
		std::string encoded;
		encoded.resize(wordChars.size()*3+16);
		char *data = const_cast<char *>(encoded.data());

		for(size_t i(0); i<wordChars.size(); i++)
		{
			data += utf8proc_encode_char(wordChars[i], (uint8_t *)data);
		}
		encoded.resize(data - encoded.data());

		//std::cout<<"-------------"<<std::endl;
		//std::cout<<encoded<<std::endl;

		//////////////////
		char ** result;
		int ns = _hunspell->stem(&result, encoded.data());

		if(ns)
		{
			encoded = result[0];
			/*for(size_t i(0); i<ns; i++)
			{
				std::cout<<result[i]<<std::endl;
			}*/
			_hunspell->free_list(&result, ns);
		}

		boost::crc_32_type  crc32;
		crc32.process_bytes(encoded.data(), encoded.size());
		compressedWords.push_back(crc32.checksum());

	}

}}
