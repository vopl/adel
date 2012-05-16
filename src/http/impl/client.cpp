#include "pch.hpp"
#include "http/impl/client.hpp"

namespace http { namespace impl
{
	namespace po = boost::program_options;
	using namespace http::client;

	//////////////////////////////////////////////////////////////////////
	utils::OptionsPtr Client::prepareOptions(const char *prefix)
	{
		utils::OptionsPtr options(new utils::Options(prefix));
		options->addOption(
			"request.writeGranula",
			po::value<size_t>()->default_value(32768),
			"buffer size during write request data");

		options->addOption(
			"response.readGranula",
			po::value<size_t>()->default_value(1024),
			"buffer size during read response data");

		options->addOption(
			"timeout",
			po::value<size_t>()->default_value(10000),
			"read/write timeout in milliseconds");

		return options;

	}

	//////////////////////////////////////////////////////////////////////
	Client::Client()
		: _responseReadGranula(1024)
		, _requestWriteGranula(32768)
		, _timeout(10000)
	{
	}

	//////////////////////////////////////////////////////////////////////
	Client::~Client()
	{
	}

	//////////////////////////////////////////////////////////////////////
	void Client::init(utils::OptionsPtr options)
	{
		utils::Options &o = *options;

		_responseReadGranula = o["response.readGranula"].as<size_t>();
		_requestWriteGranula = o["request.writeGranula"].as<size_t>();
		_timeout = o["timeout"].as<size_t>();

	}

	//////////////////////////////////////////////////////////////////////
	boost::system::error_code Client::connect(
		client::impl::RequestPtr &request,
		const char *host, const char *service, bool useSsl)
	{
		async::Future2<boost::system::error_code, net::Channel> cres =
			_connector.connect(host, service, useSsl);
		cres.wait();

		if(cres.data1NoWait())
		{
			return cres.data1NoWait();
		}

		request.reset(new client::impl::Request(shared_from_this(), cres.data2NoWait()));
		return cres.data1NoWait();
	}

	//////////////////////////////////////////////////////////////////////
	boost::system::error_code Client::connect(
		client::impl::RequestPtr &request,
		const char *url)
	{
		assert(0);
	}
}}
