#include "pch.hpp"
#include "net/http/impl/server.hpp"
#include "net/http/server/log.hpp"

namespace net { namespace http { namespace impl
{
	namespace po = boost::program_options;
	using namespace net::http::server;

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
	Server::Server()
	{
	}

	////////////////////////////////////////////////////////////////////
	Server::~Server()
	{
	}

	////////////////////////////////////////////////////////////////////
	void Server::init(async::Service asrv, utils::OptionsPtr options)
	{
		asrv.connectOnStart(boost::bind(&Server::start, shared_from_this()));
		asrv.connectOnStop(boost::bind(&Server::stop, shared_from_this()));

		utils::Options &o = *options;
		_host = o["host"].as<std::string>();
		_port = o["port"].as<std::string>();
	}

	////////////////////////////////////////////////////////////////////
	boost::signals2::connection Server::connectOnRequest(const boost::function<void(const server::Request &)> &f)
	{
		assert(0);
	}

	////////////////////////////////////////////////////////////////////
	void Server::start()
	{
		_connectionOnAccept = _acceptor.connectOnAccept(boost::bind(&Server::onAccept, shared_from_this(), _1, _2));
		async::Future<boost::system::error_code> ret = _acceptor.listen(_host.c_str(), _port.c_str(), false);

		if(ret.data())
		{
			_connectionOnAccept.disconnect();
			ELOG("listen failed: "<<ret.data());
			return;
		}
		ILOG("listen "<<_host<<":"<<_port);

	}

	////////////////////////////////////////////////////////////////////
	void Server::stop()
	{
		_acceptor.unlisten();
		_connectionOnAccept.disconnect();
	}

	////////////////////////////////////////////////////////////////////
	void Server::onAccept(const boost::system::error_code &ec, Channel channel)
	{
		if(ec)
		{
			ELOG("accept failed: "<<ec);
		}
		//TLOG(__FUNCTION__);
	}



}}}
