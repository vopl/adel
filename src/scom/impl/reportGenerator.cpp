#include "pch.hpp"
#include "scom/impl/reportGenerator.hpp"
#include "scom/log.hpp"
#include "utf8proc/utf8proc.h"

#ifdef _MSC_VER
#include <io.h>
#endif

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

#ifdef _MSC_VER
		p /= "rdb_XXXXXX";
		std::string str = p.string();
		std::vector<char> fnameBuf(str.begin(), str.end());
		fnameBuf.push_back(0);
		if(_mktemp_s(&fnameBuf[0], fnameBuf.size()))
		{
			ELOG("failed to create temporary file for report sqlite database: "<<strerror(errno));
			_isOk = false;
			return;
		}
#else
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
#endif
		_dbFileName.assign(fnameBuf.begin(), fnameBuf.end());

		try
		{
			_db.open(_dbFileName);
			_db<<"PRAGMA auto_vacuum = 0";
			_db<<"PRAGMA checkpoint_fullfsync = off";
			_db<<"PRAGMA fullfsync = off";
			_db<<"PRAGMA encoding = \"UTF-8\"";
			_db<<"PRAGMA ignore_check_constraints=on";
			_db<<"PRAGMA journal_mode = off";
			_db<<"PRAGMA synchronous = off";
			_db<<"PRAGMA count_changes = off";
			_db<<"PRAGMA temp_store = MEMORY";
			_db<<"PRAGMA locking_mode = EXCLUSIVE";

			_db<<"CREATE TABLE page(id INT4 PRIMARY KEY,uri VARCHAR,volume INT4)";

			_db<<"CREATE TABLE page_ref_page(src_page_id INT4, dst_page_id int4, PRIMARY KEY(src_page_id,dst_page_id) )";

			//_db<<"CREATE TABLE phrase1(id INT4 PRIMARY KEY,src1 VARCHAR,page_ids BLOB)";
			//_db<<"CREATE TABLE phrase2(id INT4 PRIMARY KEY,src1 VARCHAR,src2 VARCHAR,page_ids BLOB)";
			//_db<<"CREATE TABLE phrase3(id INT4 PRIMARY KEY,src1 VARCHAR,src2 VARCHAR,src3 VARCHAR,page_ids BLOB)";

			_db<<"CREATE TABLE page_phrase_page(page1_id INT4, page2_id INT4,intersect1_volume INT4,intersect2_volume INT4,intersect3_volume INT4)";
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
		//sqlitepp::transaction tr(_db);

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
				sqlitepp::transaction tr2(_db);
				for(boost::int32_t j(0); j<i; j++)
				{
					stm.use_value(1, j);//page1_id < page2_id
					stm.use_value(2, i);
					stm.exec();
				}
				tr2.commit();
			}
		}

		//tr.commit();

		_db<<"CREATE UNIQUE INDEX page_phrase_page_idx ON page_phrase_page (page1_id, page2_id)";

		return _isOk;
	}

	///////////////////////////////////////////////////
	bool ReportGenerator::setPagesContent(const utils::Variant &rows)
	{
		//sqlitepp::transaction tr(_db);
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

		//tr.commit();
		return _isOk;
	}

	/////////////////////////////////////////////////////////////////////////////////
	bool ReportGenerator::evalPhraseWeights()
	{
		//сортировать фразы, брать участки идентичных фраз и по ним обновлять веса кросса

		if(!evalPhraseWeights(_phrases1))
		{
			return false;
		}

		if(!evalPhraseWeights(_phrases2))
		{
			return false;
		}

		if(!evalPhraseWeights(_phrases3))
		{
			return false;
		}

		return true;
	}

	/////////////////////////////////////////////////////////////////////////////////
	boost::int32_t ReportGenerator::pageId(boost::int64_t id)
	{
		return (boost::int32_t)(std::lower_bound(_pageIds.begin(), _pageIds.end(), id) - _pageIds.begin());
	}

	/////////////////////////////////////////////////////////////////////////////////
	boost::int32_t ReportGenerator::pushPageText(boost::int32_t pageId, const std::string &text)
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

		fillPhrases(pageId, compressedWords);

		return (boost::int32_t)compressedWords.size();
	}

	/////////////////////////////////////////////////////////////////////////////////
	void ReportGenerator::stem(std::deque<boost::int32_t> &compressedWords, const std::vector<boost::int32_t> &wordChars)
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
		else
		{
			if(wordChars.size() <= 2)
			{
				return;
			}

		}

		boost::crc_32_type  crc32;
		crc32.process_bytes(encoded.data(), encoded.size());
		compressedWords.push_back(crc32.checksum());

	}

	///////////////////////////////////////////////////////////////////////////
	void ReportGenerator::fillPhrases(boost::int32_t pageId, const std::deque<boost::int32_t> &compressedWords)
	{
		size_t amount = compressedWords.size();

		// {w1}
		if(amount > 0) for(size_t i(0); i<amount; i++)
		{
			PhraseEntry<1> p1 = {pageId, {compressedWords[i]} };
			_phrases1.push_back(p1);
		}

		// {w1,w2}
		if(amount > 1) for(size_t i(0); i<amount-1; i++)
		{
			PhraseEntry<2> p2 = {pageId, {compressedWords[i],compressedWords[i+1]} };
			_phrases2.push_back(p2);
		}

		// {w1,s1,w2}
		if(amount > 2) for(size_t i(0); i<amount-2; i++)
		{
			PhraseEntry<2> p2 = {pageId, {compressedWords[i],compressedWords[i+2]} };
			_phrases2.push_back(p2);
		}

		// {w1,s1,s2,w2}
		if(amount > 3) for(size_t i(0); i<amount-3; i++)
		{
			PhraseEntry<2> p2 = {pageId, {compressedWords[i],compressedWords[i+3]} };
			_phrases2.push_back(p2);
		}

		// {w1,			w2,			w3}
		if(amount > 2) for(size_t i(0); i<amount-2; i++)
		{
			PhraseEntry<3> p3 = {pageId, {compressedWords[i],compressedWords[i+1],compressedWords[i+2]} };
			_phrases3.push_back(p3);
		}

		// {w1,			w2,s1,		w3}
		if(amount > 3) for(size_t i(0); i<amount-3; i++)
		{
			PhraseEntry<3> p3 = {pageId, {compressedWords[i],compressedWords[i+1],compressedWords[i+3]} };
			_phrases3.push_back(p3);
		}
		// {w1,			w2,s1,s2	w3}
		if(amount > 4) for(size_t i(0); i<amount-4; i++)
		{
			PhraseEntry<3> p3 = {pageId, {compressedWords[i],compressedWords[i+1],compressedWords[i+4]} };
			_phrases3.push_back(p3);
		}
		// {w1,s1,		w2,			w3}
		if(amount > 3) for(size_t i(0); i<amount-3; i++)
		{
			PhraseEntry<3> p3 = {pageId, {compressedWords[i],compressedWords[i+2],compressedWords[i+3]} };
			_phrases3.push_back(p3);
		}
		// {w1,s1		w2,s1		w3}
		if(amount > 4) for(size_t i(0); i<amount-4; i++)
		{
			PhraseEntry<3> p3 = {pageId, {compressedWords[i],compressedWords[i+2],compressedWords[i+4]} };
			_phrases3.push_back(p3);
		}
		// {w1,s1		w2,s1,s2	w3}
		if(amount > 5) for(size_t i(0); i<amount-5; i++)
		{
			PhraseEntry<3> p3 = {pageId, {compressedWords[i],compressedWords[i+2],compressedWords[i+5]} };
			_phrases3.push_back(p3);
		}
		// {w1,s1,s2,	w2,			w3}
		if(amount > 4) for(size_t i(0); i<amount-4; i++)
		{
			PhraseEntry<3> p3 = {pageId, {compressedWords[i],compressedWords[i+3],compressedWords[i+4]} };
			_phrases3.push_back(p3);
		}
		// {w1,s1,s2	w2,s1		w3}
		if(amount > 5) for(size_t i(0); i<amount-5; i++)
		{
			PhraseEntry<3> p3 = {pageId, {compressedWords[i],compressedWords[i+3],compressedWords[i+5]} };
			_phrases3.push_back(p3);
		}
		// {w1,s1,s2	w2,s1,s2	w3}
		if(amount > 6) for(size_t i(0); i<amount-6; i++)
		{
			PhraseEntry<3> p3 = {pageId, {compressedWords[i],compressedWords[i+3],compressedWords[i+6]} };
			_phrases3.push_back(p3);
		}
	}

}}
