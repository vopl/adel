#include "pch.hpp"
#include "net/http/server/impl/handlerFs.hpp"
#include "net/http/server/log.hpp"

namespace net { namespace http { namespace server { namespace impl
{
	namespace po = boost::program_options;

	//////////////////////////////////////////////////////////////////////////
	utils::OptionsPtr HandlerFs::prepareOptions(const char *prefix)
	{
		utils::OptionsPtr options(new utils::Options(prefix));

		options->addOption(
			"root",
			po::value<std::string>()->default_value("../statics"),
			"directory with static files");

		return options;
	}

	//////////////////////////////////////////////////////////////////////////
	HandlerFs::HandlerFs(utils::OptionsPtr options)
	{
		assert(0);
	}

	//////////////////////////////////////////////////////////////////////////
	HandlerFs::~HandlerFs()
	{
		assert(0);
	}

	//////////////////////////////////////////////////////////////////////////
	void HandlerFs::onRequest(const net::http::server::Request &r)
	{
		assert(0);
	}

}}}}
