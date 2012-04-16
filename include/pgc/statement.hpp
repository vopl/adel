#ifndef _PGC_STATEMENT_HPP_
#define _PGC_STATEMENT_HPP_

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

namespace pgc
{
	//////////////////////////////////////////////////////////////////////////
	namespace impl
	{
		class Statement;
		typedef boost::shared_ptr<Statement> StatementPtr;
	}

	//////////////////////////////////////////////////////////////////////////
	class Statement
	{
	protected:
		typedef impl::StatementPtr ImplPtr;
		ImplPtr	_impl;

	public:
		Statement();
		Statement(const char *sql);
		Statement(const std::string &sql);

		~Statement();

		operator bool() const;
		bool operator!() const;

		const std::string &getSql();
	};
}
#endif
