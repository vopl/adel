#include "pch.hpp"
#include "scom/impl/service.hpp"
#include "scom/impl/workerRaii.hpp"
#include "scom/log.hpp"

#include <boost/foreach.hpp>

#define IF_PGRES_ERROR(action, ...) {pgc::Result r = __VA_ARGS__; if(pgc::ersError == r.status()) {TLOG(r.errorMsg()<<" ("<<__LINE__<<")");action;}}

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

		runWorker(&Service::workerMain);
		runWorker(&Service::workerInstancesDeleteOld, 1000);
		runWorker(&Service::workerPageRestatusPend, 1000);
		runWorker(&Service::workerHostDeleteOld, 1000);
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
		pgc::Connection c;
		pgc::Result res;

		c = _db.allocConnection();

		res = c.query(""
			"INSERT INTO instance "
			"(password, stage, is_started, ctime, atime, dtime) "
			"VALUES "
			"($1, 0, false, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP+INTERVAL '1 month') "
			"RETURNS id", password);
		IF_PGRES_ERROR(return ee_internalError, res);

		utils::Variant id;
		res.fetch(id, 0, 0);

		auth._secret = password;
		auth._id = id;

		_evtIface.set(true);

		return ee_ok;
	}

	///////////////////////////////////////////////////////////////////
	EError Service::ping(
		Status &status,
		const Auth &auth)
	{
		pgc::Connection c;
		pgc::Result res;

		c = _db.allocConnection();

		IF_PGRES_ERROR(return ee_internalError, c.query("BEGIN"));

		res = c.query(""
			"SELECT stage, is_started, dtime FROM instance "
			"WHERE id=$1 AND password=$2 "
			"FOR UPDATE", utils::MVA(auth._id, auth._secret));
		IF_PGRES_ERROR(return ee_internalError, res);

		if(!res.rows())
		{
			IF_PGRES_ERROR(return ee_internalError, c.query("ROLLBACK"));
			return ee_badId;
		}

		IF_PGRES_ERROR(return ee_internalError, c.query("UPDATE instance SET atime=CURRENT_TIMESTAMP WHERE id=$1", utils::Variant(auth._id)));
		IF_PGRES_ERROR(return ee_internalError, c.query("COMMIT"));

		utils::Variant row;
		res.fetchRowList(row, 0);

		status._stage		= (Status::EStage)(int)row[0];
		status._isStarted	= row[1];
		status._destroyTime	= row[2];

		_evtIface.set(true);

		return ee_ok;
	}

	///////////////////////////////////////////////////////////////////
	EError Service::setup(
		const Auth &auth,
		const std::vector<PageRule> &srcRules,
		const std::vector<PageRule> &dstRules)
	{
		pgc::Connection c;
		pgc::Result res;

		c = _db.allocConnection();

		IF_PGRES_ERROR(return ee_internalError, c.query("BEGIN"));

		res = c.query(""
			"SELECT stage FROM instance "
			"WHERE id=$1 AND password=$2 "
			"FOR UPDATE", utils::MVA(auth._id, auth._secret));
		IF_PGRES_ERROR(return ee_internalError, res);

		if(!res.rows())
		{
			IF_PGRES_ERROR(return ee_internalError, c.query("ROLLBACK"));
			return ee_badId;
		}

		utils::Variant rowInstance;
		res.fetchRowList(rowInstance, 0);
		Status::EStage stage = (Status::EStage)rowInstance[0].to<int>();
		if(Status::es_init != stage)
		{
			IF_PGRES_ERROR(return ee_internalError, c.query("ROLLBACK"));
			return ee_badStage;
		}

		BOOST_FOREACH(const PageRule &pr, srcRules)
		{
			IF_PGRES_ERROR(
				return ee_internalError,
				c.query("INSERT INTO page_rule SET "
						"instance_id=$1 "
						"is_src=true "
						"base_uri=$2 "
						"kind_and_access=$3 "
						"kind_and_access_min=$4 "
						"kind_and_access_max=$5 "
						"max_amount=$6",
					utils::MVA(
						auth._id,
						pr._baseUri,
						pr._kindAndAccess,
						pr._kindAndAccessMin,
						pr._kindAndAccessMax,
						pr._maxAmount)
				)
			);
		}

		BOOST_FOREACH(const PageRule &pr, dstRules)
		{
			IF_PGRES_ERROR(
				return ee_internalError,
				c.query("INSERT INTO page_rule SET "
						"instance_id=$1 "
						"is_src=false "
						"base_uri=$2 "
						"kind_and_access=$3 "
						"kind_and_access_min=$4 "
						"kind_and_access_max=$5 "
						"max_amount=$6",
					utils::MVA(
						auth._id,
						pr._baseUri,
						pr._kindAndAccess,
						pr._kindAndAccessMin,
						pr._kindAndAccessMax,
						pr._maxAmount)
				)
			);
		}

		IF_PGRES_ERROR(return ee_internalError, c.query("UPDATE instance SET atime=CURRENT_TIMESTAMP WHERE id=$1", utils::Variant(auth._id)));
		IF_PGRES_ERROR(return ee_internalError, c.query("COMMIT"));

		_evtIface.set(true);

		return ee_ok;
	}

	///////////////////////////////////////////////////////////////////
	EError Service::start(
		const Auth &auth)
	{
		pgc::Connection c;
		pgc::Result res;

		c = _db.allocConnection();

		IF_PGRES_ERROR(return ee_internalError, c.query("BEGIN"));

		res = c.query(""
			"SELECT stage FROM instance "
			"WHERE id=$1 AND password=$2 "
			"FOR UPDATE", utils::MVA(auth._id, auth._secret));
		IF_PGRES_ERROR(return ee_internalError, res);

		if(!res.rows())
		{
			IF_PGRES_ERROR(return ee_internalError, c.query("ROLLBACK"));
			return ee_badId;
		}

		utils::Variant rowInstance;
		res.fetchRowList(rowInstance, 0);

		Status::EStage stage = (Status::EStage)rowInstance[0].to<int>();
		if(Status::es_init != stage && Status::es_load != stage)
		{
			IF_PGRES_ERROR(return ee_internalError, c.query("ROLLBACK"));
			return ee_badStage;
		}

		IF_PGRES_ERROR(
			return ee_internalError,
			c.query("UPDATE instance SET atime=CURRENT_TIMESTAMP, stage=$2, is_started=$3 WHERE id=$1", utils::MVA(auth._id, Status::es_load, true)));
		IF_PGRES_ERROR(return ee_internalError, c.query("COMMIT"));

		_evtIface.set(true);

		return ee_ok;
	}

	///////////////////////////////////////////////////////////////////
	EError Service::stop(
		const Auth &auth)
	{
		pgc::Connection c;
		pgc::Result res;

		c = _db.allocConnection();

		IF_PGRES_ERROR(return ee_internalError, c.query("BEGIN"));

		res = c.query(""
			"SELECT stage FROM instance "
			"WHERE id=$1 AND password=$2 "
			"FOR UPDATE", utils::MVA(auth._id, auth._secret));
		IF_PGRES_ERROR(return ee_internalError, res);

		if(!res.rows())
		{
			IF_PGRES_ERROR(return ee_internalError, c.query("ROLLBACK"));
			return ee_badId;
		}

		utils::Variant rowInstance;
		res.fetchRowList(rowInstance, 0);

		Status::EStage stage = (Status::EStage)rowInstance[0].to<int>();
		if(Status::es_load != stage)
		{
			IF_PGRES_ERROR(return ee_internalError, c.query("ROLLBACK"));
			return ee_badStage;
		}

		IF_PGRES_ERROR(
			return ee_internalError,
			c.query("UPDATE instance SET atime=CURRENT_TIMESTAMP, is_started=$2 WHERE id=$1", utils::MVA(auth._id, false)));
		IF_PGRES_ERROR(return ee_internalError, c.query("COMMIT"));

		_evtIface.set(true);

		return ee_ok;
	}

	////////////////////////////////////////////////////////////////////////
	void Service::runWorker(TWorker worker, size_t idleTimeout)
	{
		async::Mutex::ScopedLock sl(_mtxWorkers);
		_numWorkers++;
		async::spawn(boost::bind(&Service::workerWrapper, this, worker, idleTimeout));
	}

	////////////////////////////////////////////////////////////////////////
	void Service::workerWrapper(TWorker worker, size_t idleTimeout)
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

			if(!(this->*worker)())
			{
				async::timeout(idleTimeout).wait();
			}
		}
	}

	///////////////////////////////////////////////////////////////////
	bool Service::workerMain()
	{
		_evtIface.wait();
		return true;
	}

	///////////////////////////////////////////////////////////////////
	bool Service::workerInstancesDeleteOld()
	{
		pgc::Connection c;
		pgc::Result res;

		c = _db.allocConnection();

		res = c.query("DELETE FROM instance WHERE dtime <= CURRENT_TIMESTAMP");
		IF_PGRES_ERROR(return false, res);

		return false;
	}

	///////////////////////////////////////////////////////////////////
	bool Service::workerPageRestatusPend()
	{
		return false;
	}

	///////////////////////////////////////////////////////////////////
	bool Service::workerHostDeleteOld()
	{
		return false;
	}


}}
