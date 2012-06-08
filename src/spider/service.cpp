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
			boost::program_options::value<size_t>()->default_value(50),
			"maximum number of connections to postgres database");

		options->addOption(
			"net.concurrency",
			boost::program_options::value<size_t>()->default_value(150),
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
		async::spawn(boost::bind(&Service::processOne, this, 0, 0, "http://127.0.0.1:8080/test.html"));
		return;
		for(;;)
		{
			//контроль окончания
			{
				async::Mutex::ScopedLock sl(_mtx);
				if(_stop)
				{
					_stop = false;
					break;
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

			{
				async::Mutex::ScopedLock sl(_mtxWorkers);
				_numWorkers++;
			}

			//выбрать хост-урл
			pgc::Connection c = _db.allocConnection();

			pgc::Result res;
			//res = c.query("BEGIN");
			//if(pgc::ersError == res.status()) TLOG(res.errorMsg());

			//выбрать самый старый хост у которого есть неотработанные страницы
			res = c.query("SELECT "
				"h.id "
				"FROM host AS h "
				"WHERE new_pages_count>0 AND (CURRENT_TIMESTAMP-$1::interval)>h.atime "
				"ORDER BY new_pages_count ASC "
				"LIMIT 10 "
				" ", utils::Variant(boost::posix_time::seconds((long)_net_hostDelay))).data();
			if(pgc::ersError == res.status()) TLOG(res.errorMsg());


			utils::Variant hostId;
			utils::Variant pageId;
			utils::Variant pageUrl;
			for(size_t hrow(0); hrow < res.rows(); hrow++)
			{
				res.fetch(hostId, 0, hrow);

				pgc::Result res2 = c.query("SELECT "
					"p.id, p.url "
					"FROM page AS p  "
					"WHERE p.host_id=$1 AND p.status IS NULL "
					"LIMIT 1 "
					" ", utils::MVA(hostId, utils::Variant(boost::posix_time::seconds((long)_net_hostDelay)))).data();
				if(pgc::ersError == res2.status()) TLOG(res.errorMsg());

				if(res2.rows())
				{
					res2.fetch(pageId, 0, 0);
					res2.fetch(pageUrl, 1, 0);

					res2 = c.query("UPDATE host SET atime=CURRENT_TIMESTAMP WHERE id=$1", hostId).data();
					if(pgc::ersError == res2.status()) TLOG(res.errorMsg());
					res2 = c.query("UPDATE page SET atime=CURRENT_TIMESTAMP, status='pend' WHERE id=$1", pageId).data();
					if(pgc::ersError == res2.status()) TLOG(res.errorMsg());
					break;
				}
			}

			if(pageUrl.isNull())
			{
				//нет хостов, подождать немного
				//res = c.query("ROLLBACK");
				//if(pgc::ersError == res.status()) TLOG(res.errorMsg());
				async::timeout(100).wait();
				{
					async::Mutex::ScopedLock sl(_mtxWorkers);
					_numWorkers--;
				}
				continue;
			}

			//res = c.query("COMMIT").data();
			//if(pgc::ersError == res.status()) TLOG(res.errorMsg());

			async::spawn(boost::bind(&Service::processOne, this, hostId, pageId, pageUrl));
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
	void Service::processOne(utils::Variant hostId, utils::Variant pageId, utils::Variant uri)
	{
		boost::chrono::time_point<boost::chrono::system_clock> start = 
			boost::chrono::system_clock::now();

		http::client::Response resp;
		boost::system::error_code ec = _htc.get(resp, uri.as<std::string>().data());

		boost::chrono::milliseconds getTime = boost::chrono::duration_cast<boost::chrono::milliseconds>(boost::chrono::system_clock::now() - start);

		if(ec)
		{
			WLOG("get: "<<ec<<", ["<<uri.as<std::string>()<<"]");
			
			pgc::Result res = _db.allocConnection().data().query("UPDATE page SET status='get failed' WHERE id=$1", pageId);
			if(pgc::ersError == res.status()) TLOG(res.errorMsg());

			{
				async::Mutex::ScopedLock sl(_mtxWorkers);
				_numWorkers--;
			}
			_evtWorkerDone.set();
			return;
		}

		ec = resp.readHeaders();
		if(ec)
		{

			WLOG("read headers: "<<ec<<", ["<<uri.as<std::string>()<<"]");
			pgc::Result res = _db.allocConnection().data().query("UPDATE page SET status='read failed' WHERE id=$1", pageId);
			if(pgc::ersError == res.status()) TLOG(res.errorMsg());

			{
				async::Mutex::ScopedLock sl(_mtxWorkers);
				_numWorkers--;
			}
			_evtWorkerDone.set();
			return;
		}

		http::HeaderValue<http::Unsigned> length = resp.header(http::hn::contentLength);
		if(length.isCorrect() && length.value() > 1024*1024)
		{
			WLOG("too big: "<<length.value()<<", ["<<uri.as<std::string>()<<"]");
			pgc::Result res = _db.allocConnection().data().query("UPDATE page SET status='too big' WHERE id=$1", pageId);
			if(pgc::ersError == res.status()) TLOG(res.errorMsg());

			{
				async::Mutex::ScopedLock sl(_mtxWorkers);
				_numWorkers--;
			}
			_evtWorkerDone.set();
			return;
		}

		const http::client::Response::Segment *contentType = resp.header(http::hn::contentType);
		if(!isGoodContentType(contentType))
		{
			WLOG("bad ct: "<<(contentType?std::string(contentType->begin(), contentType->end()):std::string("absent"))<<", ["<<uri.as<std::string>()<<"]");
			pgc::Result res = _db.allocConnection().data().query("UPDATE page SET status='bad ct' WHERE id=$1", pageId);
			if(pgc::ersError == res.status()) TLOG(res.errorMsg());

			{
				async::Mutex::ScopedLock sl(_mtxWorkers);
				_numWorkers--;
			}
			_evtWorkerDone.set();
			return;
		}

		ec = resp.readBody();
		if(ec)
		{

			WLOG("read body: "<<ec<<", ["<<uri.as<std::string>()<<"]");
			pgc::Result res = _db.allocConnection().data().query("UPDATE page SET status='read failed' WHERE id=$1", pageId);
			if(pgc::ersError == res.status()) TLOG(res.errorMsg());

			{
				async::Mutex::ScopedLock sl(_mtxWorkers);
				_numWorkers--;
			}
			_evtWorkerDone.set();
			return;
		}


		//TLOG(std::string(resp.firstLine().begin(), resp.firstLine().end()));

		//parse
		std::deque<Uri> uris;
		parse(resp, uri.as<std::string>(), uris);

		//store urls
		pgc::Connection c = _db.allocConnection();


		pgc::Result res = c.query("UPDATE host SET address=$2 WHERE id=$1", 
			utils::MVA(hostId, 
				resp.channel().endpointRemote().address().to_string(ec)));
		if(pgc::ersError == res.status()) TLOG(res.errorMsg());

		//c.query("BEGIN").wait();

		BOOST_FOREACH(Uri&u, uris)
		{
			if(u.scheme()!="http" && u.scheme()!="https")
			{
				continue;
			}

			std::string uri = u.unparse(Uri::REMOVE_FRAGMENT);

			res = c.query("SELECT id FROM host WHERE name=$1", u.hostname());
			if(pgc::ersError == res.status()) TLOG(res.errorMsg());
			if(!res.rows())
			{
				res = c.query("INSERT INTO host (name, atime) VALUES ($1, CURRENT_TIMESTAMP)", u.hostname());
				if(pgc::ersError == res.status()) TLOG(res.errorMsg()<<", "<<u.hostname());
				if(pgc::ersError == res.status())
				{
					continue;
				}
				res = c.query("SELECT id FROM host WHERE name=$1", u.hostname());
				if(pgc::ersError == res.status()) TLOG(res.errorMsg());
			}
			utils::Variant hostId2;
			res.fetch(hostId2, 0,0);

			res = c.query("SELECT id, count FROM page WHERE url=$1", uri);
			if(pgc::ersError == res.status()) TLOG(res.errorMsg());
			if(!res.rows())
			{
				res = c.query("UPDATE host SET new_pages_count=new_pages_count+1 WHERE id=$1", hostId2);
				if(pgc::ersError == res.status()) TLOG(res.errorMsg());

				res = c.query("INSERT INTO page (host_id, url, count) VALUES ($1, $2, 1)", utils::MVA(hostId2, uri));
				if(pgc::ersError == res.status()) TLOG(res.errorMsg()<<", "<<uri);
				if(pgc::ersError == res.status())
				{
					continue;
				}

				res = c.query("SELECT currval('page_id_seq'::regclass) AS id");
				if(pgc::ersError == res.status()) TLOG(res.errorMsg()<<", "<<uri);
				if(pgc::ersError == res.status())
				{
					continue;
				}
				utils::Variant pageId2;
				res.fetch(pageId2, 0,0);

				res = c.query("INSERT INTO reference (from_id, to_id) VALUES ($1, $2)", utils::MVA(pageId, pageId2));
				if(pgc::ersError == res.status()) TLOG(res.errorMsg()<<", "<<uri);
				if(pgc::ersError == res.status())
				{
					continue;
				}
			}
			else
			{
				utils::Variant pageId2;
				res.fetch(pageId2, 0,0);
				res = c.query("UPDATE page SET count=count+1 WHERE id=$1", pageId2);
				if(pgc::ersError == res.status()) TLOG(res.errorMsg());

				res = c.query("INSERT INTO reference (from_id, to_id) VALUES ($1, $2)", utils::MVA(pageId, pageId2));
				if(pgc::ersError == res.status()) TLOG(res.errorMsg()<<", "<<uri);
				if(pgc::ersError == res.status())
				{
					continue;
				}

			}
		}

		res = c.query("UPDATE page SET atime=CURRENT_TIMESTAMP, status=$2, get_time=$3, body_length=$4, headers=$5 WHERE id=$1", 
			utils::MVA(
				pageId,
				std::string(resp.firstLine().begin(), resp.firstLine().end()),
				getTime.count(),
				resp.body().empty()?0:resp.body().size(),
				std::string(resp.headers().begin(), resp.headers().end())));
		if(pgc::ersError == res.status()) TLOG(res.errorMsg());

		TLOG("processed: ("<<_numWorkers<<") "<<uri.as<std::string>());


		//res = c.query("COMMIT");
		//if(pgc::ersError == res.status()) TLOG(res.errorMsg());

		{
			async::Mutex::ScopedLock sl(_mtxWorkers);
			_numWorkers--;
		}
		_evtWorkerDone.set();
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
	}

	//////////////////////////////////////////////////////////////////////////
	void Service::parse(http::client::Response resp, const std::string &baseUrlString, std::deque<Uri> &uris)
	{
	
		//искать <a href="...
		Uri base(baseUrlString);
		if(!base.isOk())
		{
			assert(0);
			return;
		}

		HTML::ParserDom parser;
		parser.parse(resp.body().begin(), resp.body().end());
		const tree<HTML::Node> &tr = parser.getTree();
		
		//выявить кодировку из заголовков, meta html
		//сформировать конвертор в utf-8 если нужен
		boost::shared_ptr<htmlcxx::CharsetConverter> cc;
		const http::InputMessage::Segment *cts = resp.header(http::hn::contentType);
		if(cts)
		{
			cc = convertorFromContentType(std::string(cts->begin(), cts->end()));
		}

		if(!cc)
		{
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
							cc = convertorFromContentType(n.attribute("content").second);
							if(cc)
							{
								break;
							}
						}
					}
				}
			}
		}

		tree<HTML::Node>::iterator iter = tr.begin();
		tree<HTML::Node>::iterator end = tr.end();

		{
			TextParser parser((Hunspell *)_hunspell);
			for(; iter!=end; ++iter)
			{
				const HTML::Node &n = *iter;
				if(n.isTag())
				{
					if("A" == n.tagName())
					{
						std::pair<bool, std::string> p = n.attribute("href");
						if(p.first)
						{
							Uri uri(p.second);
							if(uri.isOk())
							{
								uris.push_back(uri.absolute(base));
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
			

			//////////////////////////////////////////////////////////////////////////
			{
				std::map<Word, double> weights_w;

				std::cout<<"phrase streamer \n";
				PhraseStreamer<3,1,1> streamer(&parser.result());
				Phrase<3> phrase;
				while(streamer.next(phrase))
				{
					std::cout<<"phrase --------------------------------------- \n";
					const Word *words[3];
					while(phrase.nextCombination(words))
					{
						std::cout<<"combination --------------------------------------- \n";
						std::cout<<words[0]->_text<<", ";
						std::cout<<words[1]->_text<<", ";
						std::cout<<words[2]->_text;
						std::cout<<std::endl;

						weights_w[*words[0]] += 1.0;
						weights_w[*words[1]] += 1.0;
						weights_w[*words[2]] += 1.0;
					}
					std::cout<<std::endl;
				}

				////////////////
				std::cout<<"weights --------------------------------------- \n";
				std::map<Word, double>::iterator iter = weights_w.begin();
				std::map<Word, double>::iterator end = weights_w.end();
				for(; iter!=end; iter++)
				{
					std::cout<<iter->second<<"\t"<<iter->first._text<<"\n";
				}
			}
			
		}
		exit(0);
	}
}
