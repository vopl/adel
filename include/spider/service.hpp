#ifndef _SPIDER_SERVICE_HPP_
#define _SPIDER_SERVICE_HPP_

#include "utils/options.hpp"
#include "pgc/db.hpp"

#include "async/mutex.hpp"
#include "async/event.hpp"

#include "http/client.hpp"

#include "htmlcxx/html/Uri.h"
#include <deque>

namespace spider
{
	using namespace htmlcxx;

	class Service
	{
	public:
		Service(utils::OptionsPtr optionsPtr, http::Client htc);
		virtual ~Service();

		static utils::OptionsPtr prepareOptions(const char *prefix);

		void start();
		void stop();

	private:
		void onConnectionMade(size_t numConnections);
		void onConnectionLost(size_t numConnections);

		void processLoop();
		void processOne(utils::Variant hostId, utils::Variant pageId, utils::Variant url);

		void parse(http::client::Response resp, const std::string &baseUrlString, std::deque<Uri> &urls);

	private:
		http::Client _htc;

		std::string	_pgc_connectionString;
		size_t		_pgc_maxConnections;

		size_t		_net_concurrency;
		size_t		_net_hostDelay;

		pgc::Db		_db;

		volatile bool	_stop;
		async::Mutex	_mtx;

		async::Mutex	_mtxWorkers;
		size_t			_numWorkers;
		async::Event	_evtWorkerDone;

		void		*_hunspell;
	};
}
#endif
