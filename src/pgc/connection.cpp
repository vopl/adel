#include "pch.hpp"
#include "pgc/connection.hpp"
#include "pgc/impl/bindData.hpp"
#include "pgc/impl/connection.hpp"

namespace pgc
{
	//////////////////////////////////////////////////////////////////////////
	Connection::Connection()
		: _impl()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	Connection::~Connection()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	Connection::operator bool() const
	{
		return _impl?true:false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool Connection::operator!() const
	{
		return _impl?false:true;
	}

	//////////////////////////////////////////////////////////////////////////
	void Connection::reset()
	{
		_impl.reset();
	}

	//////////////////////////////////////////////////////////////////////////
	async::Future<Result> Connection::query(const char *sql)
	{
		return query(std::string(sql));
	}

	//////////////////////////////////////////////////////////////////////////
	async::Future<Result> Connection::query(const char *sql, const utils::Variant &data)
	{
		return query(std::string(sql), data);
	}

	//////////////////////////////////////////////////////////////////////////
	async::Future<Result> Connection::query(const std::string &sql)
	{
		async::Future<Result> res;
		_impl->_holder->runQuery(res, sql, impl::BindDataPtr());
		return res;
	}

	//////////////////////////////////////////////////////////////////////////
	async::Future<Result> Connection::query(const std::string &sql, const utils::Variant &data)
	{
		async::Future<Result> res;
		_impl->_holder->runQuery(res, sql, impl::BindDataPtr(new impl::BindData(data, _impl->_holder)));
		return res;
	}

	//////////////////////////////////////////////////////////////////////////
	async::Future<Result> Connection::query(Statement s, bool withPrepare)
	{
		async::Future<Result> res;
		if(withPrepare)
		{
			_impl->_holder->runQueryWithPrepare(res, s, impl::BindDataPtr());
		}
		else
		{
			_impl->_holder->runQuery(res, s.getSql(), impl::BindDataPtr());
		}
		return res;
	}

	//////////////////////////////////////////////////////////////////////////
	async::Future<Result> Connection::query(Statement s, const utils::Variant &data, bool withPrepare)
	{
		async::Future<Result> res;

		impl::BindDataPtr bindData(new impl::BindData(data, _impl->_holder));
		if(withPrepare)
		{
			_impl->_holder->runQueryWithPrepare(res, s, bindData);
		}
		else
		{
			_impl->_holder->runQuery(res, s.getSql(), bindData);
		}
		return res;
	}

	//////////////////////////////////////////////////////////////////////////
	EConnectionStatus Connection::status()
	{
		return _impl?_impl->_holder->status():ecsNull;
	}

}
