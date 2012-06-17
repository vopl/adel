#include "pch.hpp"
#include "scom/impl/service.hpp"
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
	{
	}

	///////////////////////////////////////////////////////////////////
	Service::~Service()
	{
	}

	///////////////////////////////////////////////////////////////////
	void Service::start()
	{
	}

	///////////////////////////////////////////////////////////////////
	void Service::stop()
	{
	}
}}
