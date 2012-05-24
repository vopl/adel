#include "pch.hpp"
#include "spider/service.hpp"
#include "spider/log.hpp"
#include <boost/bind.hpp>

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
			boost::program_options::value<size_t>()->default_value(5),
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

			//выбрать хост-урл
			pgc::Connection c = _db.allocConnection();

			c.query("BEGIN");

			//выбрать самый старый хост у которого есть неотработанные страницы
			pgc::Result res = c.query("SELECT "
				"h.id "
				"FROM host AS h LEFT JOIN page AS p ON (h.id=p.host_id) "
				"WHERE p.id IS NOT NULL AND p.status IS NULL AND CURRENT_TIMESTAMP>(h.atime+$1::interval) "
				"GROUP BY h.id "
				"ORDER BY COUNT(p) DESC "
				"LIMIT 10 ", utils::Variant(boost::posix_time::seconds((long)_net_hostDelay))).data();


			utils::Variant hostId;
			utils::Variant pageId;
			utils::Variant pageUrl;
			for(size_t hrow(0); hrow < res.rows(); hrow++)
			{
				res.fetch(hostId, 0, hrow);

				res = c.query("SELECT "
					"p.id, p.url "
					"FROM host AS h INNER JOIN page AS p ON (h.id=p.host_id) "
					"WHERE h.id=$1 AND p.id IS NOT NULL AND p.status IS NULL AND CURRENT_TIMESTAMP>(p.atime+$2::interval)"
					"LIMIT 1 "
					"FOR UPDATE OF h, p", utils::MVA(hostId, utils::Variant(boost::posix_time::seconds((long)_net_hostDelay)))).data();

				if(res.rows())
				{
					res.fetch(pageId, 0, 0);
					res.fetch(pageUrl, 1, 0);

					res = c.query("UPDATE host SET atime=CURRENT_TIMESTAMP WHERE id=$1", hostId).data();
					res = c.query("UPDATE page SET atime=CURRENT_TIMESTAMP WHERE id=$1", pageId).data();
					break;
				}
			}

			if(pageUrl.isNull())
			{
				//нет хостов, подождать немного
				c.query("ROLLBACK");
				async::timeout(100).wait();
				continue;
			}

			res = c.query("COMMIT").data();

			async::spawn(boost::bind(&Service::processOne, this, pageId, pageUrl));
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void Service::processOne(utils::Variant id, utils::Variant url)
	{
		TLOG("fetch: "<<url);

		http::client::Response resp;
		boost::system::error_code ec = _htc.get(resp, url.as<std::string>().data());
		if(ec)
		{
			WLOG("http get failed: "<<ec<<", ["<<url<<"]");
			return;
		}
		ec = resp.readBody();
		if(ec)
		{

			WLOG("http get failed: "<<ec<<", ["<<url<<"]");
			return;
		}

		//TLOG(std::string(resp.firstLine().begin(), resp.firstLine().end()));

		//parse

		//store urls
	}


}
