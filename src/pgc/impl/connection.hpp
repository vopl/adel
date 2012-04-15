#ifndef _PGC_IMPL_CONNECTION_HPP_
#define _PGC_IMPL_CONNECTION_HPP_

#include "pgc/impl/connectionHolder.hpp"


namespace pgc { namespace impl
{
	//////////////////////////////////////////////////////////////////////////
	class Connection
		: public enable_shared_from_this<Connection>
	{
	public:
		ConnectionHolderPtr _holder;

	public:
		Connection(ConnectionHolderPtr holder);
		~Connection();


	};
	typedef shared_ptr<Connection> ConnectionPtr;
}}

#endif
