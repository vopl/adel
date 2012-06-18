#include "pch.hpp"
#include "scom/impl/service.hpp"
#include "scom/impl/workerRaii.hpp"
#include "scom/log.hpp"

namespace scom { namespace impl
{
	///////////////////////////////////////////////////////////////////
	utils::OptionsPtr Service::prepareOptions(const char *prefix)
	{
		utils::OptionsPtr options(new utils::Options(prefix));

		options->addOption(
			"pgc.connectionString",
			boost::program_options::value<std::string>()->default_value("host=localhost port=5432 dbname=scom user=scom password=scom"),
			"connection string for postgres database");

		options->addOption(
			"pgc.maxConnections",
			boost::program_options::value<size_t>()->default_value(20),
			"maximum number of connections to postgres database");

		options->addOption(
			"net.concurrency",
			boost::program_options::value<size_t>()->default_value(50),
			"maximum number of parallel network connections");

		options->addOption(
			"net.defaultHostDelay",
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
	Service::Service(utils::OptionsPtr optionsPtr)
		: _isWork(false)
		, _evtWorkerDone(true)
		, _evtIface(true)
		, _numWorkers(0)
	{
		utils::Options &o = *optionsPtr;
		_pgc_connectionString = o["pgc.connectionString"].as<std::string>();
		_pgc_maxConnections = o["pgc.maxConnections"].as<size_t>();
	}

	///////////////////////////////////////////////////////////////////
	Service::~Service()
	{
	}

	///////////////////////////////////////////////////////////////////
	void Service::start()
	{
		async::Mutex::ScopedLock sl(_mtx);
		if(_isWork)
		{
			return;
		}
		_isWork = true;

		_db = pgc::Db(
			_pgc_connectionString.data(),
			_pgc_maxConnections);

		async::Mutex::ScopedLock sl2(_mtxWorkers);
		_numWorkers++;
		async::spawn(boost::bind(&Service::mainWorker, this));
		_evtIface.set(true);
	}

	///////////////////////////////////////////////////////////////////
	void Service::stop()
	{
		{
			async::Mutex::ScopedLock sl(_mtx);
			if(!_isWork)
			{
				return;
			}
			_isWork = false;
		}

		_evtIface.set(true);
		for(;;)
		{
			_evtIface.set(true);
			{
				async::Mutex::ScopedLock sl(_mtxWorkers);
				if(!_numWorkers)
				{
					break;
				}
			}
			_evtIface.set(true);

			//TODO: наличие события, да и вообще фибера не тормозит останов async, это может привести к ситуации разрушения работающих объектов. Надо пересмотреть останов async
			_evtWorkerDone.wait();
			//async::timeout(100).wait();
		}
		_db.reset();
	}

	///////////////////////////////////////////////////////////////////
	EError Service::create(
		Auth &auth,
		std::string password)
	{
		//insert
		assert(0);
		_evtIface.set(true);
	}

	///////////////////////////////////////////////////////////////////
	EError Service::ping(
		Status &status,
		const Auth &auth)
	{
		assert(0);
		_evtIface.set(true);
	}

	///////////////////////////////////////////////////////////////////
	EError Service::setup(
		const Auth &auth,
		const std::vector<PageRule> &srcRules,
		const std::vector<PageRule> &dstRules)
	{
		assert(0);
		_evtIface.set(true);
	}

	///////////////////////////////////////////////////////////////////
	EError Service::start(
		const Auth &auth)
	{
		assert(0);
	}

	///////////////////////////////////////////////////////////////////
	EError Service::stop(
		const Auth &auth)
	{
		assert(0);
		_evtIface.set(true);
	}

	///////////////////////////////////////////////////////////////////
	void Service::mainWorker()
	{
		WorkerRaii raii(_mtxWorkers, _numWorkers, _evtWorkerDone);

		for(;;)
		{
			//контроль окончания
			{
				async::Mutex::ScopedLock sl(_mtx);
				if(!_isWork)
				{
					break;
				}
			}

			bool workWas = false;

#define IF_PGRES_ERROR(action, ...) {pgc::Result r = __VA_ARGS__; if(pgc::ersError == r.status()) {TLOG(r.errorMsg()<<" ("<<__LINE__<<")");action;}}

			//удалить просроченные

			pgc::Connection c = _db.allocConnection();
			IF_PGRES_ERROR(continue, c.query("BEGIN"));

			pgc::Result res = c.query("SELECT id FROM instance WHERE dtime <= CURRENT_TIMESTAMP FOR UPDATE LIMIT 10");
			IF_PGRES_ERROR(continue, res);

			utils::Variant v;
			for(size_t i(0); i<res.rows(); i++)
			{
				res.fetch(v, 0, i);
				IF_PGRES_ERROR(continue, c.query("DELETE FROM page WHERE instance_id=$1", v));
				IF_PGRES_ERROR(continue, c.query("DELETE FROM page_rule WHERE instance_id=$1", v));
				IF_PGRES_ERROR(continue, c.query("DELETE FROM instance WHERE id=$1", v));
				workWas = true;
			}
			IF_PGRES_ERROR(continue, c.query("COMMIT"));
			c.reset();


			//запускать готовые
			c = _db.allocConnection();

			//если работы небыло ждать событие интерфейса
			if(!workWas)
			{
				_evtIface.wait();
			}
		}
	}
}}
