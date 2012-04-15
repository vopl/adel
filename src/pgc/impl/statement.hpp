#ifndef _PGC_IMPL_STATEMENT_HPP_
#define _PGC_IMPL_STATEMENT_HPP_

#include <boost/enable_shared_from_this.hpp>

namespace pgc { namespace impl
{
	using namespace boost;

	class Statement
		: public enable_shared_from_this<Statement>
	{
		std::string _sql;
		std::string _preparedId;

	public:
		Statement(const char *sql);
		Statement(const std::string &sql);
		~Statement();

		const std::string &getSql();

		virtual const std::string &getPreparedId();
		virtual void setPreparedId(const char *csz);
	};

	typedef shared_ptr<Statement> StatementPtr;
	typedef weak_ptr<Statement> StatementWtr;
}}

#endif

