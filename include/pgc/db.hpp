#ifndef _PGC_DB_HPP_
#define _PGC_DB_HPP_

#include "async/service.hpp"
#include "pgc/connection.hpp"
#include "async/future.hpp"

namespace pgc
{
	//////////////////////////////////////////////////////////////////////////
	namespace impl
	{
		class Db;
		typedef boost::shared_ptr<Db> DbPtr;
	}

	//////////////////////////////////////////////////////////////////////////
	class Db
	{
	protected:
		typedef impl::DbPtr ImplPtr;
		ImplPtr	_impl;

	public:
		Db();
		Db(const char *conninfo,
			size_t maxConnections=10,
			boost::function<void (size_t)> onConnectionMade = boost::function<void (size_t)>(),
			boost::function<void (size_t)> onConnectionLost = boost::function<void (size_t)>());

		~Db();

		operator bool() const;
		bool operator!() const;
		void reset();

		async::Future<Connection> allocConnection();
	};
}
#endif
