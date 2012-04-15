#ifndef _UTILS_VARIANTLOADSCOPE_HPP_
#define _UTILS_VARIANTLOADSCOPE_HPP_

#include "utils/variant.hpp"
#include <boost/spirit/home/support/info.hpp>
#include <boost/filesystem.hpp>

namespace utils
{
	class VariantLoadScope
	{
		std::deque<Variant>			_stack;

		boost::filesystem::path _fileName;
		const char		*_first;
		const char		*_last;
		std::string		*_errors;
		bool			_errorWas;


		std::string		_mapKey;

	public:
		//////////////////////////////////////////////////////////////////////////
		struct SDate
		{
			unsigned _year;
			unsigned _month;
			unsigned _day;

			SDate()
				: _year(0)
				, _month(0)
				, _day(0)
			{}
		};

		//////////////////////////////////////////////////////////////////////////
		struct STime
		{
			int _hour;
			unsigned _minute;
			unsigned _second;
			unsigned _microsec;

			STime()
				: _hour(0)
				, _minute(0)
				, _second(0)
				, _microsec(0)
			{}
		};

		//////////////////////////////////////////////////////////////////////////
		struct SDateTime
			: SDate, STime
		{
			SDateTime &operator=(const SDate &d)
			{
				*(SDate *)this = d;
				return *this;
			}
			SDateTime &operator=(const STime &t)
			{
				*(STime *)this = t;
				return *this;
			}
		};


	public:
		VariantLoadScope(
			boost::filesystem::path fileName, 
			const char *first,
			const char *last,
			std::string *errors);
		~VariantLoadScope();

		Variant &getValue();
		bool errorWas();

		void error(const char *first, const char *last, const char *errorPos, const boost::spirit::info &what);
		
		void set_null();
		void set_string(const std::string &content);
		void set_double(const double &content);
		void set_float(const float &content);
		void set_integer(const std::string &content);
		void set_bool(bool v);
		void set_uuid(const std::string &content);

		void set_date(const SDate &d);
		void set_time(const STime &t);
		void set_datetime(const SDateTime &t);

		void array_start();
		void array_push();
		void array_stop();

		void map_start();
		void map_key(const std::string &content);
		void map_push();
		void map_stop();


		bool do_include(const std::string &path);

	};
}

#endif
