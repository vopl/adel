#ifndef _PGC_CONNECTIONHOLDER_HPP_
#define _PGC_CONNECTIONHOLDER_HPP_

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <libpq-fe.h>
#include "async/service.hpp"
#include "async/future.hpp"
#include "async/mutex.hpp"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

#include <boost/thread.hpp>

#include "impl/statement.hpp"
#include "pgc/result.hpp"
#include "pgc/connection.hpp"

namespace pgc { namespace impl
{
	using namespace boost;
	using namespace multi_index;

	//////////////////////////////////////////////////////////////////////////
	class BindData;
	typedef shared_ptr<BindData> BindDataPtr;

	//////////////////////////////////////////////////////////////////////////
	class Db;
	typedef shared_ptr<Db> DbPtr;


	//////////////////////////////////////////////////////////////////////////
	class ConnectionHolder
		: public enable_shared_from_this<ConnectionHolder>
	{
	private:
		//тип протокола постгресового сокета для asio
		struct PGSockProtocol
		{
			int _family; int _type; int _protocol;
			PGSockProtocol(int f, int t, int p) : _family(f), _type(t), _protocol(p) {}

			int family() const		{return _family;}
			int type() const		{return _type;}
			int protocol() const	{return _protocol;}

			struct endpoint
			{
				PGSockProtocol protocol(){ return PGSockProtocol(0,0,0);}
			};
		};

		//состояние подготовленного запроса
		struct StatementPrepareState
		{
			//идентификатор
			std::string					_prid;
			//экземпляр, у него уникальный адрес, sql
			StatementWtr				_stm;
			//время последнего доступа
			posix_time::ptime			_accessTime;
			//время создания
			posix_time::ptime			_createTime;
		};

		//контейнер, индексирован по адресу запроса, по времени последнего доступа и повремени создания
		typedef multi_index_container<
			StatementPrepareState,
			indexed_by<
				ordered_unique<
					member<
						StatementPrepareState,
						StatementWtr,
						&StatementPrepareState::_stm
					>
				>,
				ordered_non_unique<
					member<
						StatementPrepareState,
						posix_time::ptime,
						&StatementPrepareState::_accessTime
					>
				>,
				ordered_non_unique<
					member<
						StatementPrepareState,
						posix_time::ptime,
						&StatementPrepareState::_createTime
					>
				>
			>
		> TPrepareds;

		//помогалка для модификации 'времени доступа' внутри мультииндекса
		struct ChangeAccessTime
		{
			ChangeAccessTime(const posix_time::ptime& accessTime):_accessTime(accessTime){}

			void operator()(StatementPrepareState& e)
			{
				e._accessTime = _accessTime;
			}

		private:
			posix_time::ptime _accessTime;
		};

		//помогалка для модификации 'идентификатора подготовленного запроса' внутри мультииндекса
		struct ChangePrid
		{
			ChangePrid(const std::string& prid):_prid(prid){}

			void operator()(StatementPrepareState& e)
			{
				e._prid = _prid;
			}

		private:
			std::string _prid;
		};

		//очередь входящих запросов
		enum ERequestType
		{
			ertQuery,
			ertQueryWithPrepare,
			ertQueryEndWork,
		};
		struct SRequest
		{
			ERequestType				_ert;
			async::Future<pgc::Result>	_res;
			SRequest(ERequestType ert, async::Future<pgc::Result> res)
				: _ert(ert), _res(res)
			{}
		};
		typedef boost::shared_ptr<SRequest> SRequestPtr;

		struct SRequestQuery
			: SRequest
		{
			std::string	_sql;
			BindDataPtr	_bindData;
			SRequestQuery(async::Future<pgc::Result> res, std::string sql, BindDataPtr bindData)
				: SRequest(ertQuery, res), _sql(sql), _bindData(bindData)
			{}
		};
		struct SRequestQueryWithPrepare
			: SRequest
		{
			StatementPtr		_s;
			BindDataPtr			_bindData;
			SRequestQueryWithPrepare(async::Future<pgc::Result> res, StatementPtr s, BindDataPtr bindData)
				: SRequest(ertQueryWithPrepare, res), _s(s), _bindData(bindData)
			{}
		};

		struct SRequestEndWork
			: SRequest
		{
			SRequestEndWork(async::Future<pgc::Result> res)
				: SRequest(ertQueryEndWork, res)
			{}
		};

		typedef std::deque<SRequestPtr> TRequests;

		typedef asio::basic_stream_socket<PGSockProtocol> PGSock;
	private:
		//////////////////////////////////////////////////////////////////////////
		DbPtr				_db;
		PGconn				*_pgcon;
		PGSock				_sock;
		bool				_integerDatetimes;

	private:
		//ограничение на количество одновременно хранимых запросов
		static const size_t	_max = 1000;
		//таймаут удаления по бездействию
		static const size_t	_timeout = 1000*60*60;//millisec, проверь доку по pgc::Connection, там эта цифра фигурирует как "1 час"
		//время на момент начала работы
		posix_time::ptime	_now;
		//контейнер с запросами
		TPrepareds			_prepareds;

		TRequests			_requests;
		boost::mutex		_mtx;

		async::Mutex		_mtxProcess;

	private:
		bool hasPrepared(StatementWtr p);
		std::string getPrid(StatementWtr p);
		void genPrid(StatementWtr p);
		void delPrepared(StatementWtr p);

	private:
		//помогалки для инициализации постгресового сокета в asio
		static int sockFamily(int sock);
		static int sockType(int sock);

	private:
		void setResult(async::Future<pgc::Result> &res, bool success=false);

	private:
		void pushRequest(const SRequestPtr &request);
		void processRequest_f();

	private:
		void processSingle(async::Future<pgc::Result> res);
		void processQueryWithPrepare(async::Future<pgc::Result> res, StatementPtr s, BindDataPtr bindData);

	private:
		void runQuery_f(async::Future<pgc::Result> res, const std::string &sql, BindDataPtr bindData = BindDataPtr());

		void runPrepare_f(
			async::Future<pgc::Result> res,
			const std::string &prid,
			const std::string &sql,
			BindDataPtr bindData);

		void runQueryPrepared_f(
			async::Future<pgc::Result> res,
			const std::string &prid,
			BindDataPtr bindData);

		//а так же всякие describe
		//void runDescribePrepared...
		//void runDescribePortal...

	private:
		void runQueryWithPrepare_f(async::Future<pgc::Result> res, StatementPtr s, BindDataPtr bindData);

	private:
		bool deallocateOne(const std::string &prid, async::Future<pgc::Result> res);
		void runEndWork_f(async::Future<pgc::Result> res);

	public:
		async::Future<system::error_code> send0();
		async::Future<system::error_code> recv0();

	public:
		ConnectionHolder(DbPtr db, PGconn *pgcon);
		~ConnectionHolder();

		PGconn *pgcon();

		void onOpen();
		EConnectionStatus status();
		void close();

		bool integerDatetimes();

	public:
		void runQuery(async::Future<pgc::Result> res, const std::string &sql, BindDataPtr bindData);
		void runQueryWithPrepare(async::Future<pgc::Result> res, pgc::Statement s, BindDataPtr bindData);

	public:
		void beginWork();
		void endWork();


	};
	typedef shared_ptr<ConnectionHolder> ConnectionHolderPtr;
}}
#endif
