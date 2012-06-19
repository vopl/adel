#ifndef _SCOM_IMPL_SERVICE_HPP_
#define _SCOM_IMPL_SERVICE_HPP_

#include "utils/options.hpp"
#include "scom/service.hpp"
#include "pgc/db.hpp"
#include "async/mutex.hpp"
#include "async/event.hpp"

#include "scom/impl/pageRuleApplyersContainer.hpp"

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
		async::Event	_evtIface;

		std::string						_pgc_connectionString;
		size_t							_pgc_maxConnections;
		utils::Variant::TimeDuration	_net_defaultHostDelay;
		size_t							_net_concurrency;

		pgc::Db		_db;

		utils::Variant::TimeDuration _pageRestatusPentTimeout;
		utils::Variant::TimeDuration _activeHostTimeout;

		PageRuleApplyersContainer	_prac;

	private:
		typedef bool (Service::* TWorker)();
		void runWorker(TWorker worker, size_t idleTimeout=0);
		void workerWrapper(TWorker worker, size_t idleTimeout);

		bool workerMain();
		bool workerInstancesDeleteOld();
		bool workerPageRestatusPend();
		bool workerHostDeleteOld();

	};
}}
#endif
