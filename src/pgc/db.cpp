#include "pch.hpp"
#include "pgc/db.hpp"
#include "pgc/impl/db.hpp"

namespace pgc
{
	//////////////////////////////////////////////////////////////////////////
	Db::Db()
		: _impl()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	Db::Db(const char *conninfo,
		size_t maxConnections,
		boost::function<void (size_t)> connectionMade,
		boost::function<void (size_t)> connectionLost)
		: _impl(new impl::Db(conninfo, maxConnections<1?1:maxConnections, connectionMade, connectionLost))
	{

	}

	//////////////////////////////////////////////////////////////////////////
	Db::~Db()
	{
		_impl.reset();
	}

	//////////////////////////////////////////////////////////////////////////
	Db::operator bool() const
	{
		return _impl?true:false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool Db::operator!() const
	{
		return _impl?false:true;
	}

	//////////////////////////////////////////////////////////////////////////
	void Db::reset()
	{
		if(_impl)
		{
			_impl->reset();
			_impl.reset();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	async::Future<Connection> Db::allocConnection()
	{
		return _impl->allocConnection();
	}
}
