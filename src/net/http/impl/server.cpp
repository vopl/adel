#include "net/http/impl/server.hpp"

namespace net { namespace http { namespace impl
{
	namespace po = boost::program_options;

	////////////////////////////////////////////////////////////////////
	utils::OptionsPtr Server::prepareOptions(const char *prefix)
	{
		utils::OptionsPtr options(new utils::Options(prefix));

		options->addOption(
			"host",
			po::value<std::string>()->default_value("localhost"),
			"host name for this server");

		options->addOption(
			"port",
			po::value<std::string>()->default_value("80"),
			"port name or number for this server");
/*
		options->addOption(
			"ssl",
			po::value<bool>()->default_value(false),
			"enable ssl mode");

		options->addOption(
			"ssl.method",
			po::value<std::string>()->default_value("v23"),
			"ssl method, one of:\n"
			"v1  - TLS version 1 server\n"
			"v2  - SSL version 2 server\n"
			"v3  - SSL version 3 server\n"
			"v23 - SSL/TLS server");

		//default_workarounds Implement various bug workarounds.
		options->addOption(
			"ssl.options.default_workarounds",
			po::value<bool>()->default_value(true),
			"Implement various bug workarounds");

		//no_tlsv1 Disable TLS v1.
		options->addOption(
			"ssl.options.no_tlsv1",
			po::value<bool>()->default_value(false),
			"Disable TLS v1");

		//no_sslv2 Disable SSL v2.
		options->addOption(
			"ssl.options.no_sslv2",
			po::value<bool>()->default_value(false),
			"Disable SSL v2");

		//no_sslv3 Disable SSL v3.
		options->addOption(
			"ssl.options.no_sslv3",
			po::value<bool>()->default_value(false),
			"Disable SSL v3");

		//single_dh_use Always create a new key when using tmp_dh parameters.
		options->addOption(
			"ssl.options.single_dh_use",
			po::value<bool>()->default_value(true),
			"Always create a new key when using tmp_dh parameters");

		assert(!"more settings");

		options->addOption(
			"ssl.password",
			po::value<std::string>()->default_value("test"),
			"password for sertificate file");
*/


		options->addOption(
			"statics",
			po::value<std::string>()->default_value("../statics"),
			"directory with static files");

		return options;
	}

	////////////////////////////////////////////////////////////////////
	Server::Server(async::Service asrv, utils::OptionsPtr options)
	{
		assert(0);
	}

	////////////////////////////////////////////////////////////////////
	Server::~Server()
	{
		assert(0);
	}

	////////////////////////////////////////////////////////////////////
	boost::signals2::connection Server::connectOnRequest(const boost::function<void(const server::Request &)> &f)
	{
		assert(0);
	}

	////////////////////////////////////////////////////////////////////
	void Server::start()
	{
		assert(0);
	}

	////////////////////////////////////////////////////////////////////
	void Server::stop()
	{
		assert(0);
	}


}}}
