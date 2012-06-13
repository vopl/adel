#include "pch.hpp"
#include "spider/service.hpp"
#include "spider/log.hpp"

#include "spider/textParser.hpp"
#include "spider/phraseStreamer.hpp"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/chrono.hpp>
#include <boost/crc.hpp>
#include <boost/algorithm/string.hpp>

#include "htmlcxx/html/ParserDom.h"
#include "htmlcxx/html/CharsetConverter.h"
#include "htmlcxx/html/utils.h"

#include "utils/htmlEntities.hpp"

#ifdef near
#	undef near
#endif
#include <hunspell.hxx>

#include <iconv.h>




namespace spider
{
	///////////////////////////////////////////////////////////////////
	utils::OptionsPtr Service::prepareOptions(const char *prefix)
	{
		utils::OptionsPtr options(new utils::Options(prefix));

		options->addOption(
			"pgc.connectionString",
			boost::program_options::value<std::string>()->default_value("host=localhost port=5432 dbname=spider user=spider password=spider"),
			"connection string for postgres database");

		options->addOption(
			"pgc.maxConnections",
			boost::program_options::value<size_t>()->default_value(10),
			"maximum number of connections to postgres database");

		options->addOption(
			"net.concurrency",
			boost::program_options::value<size_t>()->default_value(50),
			"maximum number of parallel network connections");

		options->addOption(
			"net.hostDelay",
			boost::program_options::value<size_t>()->default_value(1),
			"delay between requests to one host, seconds");

		options->addOption(
			"hunspell.affpath",
			boost::program_options::value<std::string>()->default_value("../spell/ru_RU.aff"),
			"hunspell aff path");

		options->addOption(
			"hunspell.dicpath",
			boost::program_options::value<std::string>()->default_value("../spell/ru_RU.dic"),
			"hunspell dic path");

		return options;

	}

	///////////////////////////////////////////////////////////////////
	Service::Service(utils::OptionsPtr optionsPtr, http::Client htc)
		: _evtWorkerDone(true)
		, _numWorkers(0)
		, _htc(htc)
		, _hunspell(NULL)
	{
		utils::Options &o = *optionsPtr;
		
		_pgc_connectionString = o["pgc.connectionString"].as<std::string>();
		_pgc_maxConnections = o["pgc.maxConnections"].as<size_t>();
		_net_concurrency = o["net.concurrency"].as<size_t>();
		_net_hostDelay = o["net.hostDelay"].as<size_t>();

		_hunspell = new Hunspell(
			o["hunspell.affpath"].as<std::string>().c_str(),
			o["hunspell.dicpath"].as<std::string>().c_str());




		_stInsertWord2ToPage_tmp = pgc::Statement("INSERT INTO word2_to_page_tmp (page_id,word1,word2) VALUES ($1::bigint,$2::int4,$3::int4)");
		_stInsertWord3ToPage_tmp = pgc::Statement("INSERT INTO word3_to_page_tmp (page_id,word1,word2,word3) VALUES ($1::bigint,$2::int4,$3::int4,$4::int4)");

		_stBegin = pgc::Statement("BEGIN");
		_stCommit = pgc::Statement("COMMIT");
		_stRollback = pgc::Statement("ROLLBACK");
		_stLockSite = pgc::Statement("LOCK TABLE site");
		_stLockPage = pgc::Statement("LOCK TABLE page");

		_stSelectPages4Process = pgc::Statement("SELECT "
			"	DISTINCT ON(log(s.amount_page_new+1)*s.priority, s.id)"
			"	s.id AS site_id, p.id AS page_id, p.uri "
			"FROM"
			"	page AS p "
			"LEFT JOIN "
			"	site AS s ON(s.id=p.site_id) "
			"WHERE "
			"	((s.time_access+s.time_per_page)<CURRENT_TIMESTAMP AND p.status IS NULL) OR"
			"	((s.time_access+INTERVAL '1 hour')<CURRENT_TIMESTAMP AND p.status='pend') "
			"ORDER BY "
			"	log(s.amount_page_new+1)*s.priority DESC, "
			"	s.id ASC "
			"LIMIT"
			"	1 "
			" ");
		_stUpdateSiteTime = pgc::Statement("UPDATE site SET time_access=CURRENT_TIMESTAMP WHERE id=$1::bigint");
		_stUpdatePageStatus = pgc::Statement("UPDATE page SET status=$2::character varying WHERE id=$1::bigint");
		_stUpdatePageAddress = pgc::Statement("UPDATE site SET address=$2 WHERE id=$1");
		_stSelectSiteIdWhereName = pgc::Statement("SELECT id FROM site WHERE name=$1");
		_stInsertSite = pgc::Statement("INSERT INTO site (name, time_access) VALUES ($1, CURRENT_TIMESTAMP)");
		_stSelectPageIdWhereUri = pgc::Statement("SELECT id FROM page WHERE uri=$1");
		_stUpdateSiteAmountPlusOne = pgc::Statement("UPDATE site SET "
			"amount_page_new=amount_page_new+1,"
			"amount_page_all=amount_page_all+1 "
			"WHERE id=$1");
		_stUpdateSiteAmountMinusOne = pgc::Statement("UPDATE site SET amount_page_new=amount_page_new-1 WHERE id=$1");
		_stInsertPage = pgc::Statement("INSERT INTO page (site_id, uri) VALUES ($1, $2)");
		_stInsertReference = pgc::Statement("INSERT INTO reference (from_id, to_id) VALUES ($1, $2)");
		_stUpdatePage = pgc::Statement("UPDATE page SET status=$2, get_time=$3, body_length=$4, headers=$5 WHERE id=$1");

	}

