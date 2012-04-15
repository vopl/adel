#include "pgc/impl/connection.hpp"

namespace pgc { namespace impl
{
	//////////////////////////////////////////////////////////////////////////
	Connection::Connection(ConnectionHolderPtr holder)
		: _holder(holder)
	{
		_holder->beginWork();
	}

	//////////////////////////////////////////////////////////////////////////
	Connection::~Connection()
	{
		_holder->endWork();
	}
}}
