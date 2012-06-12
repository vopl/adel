#include "pch.hpp"
#include "connectionHolder.hpp"
#include "async/service.hpp"
#include "async/future.hpp"

#include "pgc/impl/result.hpp"
#include "pgc/impl/db.hpp"
#include "pgc/impl/bindData.hpp"

#include "pgc/log.hpp"

#include "utils/implAccess.hpp"

namespace pgc { namespace impl
{
	//////////////////////////////////////////////////////////////////////////
	//различные помогаторы
	namespace
	{
		//////////////////////////////////////////////////////////////////////////
		char pridGenChars[] = "abcdefghijklmnopqrstuvwxyz01234567890";
		random::random_device pridGenRng;
		random::uniform_int_distribution<> pridGenIndexDist(0, sizeof(pridGenChars)-2);
		std::string pridGenerator()
		{
			std::string res;
			res.resize(24);
			res[0] = '_';
			for(int i = 1; i < 24; ++i)
			{
				res[i] = pridGenChars[pridGenIndexDist(pridGenRng)];
			}
			return res;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool ConnectionHolder::hasPrepared(StatementWtr p)
	{
		TPrepareds::nth_index<0>::type &ptrIndex = _prepareds.get<0>();
		TPrepareds::nth_index<0>::type::iterator iter = ptrIndex.find(p);
		return ptrIndex.end() != iter;
	}

    //////////////////////////////////////////////////////////////////////////
	std::string ConnectionHolder::getPrid(StatementWtr p)
	{
		TPrepareds::nth_index<0>::type &ptrIndex = _prepareds.get<0>();
		TPrepareds::nth_index<0>::type::iterator iter = ptrIndex.find(p);
		if(ptrIndex.end() != iter)
		{
			ptrIndex.modify(iter, ChangeAccessTime(_now));
			return iter->_prid;
		}

		StatementPrepareState sps =
		{
			pridGenerator(),
			p,
			_now,
			_now,
		};
		_prepareds.insert(sps);
		return sps._prid;
	}

	//////////////////////////////////////////////////////////////////////////
	void ConnectionHolder::genPrid(StatementWtr p)
	{
		TPrepareds::nth_index<0>::type &ptrIndex = _prepareds.get<0>();
		TPrepareds::nth_index<0>::type::iterator iter = ptrIndex.find(p);
		if(ptrIndex.end() != iter)
		{
			ptrIndex.modify(iter, ChangePrid(pridGenerator()));
			return;
		}

		StatementPrepareState sps =
		{
			pridGenerator(),
			p,
			_now,
			_now,
		};
		_prepareds.insert(sps);
	}

	//////////////////////////////////////////////////////////////////////////
	void ConnectionHolder::delPrepared(StatementWtr p)
	{
		TPrepareds::nth_index<0>::type &ptrIndex = _prepareds.get<0>();
		TPrepareds::nth_index<0>::type::iterator iter = ptrIndex.find(p);
		if(ptrIndex.end() != iter)
		{
			ptrIndex.erase(iter);
			return;
		}

		assert(!"unknown statement for delete");
	}

	//////////////////////////////////////////////////////////////////////////
	int ConnectionHolder::sockFamily(int sock)
	{
		sockaddr name;
		socklen_t name_len = sizeof(name);
		if(!getsockname(
			sock,
			&name,
			&name_len))
		{
			return name.sa_family;
		}
		else
		{
			ELOG(__FUNCTION__<<", getsockname failed");
		}
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	int ConnectionHolder::sockType(int sock)
	{
		int type;
		socklen_t length = sizeof(type);
		if(!getsockopt(sock, SOL_SOCKET, SO_TYPE, (char *)&type, &length))
		{
			return type;
		}
		else
		{
			ELOG(__FUNCTION__<<", getsockopt failed");
		}
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	void ConnectionHolder::setResult(async::Future<pgc::Result> &res, bool success)
	{
		ResultPtr di(new Result(PQmakeEmptyPGresult(_pgcon, success?PGRES_COMMAND_OK:PGRES_FATAL_ERROR), shared_from_this()));
		res.dataNoWait() = utils::ImplAccess<pgc::Result>(di);
		if(!success)
		{
			PQconsumeInput(_pgcon);
		}
		res.set();
	}

	//////////////////////////////////////////////////////////////////////////
	void ConnectionHolder::processRequest_f()
	{
		async::Mutex::ScopedLock sl(_mtxProcess);

		for(;;)
		{
			SRequestPtr request;
			{
				mutex::scoped_lock sl(_mtx);

				if(_requests.empty())
				{
					break;
				}

				if(ecsOk != status())
				{
					BOOST_FOREACH(const SRequestPtr &r, _requests)
					{
						ELOG(__FUNCTION__<<", "<<PQerrorMessage(_pgcon));
						setResult(r->_res);
					}
					_requests.clear();

					return;
				}
				request = _requests.front();
				_requests.erase(_requests.begin());
			}

			switch(request->_ert)
			{
			case ertQuery:
				{
					SRequestQuery *rd = static_cast<SRequestQuery *>(request.get());
					runQuery_f(rd->_res, rd->_sql, rd->_bindData);
				}
				break;
			case ertQueryWithPrepare:
				{
					SRequestQueryWithPrepare *rd = static_cast<SRequestQueryWithPrepare *>(request.get());
					runQueryWithPrepare_f(rd->_res, rd->_s, rd->_bindData);
				}
				break;
			case ertQueryEndWork:
				{
					SRequestEndWork *rd = static_cast<SRequestEndWork *>(request.get());
					runEndWork_f(rd->_res);
				}
				break;
			default:
				assert(!"unknown ert");
				throw("unknown ert");

				return;
			}
			request->_res.wait();
		}
	}


	//////////////////////////////////////////////////////////////////////////
	void ConnectionHolder::processSingle(async::Future<pgc::Result> res)
	{
		assert(_mtxProcess.isLocked());
		assert(!_now.is_not_a_date_time());

		//слать
		while(PQflush(_pgcon))
		{
			system::error_code ec = send0();
			if(ec)
			{
				ELOG(__FUNCTION__<<", send, "<<ec<<", "<<PQerrorMessage(_pgcon));
				setResult(res);
				return;
			}
		}

		//принимать
		PGresult *pgr = NULL;
		for(;;)
		{
			if(PQisBusy(_pgcon))
			{
				system::error_code ec = recv0();
				if(ec)
				{
					ELOG(__FUNCTION__<<", recv, "<<ec<<", "<<PQerrorMessage(_pgcon));
					setResult(res);
					return;
				}
			}

			if(!PQconsumeInput(_pgcon))
			{
				ELOG(__FUNCTION__<<", "<<PQerrorMessage(_pgcon));
				setResult(res);
				return;
			}

			PGresult *pgrNext = PQgetResult(_pgcon);
			if(!pgrNext)
			{
				if(pgr)
				{
					ResultPtr di(new Result(pgr, shared_from_this()));
					res.dataNoWait() = utils::ImplAccess<pgc::Result>(di);
					res.set();
				}
				else
				{
					assert(0);
					setResult(res, true);
				}
				return;
			}
			if(pgr)
			{
				PQclear(pgr);
				pgr = NULL;
			}
			pgr = pgrNext;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void ConnectionHolder::processQueryWithPrepare(async::Future<pgc::Result> res, StatementPtr s, BindDataPtr bindData)
	{
		assert(_mtxProcess.isLocked());
		assert(!_now.is_not_a_date_time());

		if(ecsOk != status())
		{
			ELOG(__FUNCTION__<<", "<<PQerrorMessage(_pgcon));
			setResult(res);
			return;
		}


		bool inTrans = false;
		PGTransactionStatusType tstatus = PQtransactionStatus(pgcon());
//		assert(PQTRANS_ACTIVE != tstatus);
		switch(tstatus)
		{
// 		case PQTRANS_ACTIVE:
		case PQTRANS_INTRANS:
		case PQTRANS_INERROR:
			inTrans = true;
			break;
		default:
			break;
		}

		for(;;)
		{
			if(inTrans)
			{
				async::Future<pgc::Result> r1;
				runQuery_f(r1, "SAVEPOINT pgcp"+getPrid(s));
				if(ersCommandOk != r1.data().status())
				{
					ELOG(__FUNCTION__<<", "<<PQerrorMessage(_pgcon));
					delPrepared(s);
					setResult(res);
					return;
				}
			}

			async::Future<pgc::Result> r2;
			runPrepare_f(
				r2,
				getPrid(s),
				s->getSql(),
				bindData);
			if(ersCommandOk != r2.data().status())
			{
				const char *s4 = r2.data().errorCode();

				if(s4 && !strcmp(s4, "42P05"))//DUPLICATE PREPARED STATEMENT
				{
					if(inTrans)
					{
						std::string oldPrid = getPrid(s);
						genPrid(s);

						async::Future<pgc::Result> r3;
						runQuery_f(r3, "ROLLBACK TO SAVEPOINT pgcp"+oldPrid);
						if(ersCommandOk != r3.data().status())
						{
							ELOG(__FUNCTION__<<", "<<PQerrorMessage(_pgcon));
							delPrepared(s);
							setResult(res);
							return;
						}
					}
					continue;
				}
				else
				{
					ELOG(__FUNCTION__<<", "<<PQerrorMessage(_pgcon));
					delPrepared(s);
					setResult(res);
					return;
				}
			}
			break;
		}

		if(inTrans)
		{
			async::Future<pgc::Result> r4;
			runQuery_f(r4, "RELEASE SAVEPOINT pgcp"+getPrid(s));
			if(ersCommandOk != r4.data().status())
			{
				ELOG(__FUNCTION__<<", "<<PQerrorMessage(_pgcon));
				delPrepared(s);
				setResult(res);
				return;
			}
		}

		runQueryPrepared_f(res, getPrid(s), bindData);
	}

	//////////////////////////////////////////////////////////////////////////
	void ConnectionHolder::runQuery_f(async::Future<pgc::Result> res, const std::string &sql, BindDataPtr bindData)
	{
		assert(_mtxProcess.isLocked());
		assert(!_now.is_not_a_date_time());

		if(bindData)
		{

			int nParams = (int)bindData->typ.size();
			const Oid *paramTypes = nParams?&bindData->typ[0]:NULL;
			const char * const *paramValues = nParams?&bindData->val[0]:NULL;
			const int *paramLengths = nParams?&bindData->len[0]:NULL;
			const int *paramFormats = nParams?&bindData->fmt[0]:NULL;

			if(!PQsendQueryParams (
				_pgcon,
				sql.c_str(),
				nParams,
				paramTypes,
				paramValues,
				paramLengths,
				paramFormats,
				1))
			{
				ELOG(__FUNCTION__<<", "<<PQerrorMessage(_pgcon));
				setResult(res);
				return;
			};
		}
		else
		{
			if(!PQsendQueryParams (
				_pgcon,
				sql.c_str(),
				0,
				NULL,
				NULL,
				NULL,
				NULL,
				1))
			{
				ELOG(__FUNCTION__<<", "<<PQerrorMessage(_pgcon));
				setResult(res);
				return;
			};
		}

		processSingle(res);
	}
	//////////////////////////////////////////////////////////////////////////
	void ConnectionHolder::runPrepare_f(
		async::Future<pgc::Result> res,
		const std::string &prid,
		const std::string &sql,
		BindDataPtr bindData)
	{
		assert(_mtxProcess.isLocked());
		assert(!_now.is_not_a_date_time());

		int nParams = bindData?(int)bindData->typ.size():0;
		const Oid *paramTypes = nParams?&bindData->typ[0]:NULL;
		if(!PQsendPrepare(
			_pgcon,
			prid.c_str(),
			sql.c_str(),
			nParams,
			paramTypes))
		{
			ELOG(__FUNCTION__<<", "<<PQerrorMessage(_pgcon));
			setResult(res);
			return;
		};

		processSingle(res);
	}

	//////////////////////////////////////////////////////////////////////////
	void ConnectionHolder::runQueryPrepared_f(
		async::Future<pgc::Result> res,
		const std::string &prid,
		BindDataPtr bindData)
	{
		assert(_mtxProcess.isLocked());
		assert(!_now.is_not_a_date_time());

		int nParams = bindData?(int)bindData->typ.size():0;
		const Oid *paramTypes = nParams?&bindData->typ[0]:NULL;
		const char * const *paramValues = nParams?&bindData->val[0]:NULL;
		const int *paramLengths = nParams?&bindData->len[0]:NULL;
		const int *paramFormats = nParams?&bindData->fmt[0]:NULL;
		if(!PQsendQueryPrepared(
			_pgcon,
			prid.c_str(),
			nParams,
			paramValues,
			paramLengths,
			paramFormats,
			1))
		{
			ELOG(__FUNCTION__<<", "<<PQerrorMessage(_pgcon));
			setResult(res);
			return;
		};

		processSingle(res);
	}

	//////////////////////////////////////////////////////////////////////////
	void ConnectionHolder::runQueryWithPrepare_f(async::Future<pgc::Result> res, StatementPtr s, BindDataPtr bindData)
	{
		assert(_mtxProcess.isLocked());
		assert(!_now.is_not_a_date_time());

		if(hasPrepared(s))
		{
			runQueryPrepared_f(res, getPrid(s), bindData);
			return;
		}

		processQueryWithPrepare(res, s, bindData);
	}

	//////////////////////////////////////////////////////////////////////////
	bool ConnectionHolder::deallocateOne(const std::string &prid, async::Future<pgc::Result> res)
	{
		async::Future<pgc::Result> r;
		runQuery_f(r, "DEALLOCATE "+prid);
		if(ersCommandOk != r.data().status())
		{
			const char * errCode = r.data().errorCode();
			if(errCode && !strcmp("26000", errCode))
			{
				//ERROR:  prepared statement "XXX" does not exist
			}
			else
			{
				//другая ошибка - фатально
				ELOG(__FUNCTION__<<", "<<PQerrorMessage(_pgcon));
				_prepareds.clear();
				setResult(res);
				return false;
			}
		}

		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	void ConnectionHolder::runEndWork_f(async::Future<pgc::Result> res)
	{
		assert(_mtxProcess.isLocked());
		assert(!_now.is_not_a_date_time());

		//удаление по устареванию
		TPrepareds::nth_index<1>::type &accessIndex = _prepareds.get<1>();
		while(
			!accessIndex.empty() &&
			(
				accessIndex.size() > _max)		)
		{
			if(!deallocateOne(accessIndex.begin()->_prid, res))
			{
				return;
			}
			accessIndex.erase(accessIndex.begin());
		}

		//удаление по времени жизни
		posix_time::ptime boundATime = _now - posix_time::milliseconds(_timeout);
		TPrepareds::nth_index<2>::type &createIndex = _prepareds.get<2>();
		while(
			!createIndex.empty() &&
			(
				boundATime > createIndex.begin()->_createTime)		)
		{
			if(!deallocateOne(createIndex.begin()->_prid, res))
			{
				return;
			}
			createIndex.erase(createIndex.begin());
		}

		_now = posix_time::ptime();
		assert(_mtxProcess.isLocked());
		assert(_pgcon);
		setResult(res, true);
	}


	//////////////////////////////////////////////////////////////////////////
	async::Future<system::error_code> ConnectionHolder::send0()
	{
		async::Future<system::error_code> h;
		_sock.async_send(asio::null_buffers(), async::bridge(boost::bind(h,_1)));
		return h;
	}

	//////////////////////////////////////////////////////////////////////////
	async::Future<system::error_code> ConnectionHolder::recv0()
	{
		async::Future<system::error_code> h;
		_sock.async_receive(asio::null_buffers(), async::bridge(boost::bind(h,_1)));
		return h;
	}


	//////////////////////////////////////////////////////////////////////////
	ConnectionHolder::ConnectionHolder(DbPtr db, PGconn *pgcon)
		: _db(db)
		, _pgcon(pgcon)
		, _sock(async::io(), PGSockProtocol(sockFamily(PQsocket(_pgcon)), sockType(PQsocket(_pgcon)), IPPROTO_TCP), PQsocket(_pgcon))
		, _integerDatetimes(false)
	{
		assert(_pgcon);
		PQsetnonblocking(_pgcon, 0);
	}

	//////////////////////////////////////////////////////////////////////////
	ConnectionHolder::~ConnectionHolder()
	{
		assert(!_mtxProcess.isLocked());
		//assert(_now.is_not_a_date_time());

		//assert(ecsOk != status());
		close();
	}

	//////////////////////////////////////////////////////////////////////////
	PGconn *ConnectionHolder::pgcon()
	{
		return _pgcon;
	}

	//////////////////////////////////////////////////////////////////////////
	void ConnectionHolder::onOpen()
	{
		const char *csz = PQparameterStatus(_pgcon, "integer_datetimes");

		if(csz && !strcmp("on", csz))
		{
			_integerDatetimes = true;
		}
		else
		{
			_integerDatetimes = false;
		}

		PQsetClientEncoding(_pgcon, "UTF-8");
		PQsetnonblocking(_pgcon, 0);
	}

	//////////////////////////////////////////////////////////////////////////
	EConnectionStatus ConnectionHolder::status()
	{
		if(!_pgcon)
		{
			return ecsNull;
		}
		ConnStatusType status = PQstatus(_pgcon);

		switch(status)
		{
		case CONNECTION_OK:
		case CONNECTION_MADE:
			return ecsOk;
		default:
			break;
		}
		return ecsLost;
	}

	//////////////////////////////////////////////////////////////////////////
	void ConnectionHolder::close()
	{
		if(_pgcon)
		{
			//assert(_now.is_not_a_date_time());
			assert(_requests.empty());

			PQfinish(_pgcon);
			_pgcon = NULL;
			system::error_code ec;
// 			_sock.close(ec);
// 			if(ec)
// 			{
// 				std::cerr<<ec.message()<<std::endl;
// 			}
			_integerDatetimes = false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool ConnectionHolder::integerDatetimes()
	{
		return _integerDatetimes;
	}

	//////////////////////////////////////////////////////////////////////////
	void ConnectionHolder::pushRequest(const SRequestPtr &request)
	{
		mutex::scoped_lock sl(_mtx);
		assert(_pgcon);

		_requests.push_back(request);
		if(_requests.size()<2)
		{
			async::spawn(bind(&ConnectionHolder::processRequest_f, shared_from_this()));
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void ConnectionHolder::runQuery(async::Future<pgc::Result> res, const std::string &sql, BindDataPtr bindData)
	{
		assert(sql.size());
		pushRequest(SRequestPtr(new SRequestQuery(res, sql, bindData)));
	}

	//////////////////////////////////////////////////////////////////////////
	void ConnectionHolder::runQueryWithPrepare(async::Future<pgc::Result> res, pgc::Statement s, BindDataPtr bindData)
	{
		assert(s);
		assert(s.getSql().size());

		pushRequest(SRequestPtr(new SRequestQueryWithPrepare(res, utils::ImplAccess<pgc::Statement>(s), bindData)));
	}

	//////////////////////////////////////////////////////////////////////////
	void ConnectionHolder::beginWork()
	{
		assert(_pgcon);
		//assert(!_mtxProcess.isLocked());//может быть в холостом процессе
		assert(_now.is_not_a_date_time());
		_now = posix_time::microsec_clock::local_time();
	}

	//////////////////////////////////////////////////////////////////////////
	void ConnectionHolder::endWork()
	{
		async::Future<pgc::Result> res;
		pushRequest(SRequestPtr(new SRequestEndWork(res)));
		res.wait();
		_db->unwork(shared_from_this());
	}
}}
