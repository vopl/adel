#include "pch.hpp"
#include <iostream>
#include <boost/bind.hpp>
#include "adel/manager.hpp"

#include "async/log.hpp"
#include "pgc/log.hpp"
#include "adel/log.hpp"
#include "net/log.hpp"

#include "http/log.hpp"
#include "http/server/log.hpp"
#include "http/server.hpp"
#include "http/server/handlerFs.hpp"
#include "http/client/log.hpp"
#include "http/client.hpp"
#include "http/headerName.hpp"

#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>

using namespace std;
namespace po = boost::program_options;




void testClient(http::Client c)
{
	http::client::Request request;
	boost::system::error_code ec;

	ec = c.connectGet(request, "http://127.0.0.1:8080/index.html?a=b");
	if(ec)
	{
		std::cout<<"get: "<<ec<<std::endl;
		return;
	}

	ec = request.bodyFlush();
	if(ec)
	{
		std::cout<<"bodyFlush: "<<ec<<std::endl;
		return;
	}

	http::client::Response response = request.response();

	ec = response.readFirstLine();
	if(ec)
	{
		std::cout<<"readFirstLine: "<<ec<<std::endl;
		return;
	}

	std::cout<<"firstLine: ["<<std::string(response.firstLine().begin(), response.firstLine().end())<<"]"<<std::endl;

	ec = response.readHeaders();
	if(ec)
	{
		std::cout<<"readHeaders: "<<ec<<std::endl;
		return;
	}
	std::cout<<"headers: ["<<std::string(response.headers().begin(), response.headers().end())<<"]"<<std::endl;

	ec = response.readBody();
	if(ec)
	{
		std::cout<<"readBody: "<<ec<<std::endl;
		return;
	}
	std::cout<<"body: ["<<std::string(response.body().begin(), response.body().end())<<"]"<<std::endl;

}


//////////////////////////////////////////////////////////////////////
int main(int argc, const char **argv)
{
	for(int k=0; k<1; k++)
	{
		po::options_description desc("general");
		desc.add_options()
				("help", "produce help message")
				("help-log", "produce help message for logging")
				("run,R", "run")
				("config", boost::program_options::value<std::string>()->default_value("../etc/global.conf"),
					"configuration file, filesystem path, relative from working directory of this process or absolute");

		po::variables_map varsGeneral;
		try
		{
			po::store(po::command_line_parser(argc, argv).options(desc).allow_unregistered().run(), varsGeneral);
		}
		catch(boost::program_options::unknown_option &e)
		{
			std::cout<<e.what()<<std::endl<<desc<<std::endl;
			return EXIT_FAILURE;
		}
		po::notify(varsGeneral);

		//////////////////////////////////////
		utils::OptionsPtr omanager = adel::Manager::prepareOptions("manager");
		desc.add(omanager->desc());
		utils::OptionsPtr ohttpServer1 = http::Server::prepareOptions("httpServer1");
		desc.add(ohttpServer1->desc());
		utils::OptionsPtr ohttpServer1HandlerFs = http::server::HandlerFs::prepareOptions("httpServer1.handlerFs");
		desc.add(ohttpServer1HandlerFs->desc());
		utils::OptionsPtr ohttpClient1 = http::Client::prepareOptions("httpClient1");
		desc.add(ohttpClient1->desc());

		if(varsGeneral.count("help"))
		{
		    cout << desc << "\n";
		    break;
		}

		//////////////////////////////////////
		po::options_description desc_log("log");
		utils::OptionsPtr oasyncLog = async::prepareOptionsLog();
		desc_log.add(oasyncLog->desc());
		utils::OptionsPtr oadelLog = adel::prepareOptionsLog();
		desc_log.add(oadelLog->desc());
		utils::OptionsPtr opgcLog = pgc::prepareOptionsLog();
		desc_log.add(opgcLog->desc());
		utils::OptionsPtr onetLog = net::prepareOptionsLog();
		desc_log.add(onetLog->desc());
		utils::OptionsPtr ohttpServerLog = http::server::prepareOptionsLog();
		desc_log.add(ohttpServerLog->desc());
		utils::OptionsPtr ohttpClientLog = http::client::prepareOptionsLog();
		desc_log.add(ohttpClientLog->desc());
		utils::OptionsPtr ohttpLog = http::prepareOptionsLog();
		desc_log.add(ohttpLog->desc());

		if(varsGeneral.count("help-log"))
		{
		    cout << desc_log << "\n";
		    break;
		}
		desc.add(desc_log);

		po::parsed_options parsedOptions1(&desc);
		po::parsed_options parsedOptions2(&desc);

		try
		{
			parsedOptions1 = po::parse_command_line(argc, argv, desc);
			if(varsGeneral.count("config"))
			{
				std::ifstream ifstr(varsGeneral["config"].as<std::string>().c_str());
				if(ifstr)
				{
					std::cout << "load config file: " << varsGeneral["config"].as<std::string>() << std::endl;
					parsedOptions2
						= po::parse_config_file(ifstr, desc);
				}
				else
				{
					cout << "config load failed: " <<  varsGeneral["config"].as<std::string>() << std::endl;
					break;
				}
			}
		}
		catch(boost::program_options::unknown_option &e)
		{
			std::cout<<e.what()<<std::endl<<desc<<std::endl;
			return EXIT_FAILURE;
		}

		ohttpLog->store(&parsedOptions1, &parsedOptions2);
		ohttpServerLog->store(&parsedOptions1, &parsedOptions2);
		ohttpClientLog->store(&parsedOptions1, &parsedOptions2);
		oasyncLog->store(&parsedOptions1, &parsedOptions2);
		oadelLog->store(&parsedOptions1, &parsedOptions2);
		opgcLog->store(&parsedOptions1, &parsedOptions2);
		onetLog->store(&parsedOptions1, &parsedOptions2);

		omanager->store(&parsedOptions1, &parsedOptions2);
		ohttpServer1->store(&parsedOptions1, &parsedOptions2);
		ohttpServer1HandlerFs->store(&parsedOptions1, &parsedOptions2);
		ohttpClient1->store(&parsedOptions1, &parsedOptions2);

		if(varsGeneral.count("run"))
		{
			async::initLog(oasyncLog);
			adel::initLog(oadelLog);
			pgc::initLog(opgcLog);
			net::initLog(onetLog);
			http::initLog(ohttpLog);
			http::server::initLog(ohttpServerLog);
			http::client::initLog(ohttpClientLog);


			adel::Manager manager(omanager);
			manager.asrv().setAsGlobal(true);
			{
				http::Server httpServer1(ohttpServer1);

				http::server::HandlerFs httpServer1HandlerFs(ohttpServer1HandlerFs);
				httpServer1.onRequest(boost::bind(&http::server::HandlerFs::onRequest, httpServer1HandlerFs, _1));

				http::Client httpClient1(ohttpClient1);

				manager.asrv().onStart(boost::bind(&testClient, httpClient1));

				//adel::HttpClient httpClient(manager, "global");
				//adel::Postgres postgres(manager, "global");
				//adel::Memcache memcache(manager, "global");

				//run workspace
				manager.run();
			}
		}

	}

	return EXIT_SUCCESS;
}
