#include "pch.hpp"
#include "spider/service.hpp"
#include "spider/log.hpp"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/chrono.hpp>
#include <boost/crc.hpp>

#include "htmlcxx/html/ParserDom.h"

#include "hunspell.hxx"




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
			boost::program_options::value<std::string>()->default_value("../spell"),
			"hunspell aff path");

		options->addOption(
			"hunspell.dicpath",
			boost::program_options::value<std::string>()->default_value("../spell"),
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
		parse(resp.body(), uri.as<std::string>(), uris);

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

	//////////////////////////////////////////////////////////////////////////
	void Service::parse(http::InputMessage::Segment text, const std::string &baseUrlString, std::deque<Uri> &uris)
	{
	
		//искать <a href="...
		Uri base(baseUrlString);
		if(!base.isOk())
		{
			assert(0);
			return;
		}

		HTML::ParserDom parser;
		parser.parse(text.begin(), text.end());
		const tree<HTML::Node> &tr = parser.getTree();
		
		//выявить кодировку из заголовков, meta html
		//сформировать конвертор в utf-8 если нужен

		tree<HTML::Node>::iterator iter = tr.begin();
		tree<HTML::Node>::iterator end = tr.end();

		{
			TextParser parser;
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
						//конвертировать в utf-8 если есть конвертор
						//нормализовать символы (бин, html-entities)
						parser.push(n.text());
					}
				}
			}
			
			PhraseStreamer<1> streamer1(parser.words());
			Phrase<1> phrase1;
			while(!streamer1.next(phrase1))
			{
				//process phrase1
				assert(0);
			}
			
			PhraseStreamer<2> streamer2(parser.words());
			Phrase<2> phrase2;
			while(!streamer2.next(phrase2))
			{
				//process phrase2
				assert(0);
			}
			
		}

		std::deque<Word> words;
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
					processWords(n.text(), words);
				}
			}

		}
	}

	namespace
	{
		boost::uint32_t hashWord(const char *src)
		{
			boost::crc_32_type  result;
			for(; *src; src++)
			{
				result.process_byte(*src);
			}

			return result.checksum();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void Service::processWords(const std::string &text, std::deque<Word> &words)
	{
		std::string::size_type pos = text.find_first_of(" \t\v");
		std::string::size_type prevPos = pos;
		while(std::string::npos != pos)
		{
			if(prevPos < pos)
			{
				std::string wordSrc(text.begin()+prevPos, text.begin()+pos);
				const char *wordSrcp(wordSrc.data());
				if(_hunspell)
				{
					//сформировать набор значений

					std::set<boost::uint32_t> means;

					char ** result, **result2;
					///////////////////////////////////////////
					int ns = ((Hunspell *)_hunspell)->suggest(&result, wordSrcp);
					for(; *result; result++)
					{
						means.insert(hashWord(*result));
					}
					((Hunspell *)_hunspell)->free_list(&result, ns);

					///////////////////////////////////////////
					ns = ((Hunspell *)_hunspell)->analyze(&result, wordSrcp);
					for(; *result; result++)
					{
						means.insert(hashWord(*result));
					}

					///////////////////////////////////////////
					int ns2 = ((Hunspell *)_hunspell)->stem(&result2, result, ns);
					for(; *result2; result2++)
					{
						means.insert(hashWord(*result2));
					}
					((Hunspell *)_hunspell)->free_list(&result, ns);
					((Hunspell *)_hunspell)->free_list(&result2, ns2);

					if(means.empty())
					{
						means.insert(hashWord(wordSrcp));
					}

					//слить
					{
						std::set<boost::uint32_t>::iterator iter = means.begin();
						std::set<boost::uint32_t>::iterator end = means.end();

						Word w;
						size_t idx(0);
						for(; iter!=end; ++iter)
						{
							w._means[idx] = *iter;
							if(idx >= Word::_meansAmount)
							{
								break;
							}
							idx++;
						}
						for(; idx < Word::_meansAmount; ++idx)
						{
							w._means[idx] = 0;
						}
						words.push_back(w);
					}
				}
				else
				{
					Word w;
					size_t idx(0);
					w._means[idx] = hashWord(wordSrcp);
					idx++;

					for(; idx < Word::_meansAmount; ++idx)
					{
						w._means[idx] = 0;
					}
					words.push_back(w);
				}
			}

			prevPos = pos;
			pos = text.find_first_of(" \t\v", pos);
		}

	}


}
