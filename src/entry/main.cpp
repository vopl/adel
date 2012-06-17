#include "pch.hpp"
#include <iostream>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include "async/manager.hpp"
#include "scom/service.hpp"

#include "async/log.hpp"
#include "pgc/log.hpp"
#include "net/log.hpp"
#include "http/log.hpp"
#include "http/server/log.hpp"
#include "http/client/log.hpp"
#include "scom/log.hpp"

#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>

using namespace std;
namespace po = boost::program_options;

//////////////////////////////////////////////////////////////////////
int main(int argc, const char **argv)
{
	for(int k=0; k<1; k++)
	{
		po::options_description desc("general");
		desc.add_options()
				("help", "produce help message")
				("help-log", "produce help message for logging")
				("run,R", boost::program_options::value<std::vector<std::string> >(),
					"module to run, [scom]")
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

		if(!varsGeneral.count("run"))
		{
		    cout << desc << "\n";
			return 0;
		}

		bool runScom = false;
		BOOST_FOREACH(const std::string &module, varsGeneral["run"].as<std::vector<std::string> >())
		{
			if("scom" == module)
			{
				runScom = true;
			}
		}

		if(!runScom)
		{
		    cout << desc << "\n";
			return 0;
		}

		//////////////////////////////////////
		utils::OptionsPtr omanager = async::Manager::prepareOptions("async.manager");
		desc.add(omanager->desc());
		utils::OptionsPtr oscom = scom::Service::prepareOptions("scom");
		desc.add(oscom->desc());

		if(varsGeneral.count("help"))
		{
		    cout << desc << "\n";
		    break;
		}

		//////////////////////////////////////
		po::options_description desc_log("log");
		utils::OptionsPtr oasyncLog = async::prepareOptionsLog();
		desc_log.add(oasyncLog->desc());
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
		utils::OptionsPtr oscomLog = scom::prepareOptionsLog();
		desc_log.add(oscomLog->desc());

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
		opgcLog->store(&parsedOptions1, &parsedOptions2);
		onetLog->store(&parsedOptions1, &parsedOptions2);
		oscomLog->store(&parsedOptions1, &parsedOptions2);

		omanager->store(&parsedOptions1, &parsedOptions2);
		oscom->store(&parsedOptions1, &parsedOptions2);

		if(varsGeneral.count("run"))
		{
			async::initLog(oasyncLog);
			pgc::initLog(opgcLog);
			net::initLog(onetLog);
			http::initLog(ohttpLog);
			http::server::initLog(ohttpServerLog);
			http::client::initLog(ohttpClientLog);
			scom::initLog(oscomLog);


			async::Manager manager(omanager);
			manager.asrv().setAsGlobal(true);
			{

				scom::Service *scom = NULL;
				if(runScom)
				{
					scom = new scom::Service(oscom);
					manager.asrv().onStart(boost::bind(&scom::Service::start, scom));
					manager.asrv().onStop(boost::bind(&scom::Service::stop, scom));
				}

				//run workspace
				manager.run();

				if(scom)
				{
					delete scom;
					scom = NULL;
				}
			}
		}

	}

	return EXIT_SUCCESS;
}
