#include "pch.hpp"
#include "net/http/headerValue.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_string.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_symbols.hpp>
//#include <boost/spirit/include/qi_int.hpp>
#include <boost/spirit/include/qi_uint.hpp>
#include <boost/spirit/include/qi_omit.hpp>

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
	/*
	   HTTP-date    = rfc1123-date | rfc850-date | asctime-date
       rfc1123-date = wkday "," SP date1 SP time SP "GMT"
       rfc850-date  = weekday "," SP date2 SP time SP "GMT"
       asctime-date = wkday SP date3 SP time SP 4DIGIT
       date1        = 2DIGIT SP month SP 4DIGIT
                      ; day month year (e.g., 02 Jun 1982)
       date2        = 2DIGIT "-" month "-" 2DIGIT
                      ; day-month-year (e.g., 02-Jun-82)
       date3        = month SP ( 2DIGIT | ( SP 1DIGIT ))
                      ; month day (e.g., Jun  2)
       time         = 2DIGIT ":" 2DIGIT ":" 2DIGIT
                      ; 00:00:00 - 23:59:59
       wkday        = "Mon" | "Tue" | "Wed"
                    | "Thu" | "Fri" | "Sat" | "Sun"
       weekday      = "Monday" | "Tuesday" | "Wednesday"
                    | "Thursday" | "Friday" | "Saturday" | "Sunday"
       month        = "Jan" | "Feb" | "Mar" | "Apr"
                    | "May" | "Jun" | "Jul" | "Aug"
                    | "Sep" | "Oct" | "Nov" | "Dec"
	*/

	namespace
	{
		symbols<char, int> wkdayInit()
		{
			symbols<char, int> res;
			res.add("Sun", 0);
			res.add("Mon", 1);
			res.add("Tue", 2);
			res.add("Wed", 3);
			res.add("Thu", 4);
			res.add("Fri", 5);
			res.add("Sat", 6);
			return res;
		}
		static const symbols<char, int> wkday = wkdayInit();

		symbols<char, int> weekdayInit()
		{
			symbols<char, int> res;
			res.add("Sunday", 0);
			res.add("Monday", 1);
			res.add("Tuesday", 2);
			res.add("Wednesday", 3);
			res.add("Thursday", 4);
			res.add("Friday", 5);
			res.add("Saturday", 6);
			return res;
		}
		static const symbols<char, int> weekday = weekdayInit();

		symbols<char, int> monthInit()
		{
			symbols<char, int> res;
			res.add("Jan", 0);
			res.add("Feb", 1);
			res.add("Mar", 2);
			res.add("Apr", 3);
			res.add("May", 4);
			res.add("Jun", 5);
			res.add("Jul", 6);
			res.add("Aug", 7);
			res.add("Sep", 8);
			res.add("Oct", 9);
			res.add("Nov", 10);
			res.add("Dec", 11);
			return res;
		}
		static const symbols<char, int> month = monthInit();

	}

	//////////////////////////////////////////////////////////////////////
	template <>
	bool HeaderValue<Date>::parse(const Message::Segment &src)
	{
		struct tm stm = {};

		rule<MessageIterator> time = 
			uint_[px::ref(stm.tm_hour) = qi::_1] >>
			':' >>
			uint_[px::ref(stm.tm_min) = qi::_1] >>
			':' >>
			uint_[px::ref(stm.tm_sec) = qi::_1];

		//rfc1123-date = wkday "," SP date1 SP time SP "GMT"
		rule <MessageIterator> rfc1123 = 
			wkday[px::ref(stm.tm_wday) = qi::_1] >>

			',' >> 
			
			+lit(' ') >> uint_[px::ref(stm.tm_mday) = qi::_1] >>
			+lit(' ') >> month[px::ref(stm.tm_mon) = qi::_1] >>
			+lit(' ') >> uint_[px::ref(stm.tm_year) = qi::_1 - 1900] >>


			+lit(' ') >> time >>
			" GMT";

		//rfc850-date  = weekday "," SP date2 SP time SP "GMT"
		rule<MessageIterator> rfc850 = 
			weekday[px::ref(stm.tm_wday) = qi::_1] >>

			',' >> 
			
			+lit(' ') >> uint_[px::ref(stm.tm_mday) = qi::_1] >>
			lit('-') >> month[px::ref(stm.tm_mon) = qi::_1] >>
			lit('-') >> uint_[px::ref(stm.tm_year) = qi::_1 + 100] >>

			+lit(' ') >> time >>
			" GMT";

		//	asctime-date = wkday SP date3 SP time SP 4DIGIT
		//	date3        = month SP ( 2DIGIT | ( SP 1DIGIT ))
		//	; month day (e.g., Jun  2)
		rule<MessageIterator> asctime = 
			weekday[px::ref(stm.tm_wday) = qi::_1] >>

			+lit(' ') >> month[px::ref(stm.tm_mon) = qi::_1] >>
			+lit(' ') >> uint_[px::ref(stm.tm_mday) = qi::_1] >>
			+lit(' ') >> time >>
			+lit(' ') >> uint_[px::ref(stm.tm_year) = qi::_1-1900];


		bool res = qi::parse(src.begin(), src.end(), rfc1123|rfc850|asctime);

		if(!res)
		{
			return false;
		}
#ifdef _MSC_VER
		_value = _mkgmtime(&stm);
#else
		_value = timegm(&stm);
#endif
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
	namespace
	{
		rule<MessageIterator, double()> itemWeightInit()
		{
			rule<MessageIterator, double()> res = 
				omit[
					*char_(' ') >> lit(';') >>
					*char_(' ') >> char_('q') >>
					*char_(' ') >> char_('=') >>
					*char_(' ')
				] >> double_;

			return res;
		}
		static const rule<MessageIterator, double()> itemWeight = itemWeightInit();
	}
	//////////////////////////////////////////////////////////////////////
	
	template <>
	bool HeaderValue<TransferEncoding>::parse(const Message::Segment &src)
	{
		double w;
		ETransferEncoding ete;

		_value = 0;
		return qi::parse(src.begin(), src.end(),
			+(
				eps[px::ref(ete) = ete_unknown][px::ref(w) = 1.0] >>
				(
					lit("deflate")	[px::ref(ete) = ete_deflate]|
					lit("gzip")		[px::ref(ete) = ete_gzip]|
					lit("compress")	[px::ref(ete) = ete_compress]|
					lit("chunked")	[px::ref(ete) = ete_chunked]|
					lit("identity")	[px::ref(ete) = ete_identity]|
					lit("*")		[px::ref(ete) = ete_any]
				) >>
				(
					(
						*char_(' ') >> 
						(itemWeight[px::ref(w) = qi::_1] | eps) >>
						*char_(" ,")
					) | eps
				) >>
				eps[px::if_(px::ref(w) > 0)[px::ref(_value) |= px::ref(ete)].else_[px::ref(_value) &= ~px::ref(ete)]]
			)
		);
	}

	//////////////////////////////////////////////////////////////////////
	template <>
	bool HeaderValue<ContentEncoding>::parse(const Message::Segment &src)
	{
		double w;
		EContentEncoding ece;

		_value = 0;
		return qi::parse(src.begin(), src.end(),
			+(
				eps[px::ref(ece) = ece_unknown][px::ref(w) = 1.0] >>
				(
					lit("deflate")	[px::ref(ece) = ece_deflate]|
					lit("gzip")		[px::ref(ece) = ece_gzip]|
					lit("compress")	[px::ref(ece) = ece_compress]|
					lit("identity")	[px::ref(ece) = ece_identity]|
					lit("*")		[px::ref(ece) = ece_any]
				) >>
				(
					(
						*char_(' ') >> 
						(itemWeight[px::ref(w) = qi::_1] | eps) >>
						*char_(" ,")
					) | eps
				) >>
				eps[px::if_(px::ref(w) > 0)[px::ref(_value) |= px::ref(ece)].else_[px::ref(_value) &= ~px::ref(ece)]]
			)
		);
	}

}}
