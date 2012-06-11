#ifndef _PGC_CONNECTION_HPP_
#define _PGC_CONNECTION_HPP_

#include "pgc/statement.hpp"
#include "pgc/result.hpp"
#include "utils/variant.hpp"
#include "async/future.hpp"

namespace pgc
{
	enum EConnectionStatus
	{
		ecsNull,
		ecsLost,
		ecsOk,
	};


	//////////////////////////////////////////////////////////////////////////
	namespace impl
	{
		class Connection;
		typedef boost::shared_ptr<Connection> ConnectionPtr;
	}

	//////////////////////////////////////////////////////////////////////////
	class Connection
	{
	protected:
		typedef impl::ConnectionPtr ImplPtr;
		ImplPtr	_impl;

	public:
		Connection();
		~Connection();

		operator bool() const;
		bool operator!() const;
		void reset();

		async::Future<Result> query(const char *sql);
		async::Future<Result> query(const char *sql, const utils::Variant &data);
		async::Future<Result> query(const std::string &sql);
		async::Future<Result> query(const std::string &sql, const utils::Variant &data);
		async::Future<Result> query(Statement s, bool withPrepare=true);
		async::Future<Result> query(Statement s, const utils::Variant &data, bool withPrepare=true);

		EConnectionStatus status();
	};
}
#endif
