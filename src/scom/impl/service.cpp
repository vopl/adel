#include "pch.hpp"
#include "scom/impl/service.hpp"
#include "scom/impl/workerRaii.hpp"
#include "scom/log.hpp"

#include "htmlcxx/html/ParserDom.h"
#include "htmlcxx/html/CharsetConverter.h"
#include "htmlcxx/html/utils.h"

#include "utils/htmlEntities.hpp"

#include "scom/impl/reportGenerator.hpp"

#include <boost/foreach.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/regex.hpp>
#include <boost/chrono.hpp>
#include <boost/algorithm/string.hpp>

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
			"net.hostAccessDelay",
			boost::program_options::value<size_t>()->default_value(1),
			"delay between requests to one host, seconds");






		options->addOption(
			"workerIdleTimeoutMain",
			boost::program_options::value<size_t>()->default_value(1),
			"main worker idle timeout, seconds");

		options->addOption(
			"workerIdleTimeoutCleanupper",
			boost::program_options::value<size_t>()->default_value(10),
			"cleanup worker idle timeout, seconds");

		options->addOption(
			"ruleApplyerCacheTimeout",
			boost::program_options::value<size_t>()->default_value(10),
			"page rule applyer cache lifetime, seconds");


		options->addOption(
			"workerIdleTimeoutMerger",
			boost::program_options::value<size_t>()->default_value(1),
			"content merger worker idle timeout, seconds");

		options->addOption(
			"pagesToLoadGranula",
			boost::program_options::value<size_t>()->default_value(50),
			"amount of pages, selected for load from database per one worker iteration");

		options->addOption(
			"httpBodyMaxSize",
			boost::program_options::value<size_t>()->default_value(1024*1024),
			"maximum size for http body during load page from network, bytes");

		options->addOption(
			"maxWorkers",
			boost::program_options::value<size_t>()->default_value(200),
			"maximum number of active workers");

		options->addOption(
			"deadWorkerTimeout",
			boost::program_options::value<size_t>()->default_value(60*60),
			"dead worker timeout, seconds");

		options->addOption(
			"activeHostTimeout",
			boost::program_options::value<size_t>()->default_value(10),
			"delay betwen http requests for same host, seconds");


		options->addOption(
			"maxPagesPerRule",
			boost::program_options::value<size_t>()->default_value(999999),
			"maximum number of pages per one rule");
		options->addOption(
			"hunspell.affpath",
			boost::program_options::value<std::string>()->default_value("../spell/en_US.aff"),
			"hunspell aff path");

		options->addOption(
			"hunspell.dicpath",
			boost::program_options::value<std::string>()->default_value("../spell/en_US.dic"),
			"hunspell dic path");

		std::vector<std::string> extraDics;
		extraDics.push_back("../spell/ru_RU.dic");
		options->addOption(
			"hunspell.extradicpath",
			boost::program_options::value<std::vector<std::string> >()->default_value(extraDics, "../spell/ru_RU.dic"),
			"hunspell extra dic paths");

		options->addOption(
			"tmp-dir",
			boost::program_options::value<std::string>()->default_value("../tmp"),
			"temporary directory for report database files");

		options->addOptions(*http::Client::prepareOptions(prefix));

		return options;

	}

	///////////////////////////////////////////////////////////////////
	Service::Service(utils::OptionsPtr optionsPtr)
		: _isWork(false)
		, _evtWorkerDone(true)
		, _evtIface(true)
		, _pgc_maxConnections(10)
		, _net_hostAccessDelay(boost::posix_time::seconds(10))
		, _workerIdleTimeoutMerger(boost::posix_time::seconds(10))
		, _workerIdleTimeoutCleanupper(boost::posix_time::seconds(10))
		, _workerIdleTimeoutMain(boost::posix_time::seconds(1))
		, _ruleApplyerCacheTimeout(boost::posix_time::seconds(10))
		, _numWorkers(0)
		, _pagesToLoadGranula(50)
		, _maxHttpBodySize(1024*1024)
		, _maxWorkers(200)
		, _maxPagesPerRule(1000)
		, _deadWorkerTimeout(boost::posix_time::minutes(60))
		, _activeHostDeleteTimeout(boost::posix_time::minutes(10))
		, _htc(optionsPtr)
		, _hunspell(NULL)
	{
		utils::Options &o = *optionsPtr;
		_pgc_connectionString = o["pgc.connectionString"].as<std::string>();
		_pgc_maxConnections = o["pgc.maxConnections"].as<size_t>();
		_net_hostAccessDelay = boost::posix_time::seconds((long)o["net.hostAccessDelay"].as<size_t>());

		_workerIdleTimeoutMain = boost::posix_time::seconds((long)o["workerIdleTimeoutMain"].as<size_t>());
		_workerIdleTimeoutCleanupper = boost::posix_time::seconds((long)o["workerIdleTimeoutCleanupper"].as<size_t>());
		_ruleApplyerCacheTimeout = boost::posix_time::seconds((long)o["ruleApplyerCacheTimeout"].as<size_t>());

		_workerIdleTimeoutMerger = boost::posix_time::seconds((long)o["workerIdleTimeoutMerger"].as<size_t>());

		_pagesToLoadGranula = o["pagesToLoadGranula"].as<size_t>();
		_maxHttpBodySize = o["httpBodyMaxSize"].as<size_t>();
		_maxWorkers = o["maxWorkers"].as<size_t>();
		_maxPagesPerRule = o["maxPagesPerRule"].as<size_t>();
		_deadWorkerTimeout = boost::posix_time::seconds((long)o["deadWorkerTimeout"].as<size_t>());
		_activeHostDeleteTimeout = boost::posix_time::seconds((long)o["activeHostTimeout"].as<size_t>());

		_hunspell = new Hunspell(
			o["hunspell.affpath"].as<std::string>().c_str(),
			o["hunspell.dicpath"].as<std::string>().c_str());

		BOOST_FOREACH(const std::string &dp, o["hunspell.extradicpath"].as<std::vector<std::string> >())
		{
			_hunspell->add_dic(dp.c_str(), NULL);
		}


		_tmpDir = o["tmp-dir"].as<std::string>();

	}

	///////////////////////////////////////////////////////////////////
	Service::~Service()
	{
		if(_hunspell)
		{
			delete _hunspell;
		}
	}

	///////////////////////////////////////////////////////////////////
	void Service::start()
	{
		async::Mutex::ScopedLock sl(_mtx);
		if(_isWork)
		{
			return;
		}




		_stCreateInsertInstance = pgc::Statement("INSERT INTO instance "
			"(password, stage, is_started, ctime, atime, dtime) "
			"VALUES "
			"($1, 0, false, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP+INTERVAL '1 month') "
			"RETURNING id");
		_stBegin = pgc::Statement("BEGIN");
		_stCommit = pgc::Statement("COMMIT");
		_stRollback = pgc::Statement("ROLLBACK");
		_stPingSelectInstance = pgc::Statement("SELECT i.stage, i.is_started, i.dtime "
			"FROM instance AS i "
			"WHERE i.id=$1 AND i.password=$2");

		_stPingSelectPagesVolume = pgc::Statement("SELECT count(*) FROM page WHERE instance_id=$1 AND (access=2 OR access=4 OR access=6)");
		_stPingSelectPagesProcessed = pgc::Statement("SELECT count(*) FROM page WHERE instance_id=$1 AND (access=2 OR access=4 OR access=6) AND (status IS NOT NULL AND status<>'pend')");

		_stPingUpdateInstance = pgc::Statement("UPDATE instance SET atime=CURRENT_TIMESTAMP WHERE id=$1");
		//_stLockInstance = pgc::Statement("LOCK instance IN EXCLUSIVE MODE");
		//_stLockPageRule = pgc::Statement("LOCK page_rule IN EXCLUSIVE MODE");
		_stLockPage = pgc::Statement("LOCK page IN EXCLUSIVE MODE");
		//_stLockPageRef = pgc::Statement("LOCK page_ref IN EXCLUSIVE MODE");
		//_stLockActiveHost = pgc::Statement("LOCK active_host IN EXCLUSIVE MODE");

		_stSetupSelectInstance = pgc::Statement("SELECT stage FROM instance "
			"WHERE id=$1 AND password=$2");
		_stSetupinsertPageRule = pgc::Statement("INSERT INTO page_rule ( "
			"instance_id,"
			"value,"
			"kind_and_access,"
			"kind_and_access_min,"
			"kind_and_access_max,"
			"amount"
			") VALUES ("
			"$1,$2,$3,$4,$5,$6)");
		_stSetupUpdateInstance = pgc::Statement("UPDATE instance SET atime=CURRENT_TIMESTAMP WHERE id=$1");


		_stStartSelectInstance = pgc::Statement("SELECT stage FROM instance "
			"WHERE id=$1 AND password=$2");
		_stStartUpdateInstance = pgc::Statement("UPDATE instance SET atime=CURRENT_TIMESTAMP, stage=$2, is_started=$3 WHERE id=$1");
		_stStopSelectInstance = pgc::Statement("SELECT stage FROM instance "
			"WHERE id=$1 AND password=$2");
		_stStopUpdateInstance = pgc::Statement("UPDATE instance SET atime=CURRENT_TIMESTAMP, is_started=$2 WHERE id=$1");
		_stDestroyDeleteInstance = pgc::Statement("DELETE FROM instance "
			"WHERE id=$1 AND password=$2 "
			"RETURNING id");
		_stDestroyDeletePageRule = pgc::Statement("DELETE FROM page_rule WHERE instance_id=$1");
		_stDestroyDeletePage = pgc::Statement("DELETE FROM page WHERE instance_id=$1");
		_stMainSelectPage = pgc::Statement("SELECT p.id, ah.id, i.id, p.uri, p.access "
			"FROM page AS p "
			"INNER JOIN instance AS i ON(p.instance_id=i.id) "
			"LEFT JOIN active_host AS ah ON(p.active_host_id=ah.id) "
			"WHERE "
			"	(ah.atime IS NULL OR ah.atime<=CURRENT_TIMESTAMP-$1::INTERVAL) AND "
			"	p.status IS NULL AND "
			"	p.access IN(x'2'::int, x'4'::int, x'6'::int) AND "
			"	i.stage=10 AND i.is_started AND "
			"	1=1 "
			"LIMIT $2 FOR UPDATE OF p");
		_stMainSelectActiveHost = pgc::Statement("SELECT id, atime<=CURRENT_TIMESTAMP-$1::INTERVAL FROM active_host WHERE name=$2 FOR UPDATE");
		_stMainUpdatePageActiveHost = pgc::Statement("UPDATE page SET active_host_id=$2 WHERE id=$1");
		_stMainUpdateActiveHostAtime = pgc::Statement("UPDATE active_host SET atime=CURRENT_TIMESTAMP WHERE id=$1");
		_stMainInsertActiveHost = pgc::Statement("INSERT INTO active_host (name, atime) VALUES ($1, CURRENT_TIMESTAMP) RETURNING id");
		_stMainUpdatePageStatusPend = pgc::Statement("UPDATE page SET status='pend', active_host_id=$2 WHERE id=$1");
		_stInsatanceDeleteOld = pgc::Statement("DELETE FROM instance WHERE dtime <= CURRENT_TIMESTAMP");
		_stPageRestatusPend = pgc::Statement("UPDATE page SET status=NULL WHERE status='pend' AND atime <= CURRENT_TIMESTAMP-$1::INTERVAL");
		_stHostDeleteOld = pgc::Statement("DELETE FROM active_host WHERE atime <= CURRENT_TIMESTAMP-$1::INTERVAL");
		_stPageRuleApplyerSelectPage = pgc::Statement("SELECT p.instance_id FROM page AS p INNER JOIN instance AS i ON (p.instance_id=i.id) WHERE p.access IS NULL AND i.stage=10 AND i.is_started GROUP BY instance_id LIMIT $1");
		_stUpdatePageStatus = pgc::Statement("UPDATE page SET status=$2::character varying WHERE id=$1::bigint");
		_stLoaderUpdatePage = pgc::Statement("UPDATE page SET fetch_order=nextval('page_id_seq'::regclass), status=$2, text=$3, ref_page_ids=$4, http_headers=$5, ip=$6, fetch_time=$7 WHERE id=$1");
		_stLoaderSelectPage = pgc::Statement("SELECT id FROM page WHERE instance_id=$1 AND uri=$2");
		_stLoaderInsertPage = pgc::Statement("INSERT INTO page (instance_id, uri) VALUES ($1,$2) RETURNING id");
		//_stLoaderInsertPageRef = pgc::Statement("INSERT INTO page_ref (src_page_id, dst_page_id) VALUES ($1,$2)");
		_stInsertPageSelectId = pgc::Statement("SELECT id FROM page WHERE instance_id=$1 AND uri=$2");
		_stInsertPage = pgc::Statement("INSERT INTO page (instance_id, uri) VALUES ($1,$2)");

		_isWork = true;

		_db = pgc::Db(
			_pgc_connectionString.data(),
			_pgc_maxConnections);

		runWorker(&Service::workerMain, _workerIdleTimeoutMain);
		runWorker(&Service::workerInstancesDeleteOld, _workerIdleTimeoutCleanupper);
		runWorker(&Service::workerPageRestatusPend, _workerIdleTimeoutCleanupper);
		runWorker(&Service::workerHostDeleteOld, _workerIdleTimeoutCleanupper);
		runWorker(&Service::workerPageRuleApplyer, _workerIdleTimeoutCleanupper);
		runWorker(&Service::workerMerger, _workerIdleTimeoutMerger);

		_prac.start(_ruleApplyerCacheTimeout);
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

		res = c.query(
			_stCreateInsertInstance, password);
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

		IF_PGRES_ERROR(return ee_internalError, c.query(_stBegin));

		res = c.query(
			_stPingSelectInstance, utils::MVA(auth._id, auth._secret));
		IF_PGRES_ERROR(return ee_internalError, res);

		if(!res.rows())
		{
			IF_PGRES_ERROR(return ee_internalError, c.query(_stRollback));
			return ee_badId;
		}

		pgc::Result resVolume = c.query(_stPingSelectPagesVolume, utils::Variant(auth._id));
		IF_PGRES_ERROR(return ee_internalError, resVolume);

		pgc::Result resProcessed = c.query(_stPingSelectPagesProcessed, utils::Variant(auth._id));
		IF_PGRES_ERROR(return ee_internalError, resProcessed);


		IF_PGRES_ERROR(return ee_internalError, c.query(_stPingUpdateInstance, utils::Variant(auth._id)));
		IF_PGRES_ERROR(return ee_internalError, c.query(_stCommit));

		utils::Variant row,rowVolume, rowProcessed;
		res.fetchRowList(row, 0);
		resVolume.fetchRowList(rowVolume, 0);
		resProcessed.fetchRowList(rowProcessed, 0);

		status._stage		= (Status::EStage)(int)row[0];
		status._isStarted	= row[1];
		status._destroyTime	= row[2];

		status._workProcessed = rowProcessed[0];
		status._workVolume = rowVolume[0];

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

		IF_PGRES_ERROR(return ee_internalError, c.query(_stBegin));

		//IF_PGRES_ERROR(return ee_internalError, c.query(_stLockInstance));
		//IF_PGRES_ERROR(return ee_internalError, c.query(_stLockPageRule));
		IF_PGRES_ERROR(return ee_internalError, c.query(_stLockPage));

		res = c.query(
			_stSetupSelectInstance, utils::MVA(auth._id, auth._secret));
		IF_PGRES_ERROR(return ee_internalError, res);

		if(!res.rows())
		{
			IF_PGRES_ERROR(return ee_internalError, c.query(_stRollback));
			return ee_badId;
		}

		utils::Variant rowInstance;
		res.fetchRowList(rowInstance, 0);
		Status::EStage stage = (Status::EStage)rowInstance[0].to<int>();
		if(Status::es_init != stage)
		{
			IF_PGRES_ERROR(return ee_internalError, c.query(_stRollback));
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
					if(
						pr._kindAndAccessMin > pr._kindAndAccessMax ||
						pr._kindAndAccessMin < -999999 ||
						pr._kindAndAccessMax > 999999
					)
					{
						validator = ee_badRange;
						break;
					}
					htmlcxx::Uri test("http://"+pr._value+"/");
					if(!test.isOk() || test.hostname() != pr._value)
					{
						validator = ee_badDomain;
					}
				}
				break;
			case PageRule::ek_path:
				{
					if(
						pr._kindAndAccessMin > pr._kindAndAccessMax ||
						pr._kindAndAccessMin < -999999 ||
						pr._kindAndAccessMax > 999999
					)
					{
						validator = ee_badRange;
						break;
					}
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
					if(
						pr._kindAndAccessMin > pr._kindAndAccessMax ||
						pr._kindAndAccessMin < 0 ||
						pr._kindAndAccessMax > 999999
					)
					{
						validator = ee_badRange;
						break;
					}
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
			
			if(!validator)
			{
				// amount in [-1;  _maxPagesPerRule]
				if(pr._amount <= -2)
				{
					validator = ee_badAmount;
				}
				else if(pr._amount > (int)_maxPagesPerRule)
				{
					validator = ee_badAmount;
				}
			}

			
			
			if(validator)
			{
				IF_PGRES_ERROR(return ee_internalError, c.query(_stRollback));
				return validator;
			}
			
			IF_PGRES_ERROR(
				return ee_internalError,
				c.query(_stSetupinsertPageRule, utils::MVA(
						auth._id,
						pr._value,
						pr._kindAndAccess,
						pr._kindAndAccessMin,
						pr._kindAndAccessMax,
						pr._amount)
				)
			);
		}

		IF_PGRES_ERROR(return ee_internalError, c.query(_stSetupUpdateInstance, utils::Variant(auth._id)));
		IF_PGRES_ERROR(return ee_internalError, c.query(_stCommit));

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

		IF_PGRES_ERROR(return ee_internalError, c.query(_stBegin));

		//IF_PGRES_ERROR(return ee_internalError, c.query(_stLockInstance));
		//IF_PGRES_ERROR(return ee_internalError, c.query(_stLockPageRule));
		//IF_PGRES_ERROR(return ee_internalError, c.query(_stLockPage));
		//IF_PGRES_ERROR(return ee_internalError, c.query(_stLockPageRef));

		res = c.query(
			_stStartSelectInstance, utils::MVA(auth._id, auth._secret));
		IF_PGRES_ERROR(return ee_internalError, res);

		if(!res.rows())
		{
			IF_PGRES_ERROR(return ee_internalError, c.query(_stRollback));
			return ee_badId;
		}

		utils::Variant rowInstance;
		res.fetchRowList(rowInstance, 0);

		Status::EStage stage = (Status::EStage)rowInstance[0].to<int>();
		if(Status::es_init != stage && Status::es_load != stage)
		{
			IF_PGRES_ERROR(return ee_internalError, c.query(_stRollback));
			return ee_badStage;
		}

		IF_PGRES_ERROR(
			return ee_internalError,
			c.query(_stStartUpdateInstance, utils::MVA(auth._id, Status::es_load, true)));

		if(!_prac.update(c, auth._id))
		{	
			return ee_internalError;
		}
		
		IF_PGRES_ERROR(return ee_internalError, c.query(_stCommit));

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

		IF_PGRES_ERROR(return ee_internalError, c.query(_stBegin));

		//IF_PGRES_ERROR(return ee_internalError, c.query(_stLockInstance));

		res = c.query(
			_stStopSelectInstance, utils::MVA(auth._id, auth._secret));
		IF_PGRES_ERROR(return ee_internalError, res);

		if(!res.rows())
		{
			IF_PGRES_ERROR(return ee_internalError, c.query(_stRollback));
			return ee_badId;
		}

		utils::Variant rowInstance;
		res.fetchRowList(rowInstance, 0);

		Status::EStage stage = (Status::EStage)rowInstance[0].to<int>();
		if(Status::es_load != stage)
		{
			IF_PGRES_ERROR(return ee_internalError, c.query(_stRollback));
			return ee_badStage;
		}

		IF_PGRES_ERROR(
			return ee_internalError,
			c.query(_stStopUpdateInstance, utils::MVA(auth._id, false)));
		IF_PGRES_ERROR(return ee_internalError, c.query(_stCommit));

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

		IF_PGRES_ERROR(return ee_internalError, c.query(_stBegin));

		//IF_PGRES_ERROR(return ee_internalError, c.query(_stLockInstance));
		//IF_PGRES_ERROR(return ee_internalError, c.query(_stLockPageRule));
		//IF_PGRES_ERROR(return ee_internalError, c.query(_stLockPage));
		//IF_PGRES_ERROR(return ee_internalError, c.query(_stLockPageRef));

		res = c.query(
			_stDestroyDeleteInstance, utils::MVA(auth._id, auth._secret));
		IF_PGRES_ERROR(return ee_internalError, res);

		if(!res.rows())
		{
			IF_PGRES_ERROR(return ee_internalError, c.query(_stRollback));
			return ee_badId;
		}

		res = c.query(
			_stDestroyDeletePageRule, utils::Variant(auth._id));
		IF_PGRES_ERROR(return ee_internalError, res);


		res = c.query(
			_stDestroyDeletePage, utils::Variant(auth._id));
		IF_PGRES_ERROR(return ee_internalError, res);

		IF_PGRES_ERROR(return ee_internalError, c.query(_stCommit));

		_evtIface.set(true);

		return ee_ok;
	}

	////////////////////////////////////////////////////////////////////////
	void Service::runWorker(TWorker worker, utils::Variant::TimeDuration idleTimeout)
	{
		async::Mutex::ScopedLock sl(_mtxWorkers);
		_numWorkers++;
		async::spawn(boost::bind(&Service::workerWrapper, this, worker, idleTimeout));
	}

	////////////////////////////////////////////////////////////////////////
	void Service::workerWrapper(TWorker worker, utils::Variant::TimeDuration idleTimeout)
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
				async::timeout(idleTimeout.ticks()*1000/idleTimeout.ticks_per_second()).wait();
			}
		}
	}

	///////////////////////////////////////////////////////////////////
	bool Service::workerMain()
	{
		{
			async::Mutex::ScopedLock sl(_mtxWorkers);
			if(_numWorkers >= _maxWorkers)
			{
				return false;
			}
		}

		//выбрать  незагруженные страницы джоин инстанс, состояние лоад и запущенный
		//инициировать загрузку этих страниц

		pgc::Connection c = _db.allocConnection();

		IF_PGRES_ERROR(return true, c.query(_stBegin));

		//IF_PGRES_ERROR(return true, c.query(_stLockInstance));
		//IF_PGRES_ERROR(return true, c.query(_stLockPage));
		//IF_PGRES_ERROR(return true, c.query(_stLockActiveHost));

		pgc::Result res;

		res = c.query(_stMainSelectPage, utils::MVA(_net_hostAccessDelay, _pagesToLoadGranula));
		IF_PGRES_ERROR(return true, res);

		if(!res.rows())
		{
			//нет нагрузки
			IF_PGRES_ERROR(return true, c.query(_stRollback));
			return false;
		}

		//список активных хостов для данной транзакции - каждый хост должен использоваться не более 1 раза
		std::set<utils::Variant> usedActiveHosts;

		for(size_t i(0); i<res.rows(); i++)
		{

			utils::Variant pageId, hostId, instanceId, uri, access;
			bool b = res.fetch(pageId, 0, i);
			assert(b);
			b = res.fetch(hostId, 1, i);
			assert(b);
			b = res.fetch(instanceId, 2, i);
			assert(b);
			b = res.fetch(uri, 3, i);
			assert(b);
			b = res.fetch(access, 4, i);
			assert(b);
			


			//обновить активный хост
			if(hostId.isNull())
			{
				htmlcxx::Uri u(uri);

				//выбрать
				pgc::Result res = c.query(_stMainSelectActiveHost, utils::MVA(_net_hostAccessDelay, u.hostname()));
				IF_PGRES_ERROR(return true, res);

				if(res.rows())
				{
					//хост есть

					//если хост уже использоватся в этой транзакции - пропустить страницу
					if(usedActiveHosts.end() != usedActiveHosts.find(hostId))
					{
						IF_PGRES_ERROR(return true, c.query(_stMainUpdatePageActiveHost, utils::MVA(pageId, hostId)));
						continue;
					}

					//проверить таймаут на хост
					utils::Variant atimeTooOld;
					bool b = res.fetch(atimeTooOld, 1, 0);
					assert(b);
					b = res.fetch(hostId, 0, 0);
					assert(b);

					if(atimeTooOld.to<bool>())
					{
						//таймаут истек, обновить шамп и отработать страницу

						IF_PGRES_ERROR(return true, c.query(_stMainUpdateActiveHostAtime, hostId));
					}
					else
					{
						//таймаут еще не истек, пометить страницу хостом и пока не отрабатывать

						IF_PGRES_ERROR(return true, c.query(_stMainUpdatePageActiveHost, utils::MVA(pageId, hostId)));
						continue;
					}
				}
				else
				{
					//нет хоста, добавить
					pgc::Result res = c.query(_stMainInsertActiveHost, u.hostname());
					IF_PGRES_ERROR(return true, res);

					bool b = res.fetch(hostId, 0, 0);
					assert(b);
				}
			}
			else
			{
				//хост есть

				//если хост уже использоватся в этой транзакции - пропустить страницу
				if(usedActiveHosts.end() != usedActiveHosts.find(hostId))
				{
					continue;
				}

				//обновить хосту штамп доступа
				IF_PGRES_ERROR(return true, c.query(_stMainUpdateActiveHostAtime, hostId));
			}

			//запомнить использованный хост
			usedActiveHosts.insert(hostId);

			//обновить статус страницы на 'pend'
			IF_PGRES_ERROR(return true, c.query(_stMainUpdatePageStatusPend, utils::MVA(pageId, hostId)));

			//грузить
			async::Mutex::ScopedLock sl(_mtxWorkers);
			_numWorkers++;
			async::spawn(boost::bind(&Service::uriLoader, this, pageId, hostId, instanceId, uri, access.to<int>()));
		}
		IF_PGRES_ERROR(return true, c.query(_stCommit));

		return true;
	}

	///////////////////////////////////////////////////////////////////
	bool Service::workerInstancesDeleteOld()
	{
		pgc::Connection c = _db.allocConnection();
		pgc::Result res = c.query(_stInsatanceDeleteOld);
		IF_PGRES_ERROR(return false, res);

		return false;
	}

	///////////////////////////////////////////////////////////////////
	bool Service::workerPageRestatusPend()
	{
		pgc::Connection c = _db.allocConnection();
		pgc::Result res = c.query(_stPageRestatusPend, utils::Variant(_deadWorkerTimeout));
		IF_PGRES_ERROR(return false, res);

		return false;
	}

	///////////////////////////////////////////////////////////////////
	bool Service::workerHostDeleteOld()
	{
		pgc::Connection c = _db.allocConnection();
		IF_PGRES_ERROR(return true, c.query(_stBegin));

		//IF_PGRES_ERROR(return true, c.query(_stLockPage));
		//IF_PGRES_ERROR(return true, c.query(_stLockActiveHost));

		pgc::Result res = c.query(_stHostDeleteOld, utils::Variant(_activeHostDeleteTimeout));
		IF_PGRES_ERROR(return false, res);

		IF_PGRES_ERROR(return true, c.query(_stCommit));

		return false;
	}

	///////////////////////////////////////////////////////////////////
	bool Service::workerPageRuleApplyer()
	{
		pgc::Connection c = _db.allocConnection();

		IF_PGRES_ERROR(return false, c.query(_stBegin));

		//IF_PGRES_ERROR(return false, c.query(_stLockInstance));
		//IF_PGRES_ERROR(return false, c.query(_stLockPageRule));
		//IF_PGRES_ERROR(return false, c.query(_stLockPage));
		//IF_PGRES_ERROR(return false, c.query(_stLockPageRef));

		pgc::Result res = c.query(_stPageRuleApplyerSelectPage, utils::Variant(_pagesToLoadGranula));
		IF_PGRES_ERROR(return false, res);

		size_t amount = res.rows();
		for(size_t i(0); i<amount; i++)
		{
			utils::Variant instanceId;
			bool b = res.fetch(instanceId,0,i);
			assert(b);

			_prac.update(c, instanceId.to<boost::int64_t>());
		}

		IF_PGRES_ERROR(return false, c.query(_stCommit));

		return amount?true:false;
	}
	//////////////////////////////////////////////////////////////////////////
	namespace
	{
		//////////////////////////////////////////////////////////////////////////
		bool isGoodContentType(const http::client::Response::Segment *seg)
		{
			if(!seg)
			{
				return false;
			}

			http::client::Response::Segment s;
			s = boost::algorithm::find_first(*seg, "html");
			if(s)
			{
				return true;
			}
			s = boost::algorithm::find_first(*seg, "text");
			if(s)
			{
				return true;
			}
			s = boost::algorithm::find_first(*seg, "xml");
			if(s)
			{
				return true;
			}
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
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

		boost::shared_ptr<htmlcxx::CharsetConverter> convertorSimple()
		{
			boost::shared_ptr<htmlcxx::CharsetConverter> cc;
			cc.reset(new htmlcxx::CharsetConverter("UTF-8", "UTF-8//IGNORE"));
			return cc;
		}

		boost::shared_ptr<htmlcxx::CharsetConverter> convertorFromContentTypeOrDom(
			const http::InputMessage::Segment *cts,
			const tree<htmlcxx::HTML::Node> &tr)
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

			tree<htmlcxx::HTML::Node>::iterator iter = tr.begin();
			tree<htmlcxx::HTML::Node>::iterator end = tr.end();
			for(; iter!=end; ++iter)
			{
				htmlcxx::HTML::Node &n = *iter;
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

			return convertorSimple();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void Service::uriLoader(const utils::Variant &pageId, const utils::Variant &hostId, const utils::Variant &instanceId, const utils::Variant &uri, int access)
	{
		WorkerRaii raii(_mtxWorkers, _numWorkers, _evtWorkerDone);
		assert((access&PageRule::ea_useLinks) || (access&PageRule::ea_useWords));
		assert(! (access&PageRule::ea_ignore) );

		boost::chrono::time_point<boost::chrono::system_clock> start = 
			boost::chrono::system_clock::now();

		http::client::Response resp;
		boost::system::error_code ec = _htc.get(resp, uri.as<std::string>().data());

		boost::chrono::milliseconds getTime = boost::chrono::duration_cast<boost::chrono::milliseconds>(boost::chrono::system_clock::now() - start);

		if(ec)
		{
			WLOG("get: "<<ec<<", ["<<uri.as<std::string>()<<"]");
			pgc::Connection c = _db.allocConnection();
			IF_PGRES_ERROR(return, c.query(_stUpdatePageStatus, utils::MVA(pageId, "get failed")));
			return;
		}
		ec = resp.readFirstLine();
		if(ec)
		{
			WLOG("read first line: "<<ec<<", ["<<uri.as<std::string>()<<"]");
			pgc::Connection c = _db.allocConnection();
			IF_PGRES_ERROR(return, c.query(_stUpdatePageStatus, utils::MVA(pageId, "read first line failed")));
			return;
		}

		ec = resp.readHeaders();
		if(ec)
		{

			WLOG("read headers: "<<ec<<", ["<<uri.as<std::string>()<<"]");
			pgc::Connection c = _db.allocConnection();
			IF_PGRES_ERROR(return, c.query(_stUpdatePageStatus, utils::MVA(pageId, "read headers failed")));
			return;
		}

		htmlcxx::Uri redirectUri;
		switch(resp.status())
		{
		case http::esc_200:
			//ok
			break;
		case http::esc_301:
		case http::esc_302:
		case http::esc_303:
			{
				if(const http::client::Response::Segment *locs = resp.header(http::hn::location))
				{
					redirectUri = htmlcxx::Uri(std::string(locs->begin(), locs->end()));
				}
				if(redirectUri.isOk())
				{
					//ok
					break;
				}
			}
			//err, no break
		default:
			WLOG("bad status: "<<std::string(resp.firstLine().begin(), resp.firstLine().end())<<", ["<<uri.as<std::string>()<<"]");
			pgc::Connection c = _db.allocConnection();
			IF_PGRES_ERROR(return, c.query(_stUpdatePageStatus, utils::MVA(pageId, "bad status")));
			return;
		}

		http::HeaderValue<http::Unsigned> length = resp.header(http::hn::contentLength);
		if(length.isCorrect() && length.value() > _maxHttpBodySize)
		{
			WLOG("too big: "<<length.value()<<", ["<<uri.as<std::string>()<<"]");
			pgc::Connection c = _db.allocConnection();
			IF_PGRES_ERROR(return, c.query(_stUpdatePageStatus, utils::MVA(pageId, "too big")));
			return;
		}

		const http::client::Response::Segment *contentType = resp.header(http::hn::contentType);
		if(!isGoodContentType(contentType) && !redirectUri.isOk())
		{
			WLOG("bad ct: "<<(contentType?std::string(contentType->begin(), contentType->end()):std::string("absent"))<<", ["<<uri.as<std::string>()<<"]");
			pgc::Connection c = _db.allocConnection();
			IF_PGRES_ERROR(return, c.query(_stUpdatePageStatus, utils::MVA(pageId, "bad content type")));
			return;
		}

		ec = resp.readBody();
		if(ec)
		{

			WLOG("read body: "<<ec<<", ["<<uri.as<std::string>()<<"]");
			pgc::Connection c = _db.allocConnection();
			IF_PGRES_ERROR(return, c.query(_stUpdatePageStatus, utils::MVA(pageId, "read body failed")));
			return;
		}


		//parse
		std::deque<htmlcxx::Uri> uris;
		std::string text;
		parse(
			resp, 
			uri.as<std::string>(), 
			access&PageRule::ea_useLinks?&uris:NULL, 
			access&PageRule::ea_useWords?&text:NULL);
		if(redirectUri.isOk())
		{
			uris.push_back(redirectUri);
		}

		//применить статус и текст
		pgc::Connection c = _db.allocConnection();
		pgc::Result res;

		boost::shared_ptr<htmlcxx::CharsetConverter> cc = convertorSimple();

		IF_PGRES_ERROR(return, c.query(_stBegin));

		//IF_PGRES_ERROR(return, c.query(_stLockInstance));
		IF_PGRES_ERROR(return, c.query(_stLockPage));
		//IF_PGRES_ERROR(return, c.query(_stLockPageRef));

		//просчитать ссылки и добавить новые страницы
		std::vector<char> refPageIds;
		BOOST_FOREACH(htmlcxx::Uri &uri2, uris)
		{
			if(uri2.scheme()!="http" && uri2.scheme()!="https")
			{
				continue;
			}

//  			if(uri2.hostname()!="127.0.0.1")
//  			{
//  				continue;
//  			}

			utils::Variant uri2v = htmlcxx::Uri::encode(htmlcxx::Uri::decode(uri2.unparse(htmlcxx::Uri::REMOVE_FRAGMENT)));
			res = c.query(
				_stLoaderSelectPage,
				utils::MVA(instanceId, uri2v)
			);
			IF_PGRES_ERROR(return, res);

			if(!res.rows())
			{
				res = c.query(
					_stLoaderInsertPage,
					utils::MVA(instanceId, uri2v)
				);
				IF_PGRES_ERROR(return, res);
			}

			utils::Variant page2Id;
			bool b = res.fetch(page2Id, 0,0);
			assert(b);

			boost::int64_t i64 = page2Id;
			refPageIds.insert(refPageIds.end(), (char *)&i64, (char *)&i64 + 8);
		}

		res = c.query(
			_stLoaderUpdatePage,
			utils::MVA(
				pageId,
				cc->convert(std::string(resp.firstLine().begin(), resp.firstLine().end())),
				text,
				refPageIds,
				cc->convert(std::string(resp.headers().begin(), resp.headers().end())),
				resp.channel().endpointRemote().address().to_string(),
				getTime.count()
			)
		);
		IF_PGRES_ERROR(return, res);


		IF_PGRES_ERROR(return, c.query(_stCommit));

		TLOG(instanceId.to<std::string>()<<", "<<uri.to<std::string>());
	}

	//////////////////////////////////////////////////////////////////////////
	namespace
	{
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
		const std::string &baseUriString,
		std::deque<htmlcxx::Uri> *uris,
		std::string *text)
	{

		//подготовить базовый урл
		htmlcxx::Uri base(baseUriString);
		if(!base.isOk())
		{
			assert(0);
			return;
		}

		//парсить dom
		htmlcxx::HTML::ParserDom parser;
		parser.parse(resp.body().begin(), resp.body().end());
		const tree<htmlcxx::HTML::Node> &tr = parser.getTree();

		//выявить кодировку из заголовков, meta html
		//сформировать конвертор в utf-8 если нужен
		boost::shared_ptr<htmlcxx::CharsetConverter> cc =
			convertorFromContentTypeOrDom(resp.header(http::hn::contentType), tr);

		//перебирать ноды, перерабатывать каждую
		if(uris)
		{
			tree<htmlcxx::HTML::Node>::iterator iter = tr.begin();
			tree<htmlcxx::HTML::Node>::iterator end = tr.end();

			for(; iter!=end; ++iter)
			{
				htmlcxx::HTML::Node &n = *iter;
				if(n.isTag() && boost::algorithm::iequals(n.tagName(), "a"))
				{
					n.parseAttributes();
					std::pair<bool, std::string> p = n.attribute("href");

					if(p.first)
					{
						htmlcxx::Uri uri(htmlcxx::HTML::convert_link(p.second, base));
						if(uri.isOk())
						{
							uris->push_back(uri);
						}
					}
				}
			}
		}

		if(text)
		{
			tree<htmlcxx::HTML::Node>::iterator iter = tr.begin();
			tree<htmlcxx::HTML::Node>::iterator end = tr.end();
			tree<htmlcxx::HTML::Node>::iterator parent;

			for(; iter!=end; ++iter)
			{
				htmlcxx::HTML::Node &n = *iter;
				if(!n.isTag() && !n.isComment())
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
							*text += utils::htmlEntitiesDecode(cc->convert(n.text()));
						}
						else
						{
							*text += utils::htmlEntitiesDecode(n.text());
						}
						*text += " ";
					}
				}
			}
		}
	}


	//////////////////////////////////////////////////////////////////////////
	bool Service::insertPageIfAbsent(pgc::Connection c, boost::int64_t instanceId, const std::string &uri)
	{
		pgc::Result res = c.query(_stInsertPageSelectId, utils::MVA(instanceId, uri));
		IF_PGRES_ERROR(return false, res);
		
		if(res.rows())
		{
			return true;
		}
		
		res = c.query(_stInsertPage, utils::MVA(instanceId, uri));
		IF_PGRES_ERROR(return false, res);
		return true;
	}
			
	//////////////////////////////////////////////////////////////////////////
	bool Service::workerMerger()
	{
		pgc::Connection c = _db.allocConnection();
		pgc::Result res;

		//транзакция
		IF_PGRES_ERROR(return false, c.query(_stBegin));

		//IF_PGRES_ERROR(return false, c.query(_stLockInstance));
		//IF_PGRES_ERROR(return false, c.query(_stLockPage));
		//IF_PGRES_ERROR(return false, c.query(_stLockPageRef));

		//	выбрать один инстанс в состоянии load который все загрузил
		res = c.query(
			"SELECT i.id "
			"FROM instance AS i "
			"WHERE "
			"	(i.stage=10 OR i.stage=20 AND i.atime<=CURRENT_TIMESTAMP-$1::INTERVAL) AND "
			"	NOT EXISTS( "
			"		SELECT * "
			"		FROM PAGE AS p "
			"		WHERE "
			"			p.instance_id=i.id AND "
			"			(p.access IS NULL OR "
			"				((p.access=2 OR p.access=4 OR p.access=6) AND "
			"				(p.status IS NULL OR p.status='pend')) "
			"			) "
			"	) "
			"LIMIT 1",
			utils::Variant(_deadWorkerTimeout)

		);
		IF_PGRES_ERROR(return false, res);

		if(!res.rows())
		{
			//нет готовых
			IF_PGRES_ERROR(return false, c.query(_stRollback));
			return false;
		}

		utils::Variant instanceId;
		bool b = res.fetch(instanceId,0,0);
		assert(b);

		//	перевесли его в состояние merge, выбрать его данные
		IF_PGRES_ERROR(return false, c.query("UPDATE instance SET stage=20, atime=CURRENT_TIMESTAMP WHERE id=$1", instanceId));

		//конец транзакции
		IF_PGRES_ERROR(return false, c.query(_stCommit));

		//считать
		{

			try
			{
				ReportGenerator rg(_tmpDir, _hunspell);

				if(!rg.isOk())
				{
					return false;
				}

				//выявить количество страниц
				res = c.query("SELECT count(*) FROM page WHERE instance_id=$1 AND (access=2 OR access=4 OR access=6) AND status IS NOT NULL", instanceId);
				IF_PGRES_ERROR(return false, res);

				size_t pagesAmount = res.fetchInt32(0,0);
#define PAGESGRANULA (1000)

				//заливать идентификаторы страницы
				for(size_t i(0); i<pagesAmount; i+=PAGESGRANULA)
				{
					res = c.query("SELECT id FROM page WHERE instance_id=$1 AND (access=2 OR access=4 OR access=6) AND status IS NOT NULL ORDER BY id LIMIT $2 OFFSET $3", utils::MVA(instanceId, PAGESGRANULA, i));
					IF_PGRES_ERROR(return false, res);

					utils::Variant pageIds;
					bool b = res.fetchColumn(pageIds, 0, 0, (size_t)-1);
					assert(b);

					if(!rg.addPageIds(pageIds))
					{
						return false;
					}
				}

				if(!rg.fixPageIds())
				{
					return false;
				}

				//заливать урлы, тексты и ссылки страниц
				for(size_t i(0); i<pagesAmount; i+=PAGESGRANULA)
				{
					res = c.query("SELECT id, uri, ref_page_ids, text FROM page WHERE instance_id=$1 AND (access=2 OR access=4 OR access=6) AND status IS NOT NULL ORDER BY id LIMIT $2 OFFSET $3", utils::MVA(instanceId, PAGESGRANULA, i));
					IF_PGRES_ERROR(return false, res);

					utils::Variant pageRows;
					bool b = res.fetchRowsList(pageRows, 0, (size_t)-1);
					assert(b);

					if(!rg.setPagesContent(pageRows))
					{
						return false;
					}
				}

				//считать веса фраз
				if(!rg.evalPhraseWeights())
				{
					return false;
				}
			}
			catch (sqlitepp::exception &e)
			{
				ELOG("sqlitepp exception: "<<e.what());
			}

		}

		//транзакция
		IF_PGRES_ERROR(return false, c.query(_stBegin));

		//	перевесли его в состояние report, сохранить данные
		IF_PGRES_ERROR(return false, c.query("UPDATE instance SET stage=30, atime=CURRENT_TIMESTAMP WHERE id=$1", instanceId));

		//конец транзакции
		IF_PGRES_ERROR(return false, c.query(_stCommit));

		return true;
	}


}}
