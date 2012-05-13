#include "pch.hpp"
#include "http/impl/server.hpp"
#include "http/server/log.hpp"

#include "http/server/request.hpp"
#include "http/server/impl/request.hpp"
#include "http/statusCode.hpp"
#include "http/headerName.hpp"
#include "http/headerValue.hpp"
#include "utils/implAccess.hpp"

namespace http { namespace impl
{
	namespace po = boost::program_options;
	using namespace http::server;

	////////////////////////////////////////////////////////////////////
	utils::OptionsPtr Server::prepareOptions(const char *prefix)
	{
		utils::OptionsPtr options(new utils::Options(prefix));

		options->addOption(
			"host",
			po::value<std::string>()->default_value("127.0.0.1"),
			"host name for this server");

		options->addOption(
			"port",
			po::value<std::string>()->default_value("8080"),
			"port name or number for this server");

		options->addOption(
			"request.readGranula",
			po::value<size_t>()->default_value(1024),
			"buffer size during read request data");

		options->addOption(
			"response.writeGranula",
			po::value<size_t>()->default_value(65536),
			"buffer size during write response data");


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

		return options;
	}

	////////////////////////////////////////////////////////////////////
	Server::Server()
		: _host("localhost")
		, _port("8080")
		, _requestReadGranula(1400)
		, _responseWriteGranula(1400)
	{
	}

	////////////////////////////////////////////////////////////////////
	Server::~Server()
	{
	}

	////////////////////////////////////////////////////////////////////
	void Server::init(async::Service asrv, utils::OptionsPtr options)
	{
		_asrv = asrv;
		_asrv.connectOnStart(boost::bind(&Server::start, shared_from_this()));
		_asrv.connectOnStop(boost::bind(&Server::stop, shared_from_this()));

		utils::Options &o = *options;
		_host = o["host"].as<std::string>();
		_port = o["port"].as<std::string>();
		_requestReadGranula = o["request.readGranula"].as<size_t>();
		_responseWriteGranula = o["response.writeGranula"].as<size_t>();
	}

	////////////////////////////////////////////////////////////////////
	boost::signals2::connection Server::connectOnRequest(const boost::function<void(const server::Request &)> &f)
	{
		return _onRequest.connect(f);
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
	size_t Server::requestReadGranula() const
	{
		return _requestReadGranula;
	}

	////////////////////////////////////////////////////////////////////
	size_t Server::responseWriteGranula() const
	{
		return _responseWriteGranula;
	}

	////////////////////////////////////////////////////////////////////
	void Server::onRequest(http::server::impl::RequestPtr requestImpl)
	{
		_asrv.spawn(boost::bind(&Server::onRequest_f, shared_from_this(), requestImpl));
	}

	////////////////////////////////////////////////////////////////////
	void Server::onRequest_f(http::server::impl::RequestPtr requestImpl)
	{
		Request request = utils::ImplAccess<Request>(requestImpl);

		if(	!request.readFirstLine() ||
			!request.readHeaders())
		{
			return;
		}

		switch(request.method_())
		{
		default:
			if(!request.readBody()) return;

			{
				Response response = request.response();
				if(
					!response.firstLine(esc_405) ||
					!response.header(http::hn::connection, HeaderValue<Connection>(ec_close)) ||
					!response.bodyFlush())
				{
					//connection lost? ok
				}
			}
			return;
		case em_OPTIONS:
			{
				Response response = request.response();
				if(
					!response.firstLine(esc_405) ||
					!response.header("Allow: GET", 10) ||
					!response.bodyFlush())
				{
					//connection lost? ok
				}
			}
			return;
		case em_GET:
			_onRequest(request);
			/*{
				Response response = request.response();
				response
					.statusCode(esc_200)
					.header("Content-Type: text/plain")
					.body("hello world")
					;
				if(!response.flush())
				{
					//connection lost? ok
				}
			}*/

			break;
		/*case em_POST:
			if(!request.readBody()) return;
			break;*/
		}


		//_onRequest(request);
	}

	////////////////////////////////////////////////////////////////////
	void Server::onAccept(boost::system::error_code ec, net::Channel channel)
	{
		if(ec)
		{
			ELOG("accept failed: "<<ec);
			return;
		}

		http::server::impl::RequestPtr requestImpl(new http::server::impl::Request(shared_from_this(), channel));
		onRequest_f(requestImpl);
	}



}}
