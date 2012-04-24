#include "pch.hpp"
#include <iostream>
#include <boost/bind.hpp>
#include "adel/manager.hpp"

#include "async/log.hpp"
#include "pgc/log.hpp"
#include "adel/log.hpp"
#include "net/log.hpp"

#include "net/http/server/log.hpp"
#include "net/http/server.hpp"
#include "net/http/server/handlerFs.hpp"

#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>

using namespace std;
namespace po = boost::program_options;

int main(int argc, const char **argv)
{
	for(int k=0; k<1; k++)
	{
		po::options_description desc("general");
		desc.add_options()
				("help", "produce help message")
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
		utils::OptionsPtr ohttpServer1 = net::http::Server::prepareOptions("httpServer1");
		desc.add(ohttpServer1->desc());
		utils::OptionsPtr ohttpServer1HandlerFs = net::http::server::HandlerFs::prepareOptions("httpServer1.handlerFs");
		desc.add(ohttpServer1HandlerFs->desc());

		//////////////////////////////////////
		utils::OptionsPtr oasyncLog = async::prepareOptionsLog();
		desc.add(oasyncLog->desc());
		utils::OptionsPtr oadelLog = adel::prepareOptionsLog();
		desc.add(oadelLog->desc());
		utils::OptionsPtr opgcLog = pgc::prepareOptionsLog();
		desc.add(opgcLog->desc());
		utils::OptionsPtr onetLog = net::prepareOptionsLog();
		desc.add(onetLog->desc());
		utils::OptionsPtr ohttpLog = net::http::server::prepareOptionsLog();
		desc.add(ohttpLog->desc());

		if(varsGeneral.count("help"))
		{
		    cout << desc << "\n";
		    break;
		}

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
		oasyncLog->store(&parsedOptions1, &parsedOptions2);
		oadelLog->store(&parsedOptions1, &parsedOptions2);
		opgcLog->store(&parsedOptions1, &parsedOptions2);
		onetLog->store(&parsedOptions1, &parsedOptions2);

		omanager->store(&parsedOptions1, &parsedOptions2);
		ohttpServer1->store(&parsedOptions1, &parsedOptions2);
		ohttpServer1HandlerFs->store(&parsedOptions1, &parsedOptions2);

		if(varsGeneral.count("run"))
		{
			async::initLog(oasyncLog);
			adel::initLog(oadelLog);
			pgc::initLog(opgcLog);
			net::initLog(onetLog);
			net::http::server::initLog(ohttpLog);


			adel::Manager manager(omanager);
			{
				net::http::Server httpServer1(manager.asrv(), ohttpServer1);

				//net::http::server::HandlerFs httpServer1HandlerFs(ohttpServer1HandlerFs);
				//httpServer1.connectOnRequest(boost::bind(&net::http::server::HandlerFs::onRequest, httpServer1HandlerFs, _1));

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
