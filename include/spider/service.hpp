#ifndef _SPIDER_SERVICE_HPP_
#define _SPIDER_SERVICE_HPP_

#include "utils/options.hpp"
#include "pgc/db.hpp"

#include "async/mutex.hpp"
#include "async/event.hpp"

#include "http/client.hpp"

#include "htmlcxx/html/Uri.h"
#include "spider/wordBucket.hpp"
#include "spider/phraseStreamer.hpp"
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

		void parse(
			http::client::Response resp,
			const std::string &baseUrlString,
			std::deque<Uri> &urls,
			std::deque<WordBucket> &wordBuckets);

		bool updatePageWords(pgc::Connection c, const utils::Variant &pageId, const std::deque<WordBucket> &wordBuckets);
		bool updatePageWords2(pgc::Connection c, const utils::Variant &pageId, const Word *words[2]);
		bool updatePageWords3(pgc::Connection c, const utils::Variant &pageId, const Word *words[3]);

		template <class Streamer>
		bool updatePageWords2(pgc::Connection c, const utils::Variant &pageId, const std::deque<WordBucket> &wordBuckets)
		{
			Phrase<2> phrase;
			Streamer streamer(&wordBuckets);
			while(streamer.next(phrase))
			{
				const Word *words[2];
				while(phrase.nextCombination(words))
				{
					if(!updatePageWords2(c, pageId, words))
					{
						return false;
					}
				}
			}

			return true;
		}

		template <class Streamer>
		bool updatePageWords3(pgc::Connection c, const utils::Variant &pageId, const std::deque<WordBucket> &wordBuckets)
		{
			Phrase<3> phrase;
			Streamer streamer(&wordBuckets);
			while(streamer.next(phrase))
			{
				const Word *words[3];
				while(phrase.nextCombination(words))
				{
					if(!updatePageWords3(c, pageId, words))
					{
						return false;
					}
				}
			}

			return true;
		}

		void processMoveTmp2();
		void processMoveTmp3();

	private:
		pgc::Statement	_stBegin;
		pgc::Statement	_stCommit;
		pgc::Statement	_stRollback;

		pgc::Statement	_stLockSite;
		pgc::Statement	_stLockPage;

		pgc::Statement	_stSelectPages4Process;
		pgc::Statement	_stUpdateSiteTime;
		pgc::Statement	_stUpdatePageAddress;
		pgc::Statement	_stUpdatePageStatus;

		pgc::Statement	_stSelectSiteIdWhereName;
		pgc::Statement	_stInsertSite;
		pgc::Statement	_stSelectPageIdWhereUri;
		pgc::Statement	_stUpdateSiteAmountPlusOne;
		pgc::Statement	_stUpdateSiteAmountMinusOne;
		pgc::Statement	_stInsertPage;

		pgc::Statement	_stInsertReference;
		pgc::Statement	_stUpdatePage;

		pgc::Statement	_stInsertWord2ToPage_tmp;
		pgc::Statement	_stInsertWord3ToPage_tmp;

		pgc::Statement	_stMoveWord2_tmp;
		pgc::Statement	_stMoveWord3_tmp;

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
