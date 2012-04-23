#include "pch.hpp"
#include "adel/manager.hpp"
#include "adel/log.hpp"
#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#include <boost/asio/signal_set.hpp>

#include <stdio.h>

namespace adel
{
	///////////////////////////////////////////////////////////////////
	utils::OptionsPtr Manager::prepareOptions(const char *prefix)
	{
		utils::OptionsPtr options(new utils::Options(prefix));

		options->addOption(
			"numWorkers",
			boost::program_options::value<std::string>()->default_value("hardware"),
			"worker threads amount, positive integer or keyword 'hardware' - will use number of hardware threads");

		options->addOption(
			"fiber.stackSize",
			boost::program_options::value<size_t>()->default_value(65536),
			"fiber stack size, in bytes");

		options->addOption(
			"fiber.maxAmount",
			boost::program_options::value<size_t>()->default_value(2000),
			"maximum amount of fibers");

		options->addOption(
			"fiber.gc.period",
			boost::program_options::value<double>()->default_value(0.5),
			"fibers garbage collector activation period, in seconds");

		options->addOption(
			"fiber.gc.spare.all",
			boost::program_options::value<double>()->default_value(20.0),
			"fibers garbage collector do not work if amount of ALL fibers less than this percentage, in percents of fiber.maxAmount");

		options->addOption(
			"fiber.gc.spare.idle",
			boost::program_options::value<double>()->default_value(20.0),
			"fibers garbage collector do not work if amount of IDLE fibers less than this percentage, in percents of fiber.maxAmount");

		return options;

	}

	///////////////////////////////////////////////////////////////////
	Manager::Manager(utils::OptionsPtr optionsPtr)
		: _asrv()
		, _numWorkers(1)
		, _fiber_stackSize(1024*64)
		, _fiber_maxAmount(1000)
		, _fiber_gc_period(boost::posix_time::seconds(10))
		, _fiber_gc_spare_all(200)
		, _fiber_gc_spare_idle(200)

	{
		utils::Options &ops(*optionsPtr);

		//////////////////////////////////////
		std::string numWorkers = ops["numWorkers"].as<std::string>();

		if("hardware" == numWorkers)
		{
			_numWorkers = boost::thread::hardware_concurrency();
		}
		else
		{
			_numWorkers = atoi(numWorkers.c_str());
			if(_numWorkers < 1)
			{
				_numWorkers = 1;
			}
		}

		ILOG("_numWorkers: "<<_numWorkers);

		//////////////////////////////////////
		//_fiber_stackSize(1024*64)
		_fiber_stackSize = ops["fiber.stackSize"].as<size_t>();
		ILOG("_fiber_stackSize: "<<_fiber_stackSize);

		//_fiber_maxAmount(1000)
		_fiber_maxAmount = ops["fiber.maxAmount"].as<size_t>();
		ILOG("_fiber_maxAmount: "<<_fiber_maxAmount);

		//_fiber_gc_period(boost::posix_time::seconds(10))
		double d = ops["fiber.gc.period"].as<double>();
		_fiber_gc_period =
				boost::posix_time::seconds(long(d)) +
				boost::posix_time::microseconds(boost::int64_t(d-long(d))*1000000);
		ILOG("_fiber_gc_period: "<<_fiber_gc_period);

		//_fiber_gc_spare_all(200)
		d = ops["fiber.gc.spare.all"].as<double>();
		_fiber_gc_spare_all = boost::int64_t(double(_fiber_maxAmount) * d /100);
		ILOG("_fiber_gc_spare_all: "<<_fiber_gc_spare_all);

		//_fiber_gc_spare_idle(200)
		d = ops["fiber.gc.spare.idle"].as<double>();
		_fiber_gc_spare_idle = boost::int64_t(double(_fiber_maxAmount) * d /100);
		ILOG("_fiber_gc_spare_idle: "<<_fiber_gc_spare_idle);


	}

	///////////////////////////////////////////////////////////////////
	Manager::~Manager()
	{

	}

	///////////////////////////////////////////////////////////////////
	async::Service Manager::asrv()
	{
		return _asrv;
	}

	///////////////////////////////////////////////////////////////////
	void Manager::run()
	{
		_asrv.setupFibers(_fiber_stackSize, _fiber_maxAmount);
		_asrv.start(_numWorkers);

		boost::asio::signal_set ss(_asrv.io());

//Сигнал прерывания (Ctrl-C) с терминала
#ifdef SIGINT
		ss.add(SIGINT);
#endif

//Сигнал завершения (сигнал по умолчанию для утилиты kill)
#ifdef SIGTERM
		ss.add(SIGTERM);
#endif

//Ctrl-Break sequence
#ifdef SIGBREAK
		ss.add(SIGBREAK);
#endif

//Закрытие терминала
#ifdef SIGHUP
		ss.add(SIGHUP);
#endif

//Безусловное завершение
#ifdef SIGKILL
		//ss.add(SIGKILL);
#endif

//Сигнал «Quit» с терминала (Ctrl-\)
#ifdef SIGQUIT
		ss.add(SIGQUIT);
#endif

//Сигнал остановки с терминала (Ctrl-Z)
#ifdef SIGTSTP
		ss.add(SIGTSTP);
#endif

		ss.async_wait(boost::bind(&Manager::onSignal, this, _1, _2));

		setvbuf(stdin, (char *)NULL, _IONBF, 0);
		//////////////////////////////////////////////////////////////////////////
		bool bStop = false;
		while(!bStop)
		{
			int ch = fgetc(stdin);
			switch(ch)
			{
			case 'e':
				ILOG("exit request");
				bStop = true;
				break;
			case EOF:
				bStop = true;
				break;
			default:
				ILOG("?");
				break;
			}
		};
		ss.cancel();

		_asrv.stop();
	}

	///////////////////////////////////////////////////////////////////
	void Manager::onSignal(const boost::system::error_code& error, int signal_number)
	{
		if(!error)
		{
			//ILOG("signal catched: "<<signal_number);
			fclose(stdin);
		}
	}

}
