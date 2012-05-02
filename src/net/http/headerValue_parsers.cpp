#include "pch.hpp"
#include "net/http/headerValue.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_string.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_symbols.hpp>
//#include <boost/spirit/include/qi_int.hpp>
#include <boost/spirit/include/qi_uint.hpp>

#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_container.hpp>
#include <time.h>


namespace net { namespace http
{
	namespace qi = boost::spirit::qi;
	using namespace qi;
	namespace px = boost::phoenix;


	//////////////////////////////////////////////////////////////////////
	//RFC 822, updated by RFC 1123
	/*
		rfc1123-date = wkday "," SP date1 SP time SP "GMT"

		date1        = 2DIGIT SP month SP 4DIGIT
					  ; day month year (e.g., 02 Jun 1982)

		time         = 2DIGIT ":" 2DIGIT ":" 2DIGIT
					  ; 00:00:00 - 23:59:59

		wkday        = "Mon" | "Tue" | "Wed"
					| "Thu" | "Fri" | "Sat" | "Sun"

		month        = "Jan" | "Feb" | "Mar" | "Apr"
					| "May" | "Jun" | "Jul" | "Aug"
					| "Sep" | "Oct" | "Nov" | "Dec"
	*/

	namespace
	{
		symbols<char, int> wkdayInit()
		{
			symbols<char, int> wkday;
			wkday.add("Sun", 0);
			wkday.add("Mon", 1);
			wkday.add("Tue", 2);
			wkday.add("Wed", 3);
			wkday.add("Thu", 4);
			wkday.add("Fri", 5);
			wkday.add("Sat", 6);
			return wkday;
		}
		static const symbols<char, int> wkday = wkdayInit();

		symbols<char, int> monthInit()
		{
			symbols<char, int> month;
			month.add("Jan", 0);
			month.add("Feb", 1);
			month.add("Mar", 2);
			month.add("Apr", 3);
			month.add("May", 4);
			month.add("Jun", 5);
			month.add("Jul", 6);
			month.add("Aug", 7);
			month.add("Sep", 8);
			month.add("Oct", 9);
			month.add("Nov", 10);
			month.add("Dec", 11);
			return month;
		}
		static const symbols<char, int> month = monthInit();

	}

	//////////////////////////////////////////////////////////////////////
	template <>
	bool HeaderValue<Date>::parse(const Message::Segment &src)
	{
		struct tm stm = {};

		bool res = qi::parse(src.begin(), src.end(),
			wkday[px::ref(stm.tm_wday) = qi::_1] >>

			',' >> +lit(' ') >>

			uint_[px::ref(stm.tm_mday) = qi::_1] >>
			+lit(' ') >>
			month[px::ref(stm.tm_mon) = qi::_1] >>
			+lit(' ') >>
			uint_[px::ref(stm.tm_year) = qi::_1 - 1900] >>


			+lit(' ') >>

			uint_[px::ref(stm.tm_hour) = qi::_1] >>
			':' >>
			uint_[px::ref(stm.tm_min) = qi::_1] >>
			':' >>
			uint_[px::ref(stm.tm_sec) = qi::_1] >>

			" GMT"
			);

		if(!res)
		{
			return false;
		}

		_value = timegm(&stm);
		return true;
	}

	//////////////////////////////////////////////////////////////////////
	template <>
	bool HeaderValue<Unsigned>::parse(const Message::Segment &src)
	{
		return qi::parse(src.begin(), src.end(), uint_parser<Value, 10>()[px::ref(_value) = qi::_1]);
	}

	//////////////////////////////////////////////////////////////////////
	namespace
	{
		symbols<char, EConnection> connectionInit()
		{
			symbols<char, EConnection> res;
			res.add("close", ec_close);
			res.add("keep-alive", ec_keepAlive);
			return res;
		}
		static const symbols<char, EConnection> connection = connectionInit();
	}
	//////////////////////////////////////////////////////////////////////
	template <>
	bool HeaderValue<Connection>::parse(const Message::Segment &src)
	{
		return qi::parse(src.begin(), src.end(), boost::spirit::ascii::no_case[connection[px::ref(_value) = qi::_1]]);
	}

	//////////////////////////////////////////////////////////////////////
	template <>
	bool HeaderValue<TransferEncoding>::parse(const Message::Segment &src)
	{
		return qi::parse(src.begin(), src.end(),
			(
				lit("deflate")	[px::ref(_value) |= ete_deflate]|
				lit("gzip")		[px::ref(_value) |= ete_gzip]|
				lit("compress")	[px::ref(_value) |= ete_compress]|
				lit("chunked")	[px::ref(_value) |= ete_chunked]|
				lit("identity")	[px::ref(_value) |= ete_identity]|
				lit("*")		[px::ref(_value)  = ete_any]
			) >>
			(
				(
					*char_(' ') >> lit(';') >>
					*char_(' ') >> char_('q') >>
					*char_(' ') >> char_('=') >>
					*char_(' ') >> +char_("0-9\\.") >>
					*char_(" ,")
				) | eps
			)
		);
	}

	//////////////////////////////////////////////////////////////////////
	template <>
	bool HeaderValue<ContentEncoding>::parse(const Message::Segment &src)
	{
		assert(0);
		return false;
	}

}}
