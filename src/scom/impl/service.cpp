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
			boost::program_options::value<std::string>()->default_value("host=localhost port=5432 dbname=spider user=spider password=spider"),
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
	}

	///////////////////////////////////////////////////////////////////
	EError Service::ping(
		Status &status,
		const Auth &auth)
	{
		assert(0);
	}

	///////////////////////////////////////////////////////////////////
	EError Service::setup(
		const Auth &auth,
		const std::vector<PageRule> &srcRules,
		const std::vector<PageRule> &dstRules)
	{
		assert(0);
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

			//удалить просроченные

			//запускать готовые
		}
	}
}}
