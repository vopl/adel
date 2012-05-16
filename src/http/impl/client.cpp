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
	{
		assert(0);
	}

	//////////////////////////////////////////////////////////////////////
	Client::~Client()
	{
		assert(0);
	}

	//////////////////////////////////////////////////////////////////////
	void Client::init(async::Service asrv, utils::OptionsPtr options)
	{
		assert(0);
	}

	//////////////////////////////////////////////////////////////////////
	boost::system::error_code Client::connect(
		client::impl::RequestPtr &request,
		const char *host, const char *service, bool useSsl)
	{
		assert(0);
	}

	//////////////////////////////////////////////////////////////////////
	boost::system::error_code Client::connect(
		client::impl::RequestPtr &request,
		const char *url)
	{
		assert(0);
	}
}}
