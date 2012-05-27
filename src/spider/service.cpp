#include "pch.hpp"
#include "spider/service.hpp"
#include "spider/log.hpp"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/chrono.hpp>

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

		return options;

	}

	///////////////////////////////////////////////////////////////////
	Service::Service(utils::OptionsPtr optionsPtr, http::Client htc)
		: _evtWorkerDone(true)
		, _numWorkers(0)
		, _htc(htc)
	{
		utils::Options &o = *optionsPtr;
		
		_pgc_connectionString = o["pgc.connectionString"].as<std::string>();
		_pgc_maxConnections = o["pgc.maxConnections"].as<size_t>();
		_net_concurrency = o["net.concurrency"].as<size_t>();
		_net_hostDelay = o["net.hostDelay"].as<size_t>();
	}

	///////////////////////////////////////////////////////////////////
	Service::~Service()
	{
		_db.reset();
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

		namespace qi = boost::spirit::qi;
		using namespace qi;
		namespace px = boost::phoenix;

		bool ok = false;
		bool b = qi::parse(seg->begin(), seg->end(),
			*(
				lit("html")[px::ref(ok)=true] | lit("text")[px::ref(ok)=true] | lit("xml")[px::ref(ok)=true] |
				char_
			)
		);

		return ok;
	}


	//////////////////////////////////////////////////////////////////////////
	void Service::processOne(utils::Variant hostId, utils::Variant pageId, utils::Variant url)
	{
		boost::chrono::time_point<boost::chrono::system_clock> start = 
			boost::chrono::system_clock::now();

		http::client::Response resp;
		boost::system::error_code ec = _htc.get(resp, url.as<std::string>().data());

		boost::chrono::milliseconds getTime = boost::chrono::duration_cast<boost::chrono::milliseconds>(boost::chrono::system_clock::now() - start);

		if(ec)
		{
			WLOG("get: "<<ec<<", ["<<url.as<std::string>()<<"]");
			
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

			WLOG("read headers: "<<ec<<", ["<<url.as<std::string>()<<"]");
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
			WLOG("too big: "<<length.value()<<", ["<<url.as<std::string>()<<"]");
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
			WLOG("bad ct: "<<(contentType?std::string(contentType->begin(), contentType->end()):std::string("absent"))<<", ["<<url.as<std::string>()<<"]");
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

			WLOG("read body: "<<ec<<", ["<<url.as<std::string>()<<"]");
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
		std::deque<Url> urls;
		parse(resp.body(), url.as<std::string>(), urls);

		//store urls
		pgc::Connection c = _db.allocConnection();


		pgc::Result res = c.query("UPDATE host SET address=$2 WHERE id=$1", 
			utils::MVA(hostId, 
				resp.channel().endpointRemote().address().to_string(ec)));
		if(pgc::ersError == res.status()) TLOG(res.errorMsg());

		//c.query("BEGIN").wait();

		BOOST_FOREACH(Url&u, urls)
		{
			if(!u._isValid || (u._scheme!="http" && u._scheme!="https"))
			{
				continue;
			}

			std::string url = u.string();

			res = c.query("SELECT id FROM host WHERE name=$1", u._host);
			if(pgc::ersError == res.status()) TLOG(res.errorMsg());
			if(!res.rows())
			{
				res = c.query("INSERT INTO host (name, atime) VALUES ($1, CURRENT_TIMESTAMP)", u._host);
				if(pgc::ersError == res.status()) TLOG(res.errorMsg()<<", "<<u._host);
				if(pgc::ersError == res.status())
				{
					continue;
				}
				res = c.query("SELECT id FROM host WHERE name=$1", u._host);
				if(pgc::ersError == res.status()) TLOG(res.errorMsg());
			}
			utils::Variant hostId2;
			res.fetch(hostId2, 0,0);

			res = c.query("SELECT id, count FROM page WHERE url=$1", url);
			if(pgc::ersError == res.status()) TLOG(res.errorMsg());
			if(!res.rows())
			{
				res = c.query("UPDATE host SET new_pages_count=new_pages_count+1 WHERE id=$1", hostId2);
				if(pgc::ersError == res.status()) TLOG(res.errorMsg());

				res = c.query("INSERT INTO page (host_id, url, count) VALUES ($1, $2, 1)", utils::MVA(hostId2, url));
				if(pgc::ersError == res.status()) TLOG(res.errorMsg()<<", "<<url);
				if(pgc::ersError == res.status())
				{
					continue;
				}

				res = c.query("SELECT currval('page_id_seq'::regclass) AS id");
				if(pgc::ersError == res.status()) TLOG(res.errorMsg()<<", "<<url);
				if(pgc::ersError == res.status())
				{
					continue;
				}
				utils::Variant pageId2;
				res.fetch(pageId2, 0,0);

				res = c.query("INSERT INTO reference (from_id, to_id) VALUES ($1, $2)", utils::MVA(pageId, pageId2));
				if(pgc::ersError == res.status()) TLOG(res.errorMsg()<<", "<<url);
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
				if(pgc::ersError == res.status()) TLOG(res.errorMsg()<<", "<<url);
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

		TLOG("processed: ("<<_numWorkers<<") "<<url.as<std::string>());


		//res = c.query("COMMIT");
		//if(pgc::ersError == res.status()) TLOG(res.errorMsg());

		{
			async::Mutex::ScopedLock sl(_mtxWorkers);
			_numWorkers--;
		}
		_evtWorkerDone.set();
	}

	//////////////////////////////////////////////////////////////////////////
	namespace 
	{
		struct Handler
		{
			std::deque<Url> &_urls;

			Handler(std::deque<Url> &urls)
				: _urls(urls)
			{

			}
			template <typename F, typename Attribute, typename Context>
			void operator()(F const& f, Attribute& attr, Context& context) const
			{
				_urls.push_back(Url(f));
			}
		};

	}
	//////////////////////////////////////////////////////////////////////////
	void Service::parse(http::InputMessage::Segment text, const std::string &baseUrlString, std::deque<Url> &urls)
	{
		//искать <a href="...
		namespace qi = boost::spirit::qi;
		using namespace qi;
		namespace px = boost::phoenix;

		Handler handler(urls);

		bool b = qi::parse(text.begin(), text.end(),
			*(
				(lit("<a") >> *(char_ - char_("h>")) >> "href=" >> char_("'\"") >> raw[+(char_ - char_("'\""))][handler]) |
				char_
			)
		);

		//применить базу
		Url base(baseUrlString);
		for(size_t i(0); i<urls.size(); i++)
		{
			urls[i].combine(base);
		}
	}


}
