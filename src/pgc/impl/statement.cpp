#include "pch.hpp"
#include "pgc/impl/statement.hpp"

namespace pgc { namespace impl
{
	//////////////////////////////////////////////////////////////////////////
	Statement::Statement(const char *sql)
		: _sql(sql)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	Statement::Statement(const std::string &sql)
		: _sql(sql)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	Statement::~Statement()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	const std::string &Statement::getSql()
	{
		return _sql;
	}

	//////////////////////////////////////////////////////////////////////////
	const std::string &Statement::getPreparedId()
	{
		return _preparedId;
	}

	//////////////////////////////////////////////////////////////////////////
	void Statement::setPreparedId(const char *csz)
	{
		_preparedId = csz;
	}

}}
