#ifndef _SCOM_IMPL_SERVICE_HPP_
#define _SCOM_IMPL_SERVICE_HPP_

#include "utils/options.hpp"
#include "scom/service.hpp"
#include "pgc/db.hpp"
#include "async/mutex.hpp"
#include "async/event.hpp"

namespace scom { namespace impl
{
	class Service
	{
	public:
		Service(utils::OptionsPtr optionsPtr);
		virtual ~Service();

		static utils::OptionsPtr prepareOptions(const char *prefix);

		void start();
		void stop();

		EError create(
			Auth &auth,
			std::string password);

		EError ping(
			Status &status,
			const Auth &auth);

		EError setup(
			const Auth &auth,
			const std::vector<PageRule> &srcRules,
			const std::vector<PageRule> &dstRules);

		EError start(
			const Auth &auth);

		EError stop(
			const Auth &auth);


	private:
		bool		_isWork;

		async::Mutex	_mtx;
		async::Mutex	_mtxWorkers;
		size_t			_numWorkers;
		async::Event	_evtWorkerDone;

		std::string	_pgc_connectionString;
		size_t		_pgc_maxConnections;
		pgc::Db		_db;
	};
}}
#endif
