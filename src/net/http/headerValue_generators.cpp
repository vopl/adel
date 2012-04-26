#include "pch.hpp"
#include "net/http/headerValue.hpp"

#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/karma_string.hpp>
#include <boost/spirit/include/karma_char.hpp>

#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_container.hpp>


namespace net { namespace http
{
	namespace karma = boost::spirit::karma;
	using namespace karma;
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
		symbols<int, char const*> wkdayInit()
		{
			symbols<int, char const*> wkday;
			wkday.add(0, "Sun");
			wkday.add(1, "Mon");
			wkday.add(2, "Tue");
			wkday.add(3, "Wed");
			wkday.add(4, "Thu");
			wkday.add(5, "Fri");
			wkday.add(6, "Sat");
			return wkday;
		}
		static const symbols<int, char const*> wkday = wkdayInit();

		symbols<int, char const*> monthInit()
		{
			symbols<int, char const*> month;
			month.add(0, "Jan");
			month.add(1, "Feb");
			month.add(2, "Mar");
			month.add(3, "Apr");
			month.add(4, "May");
			month.add(5, "Jun");
			month.add(6, "Jul");
			month.add(7, "Aug");
			month.add(8, "Sep");
			month.add(9, "Oct");
			month.add(10, "Nov");
			month.add(11, "Dec");
			return month;
		}
		static const symbols<int, char const*> month = wkdayInit();

	}

	//////////////////////////////////////////////////////////////////////
	template <>
	template <class Iterator>
	bool HeaderValue<Date>::generate(Iterator &outIter) const
	{
		struct tm stm;
		gmtime_r(&_value, &stm);

		return karma::generate(outIter,

			wkday[karma::_1 = stm.tm_wday] <<

			", " <<

			right_align(2, '0')[uint_[karma::_1 = (unsigned)stm.tm_mday]] <<
			' ' <<
			month[karma::_1 = stm.tm_mon] <<
			' ' <<
			uint_[karma::_1 = (unsigned)(stm.tm_year+1900)] <<


			' ' <<

			right_align(2, '0')[uint_[karma::_1 = (unsigned)stm.tm_hour]] <<
			':' <<
			right_align(2, '0')[uint_[karma::_1 = (unsigned)stm.tm_min]] <<
			':' <<
			right_align(2, '0')[uint_[karma::_1 = (unsigned)stm.tm_sec]] <<

			" GMT");
	}
	template bool HeaderValue<Date>::generate<Message::Iterator>(Message::Iterator &outIter) const;
	template bool HeaderValue<Date>::generate<std::back_insert_iterator<std::string> >(std::back_insert_iterator<std::string> &outIter) const;


}}