	///////////////////////////////////////////////////////////////////
	Service::~Service()
	{
		_db.reset();

		if(_hunspell)
		{
			delete ((Hunspell *)_hunspell);
		}
	}

	///////////////////////////////////////////////////////////////////
	void Service::start()
	{
		{
			async::Mutex::ScopedLock sl(_mtx);
			_stop = false;
		}

		_db = pgc::Db(
			_pgc_connectionString.data(),
			_pgc_maxConnections,
			boost::bind(&Service::onConnectionMade, this, _1),
			boost::bind(&Service::onConnectionLost, this, _1));

		async::spawn(boost::bind(&Service::processLoop, this));
	}

	///////////////////////////////////////////////////////////////////
	void Service::stop()
	{
		{
			async::Mutex::ScopedLock sl(_mtx);
			_stop = true;
		}
		for(;;)
		{
			{
				async::Mutex::ScopedLock sl(_mtx);
				if(!_stop)
				{
					break;
				}
			}

			async::timeout(100).wait();
		}
		_db.reset();
	}

	///////////////////////////////////////////////////////////////////
	void Service::onConnectionMade(size_t numConnections)
	{

	}

	///////////////////////////////////////////////////////////////////
	void Service::onConnectionLost(size_t numConnections)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	void Service::processLoop()
	{
		for(;;)
		{
			//контроль окончания
			bool stop = false;
			{
				async::Mutex::ScopedLock sl(_mtx);
				if(_stop)
				{
					_stop = false;
					stop = true;
				}
			}

			if(stop)
			{
				for(;;)
				{
					{
						async::Mutex::ScopedLock sl(_mtxWorkers);
						if(!_numWorkers)
						{
							break;
						}
					}
					_evtWorkerDone.wait();
				}
			}

			//контроль переполнения пула воркеров
			bool needWaitWorker = false;
			{
				async::Mutex::ScopedLock sl(_mtxWorkers);
				if(_numWorkers >= _net_concurrency)
				{
					needWaitWorker = true;
				}
			}
			if(needWaitWorker)
			{
				_evtWorkerDone.wait();
				continue;
			}

			//выбрать хост-урл
			pgc::Connection c = _db.allocConnection();

			pgc::Result res;

#define CHECK_PGR_NORETURN(...) {pgc::Result r = __VA_ARGS__; if(pgc::ersError == r.status()) {TLOG(__LINE__<<", "<<r.errorMsg());}}
#define CHECK_PGR(...) {pgc::Result r = __VA_ARGS__; if(pgc::ersError == r.status()) {TLOG(__LINE__<<", "<<r.errorMsg());return;}}

			CHECK_PGR(c.query(_stBegin));
			CHECK_PGR(c.query(_stLockSite));
			CHECK_PGR(c.query(_stLockPage));

			//выбрать страницы
			res = c.query(_stSelectPages4Process).data();
			CHECK_PGR(res);


			if(res.rows())
			{
				utils::Variant siteId;
				utils::Variant pageId;
				utils::Variant pageUrl;
				for(size_t hrow(0); hrow < res.rows(); hrow++)
				{
					res.fetch(siteId, 0, hrow);
					res.fetch(pageId, 1, hrow);
					res.fetch(pageUrl, 2, hrow);

					CHECK_PGR(c.query(_stUpdateSiteTime, siteId));
					CHECK_PGR(c.query(_stUpdatePageStatus, MVA(pageId, "pend")));

					{
						async::Mutex::ScopedLock sl(_mtx);
						_numWorkers++;
					}
					async::spawn(boost::bind(&Service::processOne, this, siteId, pageId, pageUrl));
				}
				CHECK_PGR(c.query(_stCommit));
			}
			else
			{
				CHECK_PGR(c.query(_stRollback));
				c.reset();

				async::timeout(1000).wait();
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool isGoodContentType(const http::client::Response::Segment *seg)
	{
		if(!seg)
		{
			return false;
		}

		std::string ct(seg->begin(), seg->end());

		return
			std::string::npos != ct.find("html") ||
			std::string::npos != ct.find("text") ||
			std::string::npos != ct.find("xml");
	}


	//////////////////////////////////////////////////////////////////////////
	namespace {
		class WorkerRaii
		{
			async::Mutex	&_mtx;
			size_t			&_numWorkers;
			async::Event	&_evtDone;
		public:
			WorkerRaii(async::Mutex &mtx, size_t &numWorkers, async::Event &evtDone)
				: _mtx(mtx)
				, _numWorkers(numWorkers)
				, _evtDone(evtDone)
			{
			}

			~WorkerRaii()
			{
				{
					async::Mutex::ScopedLock sl(_mtx);
					_numWorkers--;
				}
				_evtDone.set();
			}
		};
	}
	//////////////////////////////////////////////////////////////////////////
	void Service::processOne(utils::Variant hostId, utils::Variant pageId, utils::Variant uri)
	{
		WorkerRaii wr(_mtxWorkers, _numWorkers, _evtWorkerDone);

		boost::chrono::time_point<boost::chrono::system_clock> start = 
			boost::chrono::system_clock::now();

		http::client::Response resp;
		boost::system::error_code ec = _htc.get(resp, uri.as<std::string>().data());

		boost::chrono::milliseconds getTime = boost::chrono::duration_cast<boost::chrono::milliseconds>(boost::chrono::system_clock::now() - start);

		pgc::Connection c = _db.allocConnection();
		if(ec)
		{
			WLOG("get: "<<ec<<", ["<<uri.as<std::string>()<<"]");
			CHECK_PGR(c.query(_stUpdatePageStatus, utils::MVA(pageId, "get failed")));
			return;
		}
		ec = resp.readFirstLine();
		if(ec)
		{
			WLOG("read first line: "<<ec<<", ["<<uri.as<std::string>()<<"]");
			CHECK_PGR(c.query(_stUpdatePageStatus, utils::MVA(pageId, "read first line failed")));
			return;
		}
		if(std::string::npos == std::string(resp.firstLine().begin(), resp.firstLine().end()).find("200"))
		{
			WLOG("bad status: "<<std::string(resp.firstLine().begin(), resp.firstLine().end())<<", ["<<uri.as<std::string>()<<"]");
			CHECK_PGR(c.query(_stUpdatePageStatus, utils::MVA(pageId, "bad status")));
			return;
		}

		ec = resp.readHeaders();
		if(ec)
		{

			WLOG("read headers: "<<ec<<", ["<<uri.as<std::string>()<<"]");
			CHECK_PGR(c.query(_stUpdatePageStatus, utils::MVA(pageId, "read headers failed")));
			return;
		}


		http::HeaderValue<http::Unsigned> length = resp.header(http::hn::contentLength);
		if(length.isCorrect() && length.value() > 1024*1024)
		{
			WLOG("too big: "<<length.value()<<", ["<<uri.as<std::string>()<<"]");
			CHECK_PGR(c.query(_stUpdatePageStatus, utils::MVA(pageId, "too big")));
			return;
		}

		const http::client::Response::Segment *contentType = resp.header(http::hn::contentType);
		if(!isGoodContentType(contentType))
		{
			WLOG("bad ct: "<<(contentType?std::string(contentType->begin(), contentType->end()):std::string("absent"))<<", ["<<uri.as<std::string>()<<"]");
			CHECK_PGR(c.query(_stUpdatePageStatus, utils::MVA(pageId, "bad content type")));
			return;
		}

		ec = resp.readBody();
		if(ec)
		{

			WLOG("read body: "<<ec<<", ["<<uri.as<std::string>()<<"]");
			CHECK_PGR(c.query(_stUpdatePageStatus, utils::MVA(pageId, "read body failed")));
			return;
		}


		//parse
		std::deque<Uri> uris;
		std::deque<WordBucket> wordBuckets;
		parse(resp, uri.as<std::string>(), uris, wordBuckets);

		//store urls



		CHECK_PGR(c.query(_stBegin));
		CHECK_PGR(c.query(_stLockSite));
		CHECK_PGR(c.query(_stLockPage));

		CHECK_PGR(c.query(_stUpdatePageAddress,
			utils::MVA(hostId,
				resp.channel().endpointRemote().address().to_string(ec))));

		BOOST_FOREACH(Uri&u, uris)
		{
			if(u.scheme()!="http" && u.scheme()!="https")
			{
				continue;
			}

// 			if(u.hostname().find(".ru") == std::string::npos)
// 			{
// 				continue;
// 			}

  			if(u.hostnameWithPort() != "127.0.0.1:8080")
  			{
  				continue;
  			}

			std::string uri = Uri::encode(Uri::decode(u.unparse(Uri::REMOVE_FRAGMENT)));

			utils::Variant hostId2;
			utils::Variant pageId2;

			pgc::Result res = c.query(_stSelectSiteIdWhereName, u.hostnameWithPort());
			CHECK_PGR(res);

			if(!res.rows())
			{
				res = c.query(_stInsertSite, u.hostnameWithPort());
				CHECK_PGR(res);

				res = c.query(_stSelectSiteIdWhereName, u.hostnameWithPort());
				CHECK_PGR(res);
			}
			res.fetch(hostId2, 0,0);

			res = c.query(_stSelectPageIdWhereUri, uri);
			CHECK_PGR(res);

			if(!res.rows())
			{

				res = c.query(_stUpdateSiteAmountPlusOne, hostId2);
				CHECK_PGR(res);

				res = c.query(_stInsertPage, utils::MVA(hostId2, uri));
				CHECK_PGR(res);

				res = c.query(_stSelectPageIdWhereUri, uri);
				CHECK_PGR(res);

				res.fetch(pageId2, 0,0);
			}
			else
			{
				res.fetch(pageId2, 0,0);
			}
			res = c.query(_stInsertReference, utils::MVA(pageId, pageId2));
			CHECK_PGR(res);
		}


		pgc::Result res = c.query(_stUpdatePage,
			utils::MVA(
				pageId,
				std::string(resp.firstLine().begin(), resp.firstLine().end()),
				getTime.count(),
				resp.body().empty()?0:resp.body().size(),
				std::string(resp.headers().begin(), resp.headers().end())));
		CHECK_PGR(res);

		res = c.query(_stUpdateSiteAmountMinusOne, hostId);
		CHECK_PGR(res);

		CHECK_PGR(c.query(_stCommit));
		c.reset();

		c = _db.allocConnection();
		CHECK_PGR(c.query(_stBegin));
		//CHECK_PGR(c.query(_stLockWord2));
		//CHECK_PGR(c.query(_stLockWord3));
		updatePageWords(c, pageId, wordBuckets);
		CHECK_PGR(c.query(_stCommit));

		TLOG("processed: ("<<_numWorkers<<") "<<uri.as<std::string>());
	}

	namespace
	{
		boost::shared_ptr<htmlcxx::CharsetConverter> convertorFromContentType(const std::string &contentType)
		{
			boost::shared_ptr<htmlcxx::CharsetConverter> cc;

			std::string::size_type pos = contentType.find("charset=");
			if(std::string::npos != pos)
			{
				std::string charset(contentType.begin()+pos+8, contentType.end());

				charset = iconv_canonicalize(charset.data());

				cc.reset(new htmlcxx::CharsetConverter(charset, "UTF-8//IGNORE"));

				if(!cc->isOk())
				{
					cc.reset();
				}

				return cc;
			}
			return cc;
		}

		boost::shared_ptr<htmlcxx::CharsetConverter> convertorFromContentTypeOrDom(
			const http::InputMessage::Segment *cts,
			const tree<HTML::Node> &tr)
		{
			if(cts)
			{
				boost::shared_ptr<htmlcxx::CharsetConverter> cc;
				cc = convertorFromContentType(std::string(cts->begin(), cts->end()));
				if(cc && cc->isOk())
				{
					return cc;
				}
			}

			tree<HTML::Node>::iterator iter = tr.begin();
			tree<HTML::Node>::iterator end = tr.end();
			for(; iter!=end; ++iter)
			{
				HTML::Node &n = *iter;
				if(n.isTag())
				{
					if(boost::algorithm::iequals(n.tagName(), "meta"))
					{
						n.parseAttributes();
						if(boost::algorithm::iequals(n.attribute("http-equiv").second, "Content-Type"))
						{
							boost::shared_ptr<htmlcxx::CharsetConverter> cc;
							cc = convertorFromContentType(n.attribute("content").second);
							if(cc && cc->isOk())
							{
								return cc;
							}
						}
					}
				}
			}
			return boost::shared_ptr<htmlcxx::CharsetConverter>();
		}



		std::map<std::string, bool> init_skipTextInTag()
		{
			std::map<std::string, bool> res;
			res["script"] = true;
			res["style"] = true;
			res["meta"] = true;

			return res;
		}
		std::map<std::string, bool> map_skipTextInTag = init_skipTextInTag();

		bool skipTextInTag(const std::string &tagName)
		{
			std::string lowerTagName = tagName;
			boost::algorithm::to_lower(lowerTagName);
			return map_skipTextInTag.end() != map_skipTextInTag.find(lowerTagName);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void Service::parse(
		http::client::Response resp,
		const std::string &baseUrlString,
		std::deque<Uri> &uris,
		std::deque<WordBucket> &wordBuckets)
	{
	
		//подготовить базовый урл
		Uri base(baseUrlString);
		if(!base.isOk())
		{
			assert(0);
			return;
		}

		//парсить dom
		HTML::ParserDom parser;
		parser.parse(resp.body().begin(), resp.body().end());
		const tree<HTML::Node> &tr = parser.getTree();
		
		//выявить кодировку из заголовков, meta html
		//сформировать конвертор в utf-8 если нужен
		boost::shared_ptr<htmlcxx::CharsetConverter> cc =
			convertorFromContentTypeOrDom(resp.header(http::hn::contentType), tr);

		//перебирать ноды, перерабатывать каждую
		tree<HTML::Node>::iterator iter = tr.begin();
		tree<HTML::Node>::iterator end = tr.end();
		tree<HTML::Node>::iterator parent;

		{
			TextParser parser((Hunspell *)_hunspell, wordBuckets);
			for(; iter!=end; ++iter)
			{
				HTML::Node &n = *iter;
				if(n.isTag())
				{
					if(boost::algorithm::iequals(n.tagName(), "a"))
					{
						n.parseAttributes();
						std::pair<bool, std::string> p = n.attribute("href");
						if(p.first)
						{

							Uri uri(HTML::convert_link(p.second, base));
							if(uri.isOk())
							{
								uris.push_back(uri);
							}
						}
					}
				}
				else
				{
					if(!n.isComment())
					{

						if(!n.text().empty())
						{
							parent = tr.parent(iter);
							if(parent != tr.end() && parent->isTag() && skipTextInTag(parent->tagName()))
							{
								continue;
							}

							//конвертировать в utf-8 если есть конвертор
							//декодировать html-entities
							if(cc)
							{
								parser.push(utils::htmlEntitiesDecode(cc->convert(n.text())));
							}
							else
							{
								parser.push(utils::htmlEntitiesDecode(n.text()));
							}
						}
					}
				}
			}
		}
	}

	bool Service::updatePageWords(pgc::Connection c, const utils::Variant &pageId, const std::deque<WordBucket> &wordBuckets)
	{
		if(!updatePageWords2<PhraseStreamer<2,0> >(c, pageId, wordBuckets))
		{
			return false;
		}
		if(!updatePageWords2<PhraseStreamer<2,1> >(c, pageId, wordBuckets))
		{
			return false;
		}
		if(!updatePageWords2<PhraseStreamer<2,2> >(c, pageId, wordBuckets))
		{
			return false;
		}

		/*if(!updatePageWords3<PhraseStreamer<3,0,0> >(c, pageId, wordBuckets))
		{
			return false;
		}
		if(!updatePageWords3<PhraseStreamer<3,0,1> >(c, pageId, wordBuckets))
		{
			return false;
		}
		if(!updatePageWords3<PhraseStreamer<3,0,2> >(c, pageId, wordBuckets))
		{
			return false;
		}
		if(!updatePageWords3<PhraseStreamer<3,1,0> >(c, pageId, wordBuckets))
		{
			return false;
		}
		if(!updatePageWords3<PhraseStreamer<3,1,1> >(c, pageId, wordBuckets))
		{
			return false;
		}
		if(!updatePageWords3<PhraseStreamer<3,1,2> >(c, pageId, wordBuckets))
		{
			return false;
		}
		if(!updatePageWords3<PhraseStreamer<3,2,0> >(c, pageId, wordBuckets))
		{
			return false;
		}
		if(!updatePageWords3<PhraseStreamer<3,2,1> >(c, pageId, wordBuckets))
		{
			return false;
		}
		if(!updatePageWords3<PhraseStreamer<3,2,2> >(c, pageId, wordBuckets))
		{
			return false;
		}
		*/

		return true;
	}

	///////////////////////////////////////////////////////////////////////////////////
	bool Service::updatePageWords2(pgc::Connection c, const utils::Variant &pageId, const Word *words[2])
	{
		pgc::Result res = c.query(_stInsertWord2ToPage_tmp, utils::MVA(pageId, words[0]->_value,words[1]->_value));
		CHECK_PGR_NORETURN(res);

		if(pgc::ersError == res.status())
		{
			return false;
		}

		return true;
	}

	///////////////////////////////////////////////////////////////////////////////////
	bool Service::updatePageWords3(pgc::Connection c, const utils::Variant &pageId, const Word *words[3])
	{
		pgc::Result res = c.query(_stInsertWord3ToPage_tmp, utils::MVA(pageId, words[0]->_value,words[1]->_value,words[2]->_value));
		CHECK_PGR_NORETURN(res);

		if(pgc::ersError == res.status())
		{
			return false;
		}
		return true;

	}


}
