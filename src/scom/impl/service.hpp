#ifndef _SCOM_IMPL_SERVICE_HPP_
#define _SCOM_IMPL_SERVICE_HPP_

#include "utils/options.hpp"
#include "scom/service.hpp"
#include "pgc/db.hpp"
#include "async/mutex.hpp"
#include "async/event.hpp"
#include "http/client.hpp"

#include "scom/impl/pageRuleApplyersContainer.hpp"
#include "scom/impl/wordBucket.hpp"

#ifdef near
#	undef near
#endif
#include <hunspell.hxx>

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
			const std::vector<PageRule> &rules);

		EError start(
			const Auth &auth);

		EError stop(
			const Auth &auth);

		EError destroy(
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

		utils::Variant::TimeDuration	_workerIdleTimeoutMain;
		utils::Variant::TimeDuration	_workerIdleTimeoutCleanupper;
		utils::Variant::TimeDuration	_ruleApplyerCacheTimeout;

		size_t							_pagesToLoadGranula;
		size_t							_maxHttpBodySize;
		size_t							_maxWorkers;

		utils::Variant::TimeDuration	_pageRestatusPentTimeout;
		utils::Variant::TimeDuration	_activeHostTimeout;

		pgc::Db		_db;
		PageRuleApplyersContainer	_prac;

		http::Client	_htc;
		Hunspell		*_hunspell;

	private:
		typedef bool (Service::* TWorker)();
		void runWorker(TWorker worker, utils::Variant::TimeDuration idleTimeout);
		void workerWrapper(TWorker worker, utils::Variant::TimeDuration idleTimeout);

		bool workerMain();
		bool workerInstancesDeleteOld();
		bool workerPageRestatusPend();
		bool workerHostDeleteOld();
		bool workerPageRuleApplyer();

		void uriLoader(const utils::Variant &pageId, const utils::Variant &hostId, const utils::Variant &instanceId, const utils::Variant &uri, int access);
		void parse(
			http::client::Response resp,
			const std::string &baseUriString,
			std::deque<htmlcxx::Uri> *uris,
			std::string *text);

	private:
		bool insertPageIfAbsent(pgc::Connection c, boost::int64_t instanceId, const std::string &uri);

	private:
		pgc::Statement _stCreateInsertInstance;
		pgc::Statement _stBegin;
		pgc::Statement _stCommit;
		pgc::Statement _stRollback;
		pgc::Statement _stPingSelectInstance;
		pgc::Statement _stPingUpdateInstance;
		
		pgc::Statement _stLockInstance;
		pgc::Statement _stLockPageRule;
		pgc::Statement _stLockPage;
		pgc::Statement _stLockPageRef;
		pgc::Statement _stLockActiveHost;

		pgc::Statement _stSetupSelectInstance;
		pgc::Statement _stSetupinsertPageRule;
		pgc::Statement _stSetupUpdateInstance;




		pgc::Statement _stStartSelectInstance;
		pgc::Statement _stStartUpdateInstance;
		pgc::Statement _stStopSelectInstance;
		pgc::Statement _stStopUpdateInstance;
		pgc::Statement _stDestroyDeleteInstance;
		pgc::Statement _stMainSelectPage;
		pgc::Statement _stMainSelectActiveHost;
		pgc::Statement _stMainUpdatePageActiveHost;
		pgc::Statement _stMainUpdateActiveHostAtime;
		pgc::Statement _stMainInsertActiveHost;
		pgc::Statement _stMainUpdatePageStatusPend;
		pgc::Statement _stInsatanceDeleteOld;
		pgc::Statement _stPageRestatusPend;
		pgc::Statement _stHostDeleteOld;
		pgc::Statement _stPageRuleApplyerSelectPage;
		pgc::Statement _stUpdatePageStatus;
		pgc::Statement _stLoaderUpdatePage;
		pgc::Statement _stLoaderSelectPage;
		pgc::Statement _stLoaderInsertPage;
		pgc::Statement _stLoaderInsertPageRef;
		pgc::Statement _stInsertPageSelectId;
		pgc::Statement _stInsertPage;


	};
}}
#endif
