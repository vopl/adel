#include "pch.hpp"
#include "net/http/headerValue.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_string.hpp>
#include <boost/spirit/include/qi_char.hpp>

#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_container.hpp>


namespace net { namespace http
{
	namespace qi = boost::spirit::qi;
	using namespace qi;
	namespace px = boost::phoenix;


	//////////////////////////////////////////////////////////////////////
	template <>
	bool HeaderValue<Date>::parse(const Message::Segment &src)
	{
		assert(0);
		return false;
	}


}}
