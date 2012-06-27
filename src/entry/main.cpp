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




#include "http/server.hpp"
#include "http/server/handlerFs.hpp"


#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>

using namespace std;
namespace po = boost::program_options;

//////////////////////////////////////////////////////////////////////
#define CHECK_ERR(x) if(scom::ee_ok != x) {std::cout<<"scom err: "<<x<<", line "<<__LINE__<<std::endl;}
void testScomClient(scom::Service *scom)
{
	scom::Auth auth;
	scom::EError err;

	//create
	{
		err = scom->create(auth, "secret");
		CHECK_ERR(err);
	}

	//setup
	{
		std::vector<scom::PageRule> rules;

		scom::PageRule r1 = {
			"127.0.0.1",
			scom::PageRule::ea_ignore | scom::PageRule::ek_domain | scom::PageRule::ek_negative,
			-3, 0, -1};
		rules.push_back(r1);

		scom::PageRule r4 = {
			"http://127.0.0.1:8080/index.html",
			scom::PageRule::ea_useLinks | scom::PageRule::ea_useWords | scom::PageRule::ek_reference,
			0, 10, 999999};
		rules.push_back(r4);

		err = scom->setup(auth, rules);
		CHECK_ERR(err);
	}

	//start
	{
		err = scom->start(auth);
		CHECK_ERR(err);
	}

    //ping-ping
	{
		for(;;)
		{
			scom::Status status;
			err = scom->ping(status, auth);
			CHECK_ERR(err);

			if(status._stage == scom::Status::es_report)
			{
				std::cout<<"scom complete"<<std::endl;
				break;
			}
			std::cout<<"----------- scom stage: "<<status._stage<<", "<<status._workProcessed<<"/"<<status._workVolume<<std::endl;
			async::timeout(1000).wait();
		}
	}
    //getResult

//     //delete
// 	{
// 		err = scom->destroy(auth);
// 		CHECK_ERR(err);
// 	}
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
				("run,R", boost::program_options::value<std::vector<std::string> >(),
					"module to run, [scom]")
				("config", boost::program_options::value<std::string>()->default_value("../etc/global.conf"),
					"configuration file, filesystem path, relative from working directory of this process or absolute");

		po::variables_map varsGeneral;
		try
		{
			po::store(po::command_line_parser(argc, argv).options(desc).allow_unregistered().run(), varsGeneral);
		}
		catch(std::exception &e)
		{
			std::cout<<e.what()<<std::endl;
			return EXIT_FAILURE;
		}
		catch(...)
		{
			std::cout<<"unknown exception"<<std::endl;
			return EXIT_FAILURE;
		}
		po::notify(varsGeneral);

		//////////////////////////////////////
		utils::OptionsPtr omanager = async::Manager::prepareOptions("async.manager");
		desc.add(omanager->desc());
		utils::OptionsPtr oscom = scom::Service::prepareOptions("scom");
		desc.add(oscom->desc());

		utils::OptionsPtr s_opts = http::Server::prepareOptions("http.server");
		desc.add(s_opts->desc());
		utils::OptionsPtr sh_opts = http::server::HandlerFs::prepareOptions("http.server.fs");
		desc.add(sh_opts->desc());


		if(varsGeneral.count("help"))
		{
		    cout << desc << std::endl;
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
		    cout << desc_log << std::endl;
		    break;
		}
		desc.add(desc_log);

		///////////////////////////////////////////////////////
		if(!varsGeneral.count("run"))
		{
		    cout << "no module to run" << std::endl;
			return -4;
		}

		///////////////////////////////////////////////////////
		bool runScom = false;
		BOOST_FOREACH(const std::string &module, varsGeneral["run"].as<std::vector<std::string> >())
		{
			if("scom" == module)
			{
				cout << "run scom" << std::endl;
				runScom = true;
			}
		}

		if(!runScom)
		{
		    cout << "unknown module to run" << std::endl;
			return -5;
		}


		///////////////////////////////////////////////////////
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
		catch(std::exception &e)
		{
			std::cout<<e.what()<<std::endl;
			return EXIT_FAILURE;
		}
		catch(...)
		{
			std::cout<<"unknown exception"<<std::endl;
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

		s_opts->store(&parsedOptions1, &parsedOptions2);
		sh_opts->store(&parsedOptions1, &parsedOptions2);


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
				http::Server s(s_opts);
				http::server::HandlerFs sh(sh_opts);

				manager.asrv().onStart(boost::bind(&http::Server::start, s));
				manager.asrv().onStop(boost::bind(&http::Server::stop, s));
				s.onRequest(boost::bind(&http::server::HandlerFs::onRequest, sh, _1));

				////////////////////////////////
				scom::Service *scom = NULL;
				if(runScom)
				{
					scom = new scom::Service(oscom);
					manager.asrv().onStart(boost::bind(&scom::Service::start, scom));
					manager.asrv().onStop(boost::bind(&scom::Service::stop, scom));
					
					manager.asrv().onStart(boost::bind(&testScomClient, scom));
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
