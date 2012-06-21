#include "pch.hpp"
#include "scom/impl/service.hpp"
#include "scom/impl/workerRaii.hpp"
#include "scom/log.hpp"
#include "htmlcxx/html/Uri.h"

#include <boost/foreach.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/regex.hpp>

#define IF_PGRES_ERROR(action, ...) {pgc::Result r = __VA_ARGS__; if(pgc::ersError == r.status()) {ELOG(r.errorMsg()<<" ("<<__LINE__<<")");action;}}

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
		, _pgc_maxConnections(10)
		, _net_defaultHostDelay(boost::posix_time::seconds(10))
		, _net_concurrency(1000)
		, _numWorkers(0)
		, _pageRestatusPentTimeout(boost::posix_time::minutes(60))
		, _activeHostTimeout(boost::posix_time::minutes(10))
	{
		utils::Options &o = *optionsPtr;
		_pgc_connectionString = o["pgc.connectionString"].as<std::string>();
		_pgc_maxConnections = o["pgc.maxConnections"].as<size_t>();
		_net_defaultHostDelay = boost::posix_time::seconds((long)o["net.defaultHostDelay"].as<size_t>());
		_net_concurrency = o["net.concurrency"].as<size_t>();
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

		_prac.start(boost::posix_time::minutes(10));
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

		_prac.stop();

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
			"RETURNING id", password);
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
		const std::vector<PageRule> &rules)
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

		BOOST_FOREACH(const PageRule &pr, rules)
		{
			EError validator = ee_ok;
			switch(pr._kindAndAccess & PageRule::ek_mask)
			{
			case PageRule::ek_regex:
				try
				{
					boost::regex test(pr._value);
				}
				catch(...)
				{
					validator = ee_badRegex;
				}
				break;
			case PageRule::ek_domain:
				{
					htmlcxx::Uri test("http://"+pr._value+"/");
					if(!test.isOk() || test.hostname() != pr._value)
					{
						validator = ee_badDomain;
					}
				}
				break;
			case PageRule::ek_path:
				{
					htmlcxx::Uri test(pr._value);
					if(
						!test.isOk() ||
						test.scheme().empty() ||
						(test.scheme()!="http" && test.scheme()!="https") ||
						test.hostname().empty() ||
						test.path().empty() ||
						!test.fragment().empty())
					{
						validator = ee_badUri;
					}
				}
				if(pr._kindAndAccess & (PageRule::ea_useWords | PageRule::ea_useLinks))
				{
					if(!insertPageIfAbsent(c, auth._id, pr._value))
					{
						return ee_internalError;
					}
				}
				break;
			case PageRule::ek_reference:
				{
					htmlcxx::Uri test(pr._value);
					if(
						!test.isOk() ||
						test.scheme().empty() ||
						(test.scheme()!="http" && test.scheme()!="https") ||
						test.hostname().empty() ||
						test.path().empty() ||
						!test.fragment().empty())
					{
						validator = ee_badUri;
					}
				}
				if(pr._kindAndAccess & (PageRule::ea_useWords | PageRule::ea_useLinks))
				{
					if(!insertPageIfAbsent(c, auth._id, pr._value))
					{
						return ee_internalError;
					}
				}
				break;
			}
			
			
			
			if(validator)
			{
				IF_PGRES_ERROR(return ee_internalError, c.query("ROLLBACK"));
				return validator;
			}
			
			IF_PGRES_ERROR(
				return ee_internalError,
				c.query("INSERT INTO page_rule ( "
						"instance_id,"
						"value,"
						"kind_and_access,"
						"kind_and_access_min,"
						"kind_and_access_max"
						") VALUES ("
						"$1,$2,$3,$4,$5)",
					utils::MVA(
						auth._id,
						pr._value,
						pr._kindAndAccess,
						pr._kindAndAccessMin,
						pr._kindAndAccessMax)
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
			
		if(!_prac.update(c, auth._id))
		{	
			return ee_internalError;
		}
		
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

	///////////////////////////////////////////////////////////////////
	EError Service::destroy(
		const Auth &auth)
	{
		pgc::Connection c;
		pgc::Result res;

		c = _db.allocConnection();

		IF_PGRES_ERROR(return ee_internalError, c.query("BEGIN"));

		res = c.query(""
			"DELETE FROM instance "
			"WHERE id=$1 AND password=$2 "
			"RETURNING id", utils::MVA(auth._id, auth._secret));
		IF_PGRES_ERROR(return ee_internalError, res);

		if(!res.rows())
		{
			IF_PGRES_ERROR(return ee_internalError, c.query("ROLLBACK"));
			return ee_badId;
		}

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
		//выбрать  незагруженные страницы джоин инстанс, состояние лоад и запущенный
		//инициировать загрузку этих страниц
		_evtIface.wait();
		return true;
	}

	///////////////////////////////////////////////////////////////////
	bool Service::workerInstancesDeleteOld()
	{
		pgc::Connection c = _db.allocConnection();
		pgc::Result res = c.query("DELETE FROM instance WHERE dtime <= CURRENT_TIMESTAMP");
		IF_PGRES_ERROR(return false, res);

		return false;
	}

	///////////////////////////////////////////////////////////////////
	bool Service::workerPageRestatusPend()
	{
		pgc::Connection c = _db.allocConnection();
		pgc::Result res = c.query("UPDATE page SET http_status=NULL WHERE http_status='pend' AND atime <= CURRENT_TIMESTAMP-$1::INTERVAL", utils::Variant(_pageRestatusPentTimeout));
		IF_PGRES_ERROR(return false, res);

		return false;
	}

	///////////////////////////////////////////////////////////////////
	bool Service::workerHostDeleteOld()
	{
		pgc::Connection c = _db.allocConnection();
		pgc::Result res = c.query("DELETE FROM active_host WHERE atime <= CURRENT_TIMESTAMP-$1::INTERVAL", utils::Variant(_activeHostTimeout));
		IF_PGRES_ERROR(return false, res);

		return false;
	}

	bool Service::insertPageIfAbsent(pgc::Connection c, boost::int64_t instanceId, const std::string &uri)
	{
		pgc::Result res = c.query("SELECT id FROM page WHERE instance_id=$1 AND uri=$2", utils::MVA(instanceId, uri));
		IF_PGRES_ERROR(return false, res);
		
		if(res.rows())
		{
			return true;
		}
		
		res = c.query("INSERT INTO page (instance_id, uri) VALUES ($1,$2)", utils::MVA(instanceId, uri));
		IF_PGRES_ERROR(return false, res);
		return true;
	}
			


}}
