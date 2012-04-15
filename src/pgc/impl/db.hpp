#ifndef _PGC_IMPL_DB_HPP_
#define _PGC_IMPL_DB_HPP_

#include "pgc/db.hpp"
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread/mutex.hpp>
#include <set>
#include <deque>
#include "pgc/connection.hpp"
#include "connectionHolder.hpp"

namespace pgc { namespace impl
{
	using namespace boost;

	class Db
		: public enable_shared_from_this<Db>
	{
		mutex					_mtx;
		std::string				_conninfo;
		size_t					_maxConnections;
		function<void (size_t)> _onConnectionMade;
		function<void (size_t)> _onConnectionLost;

	private:
		typedef std::set<ConnectionHolderPtr> TSConnectins;
		TSConnectins							_startConnections;
		TSConnectins							_readyConnections;
		TSConnectins							_workConnections;
		std::deque<async::Future<pgc::Connection> >	_waiters;

		typedef asio::deadline_timer Timeout;
		typedef boost::shared_ptr<Timeout> TimeoutPtr;
		TimeoutPtr	_timeout;

	private:
		void balanceConnections();

		void makeConnection();
		void makeConnection_poll(ConnectionHolderPtr pcw);

	private:
		void onRebalanceTimer();

	public:
		void unwork(ConnectionHolderPtr pcw);

	public:
		Db(const char *conninfo,
			size_t maxConnections,
			function<void (size_t)> connectionMade,
			function<void (size_t)> connectionLost);
		~Db();

		async::Future<pgc::Connection> allocConnection();

		void reset();
	};
}}

#endif

