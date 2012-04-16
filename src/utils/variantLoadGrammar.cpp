#include "pch.hpp"
#include "variantLoadGrammar.hpp"

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/date_time.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

namespace phx = boost::phoenix;

BOOST_FUSION_ADAPT_STRUCT(
	utils::VariantLoadScope::SDate,
	(unsigned, _year)
	(unsigned, _month)
	(unsigned, _day)
);


BOOST_FUSION_ADAPT_STRUCT(
	utils::VariantLoadScope::STime,
	(int, _hour)
	(unsigned, _minute)
	(unsigned, _second)
	(unsigned, _microsec)
);

namespace utils
{
	using namespace qi::labels;
	using namespace qi;

	//////////////////////////////////////////////////////////////////////////
	struct Checker
	{
		//////////////////////////////////////////////////////////////////////////
		template <typename Context>
		void operator()(const VariantLoadScope::SDate &d, Context& context, bool& pass) const
		{
			try
			{
				boost::gregorian::date gd(d._year, d._month, d._day);
				pass = !gd.is_special();
			}
			catch (...)
			{
				pass = false;
			}
		}

		//////////////////////////////////////////////////////////////////////////
		template <typename Context>
		void operator()(const VariantLoadScope::STime &t, Context& context, bool& pass) const
		{
			try
			{

				boost::posix_time::time_duration ptd(
					t._hour,
					t._minute,
					t._second);
				ptd += boost::posix_time::microseconds(t._microsec);

				pass = !ptd.is_not_a_date_time();
			}
			catch (...)
			{
				pass = false;
			}
		}

		//////////////////////////////////////////////////////////////////////////
		template <typename Context>
		void operator()(const VariantLoadScope::SDateTime &dt, Context& context, bool& pass) const
		{
			try
			{

				boost::gregorian::date gd(dt._year, dt._month, dt._day);
				boost::posix_time::time_duration ptd(
					dt._hour,
					dt._minute,
					dt._second);
				ptd += boost::posix_time::microseconds(dt._microsec);

				pass = !boost::posix_time::ptime(gd, ptd).is_not_a_date_time();
			}
			catch (...)
			{
				pass = false;
			}
		}

	};

	//////////////////////////////////////////////////////////////////////////
	struct CheckerInclude
	{
		VariantLoadScope &_scope;
		CheckerInclude(VariantLoadScope &scope)
			: _scope(scope)
		{
		}

		//////////////////////////////////////////////////////////////////////////
		template <typename Context>
		void operator()(const std::string &path, Context& context, bool& pass) const
		{
			pass = _scope.do_include(path);
		}
	};


	//////////////////////////////////////////////////////////////////////////
	VariantLoadGrammar::VariantLoadGrammar(VariantLoadScope &scope)
		: base_type(_start, "variant")
		, _scope(scope)
	{

		//////////////////////////////////////////////////////////////////////////
		_scalar.name("scalar");
		_scalar =
			_null				[phx::bind(&VariantLoadScope::set_null, _scope)] |
			_string				[phx::bind(&VariantLoadScope::set_string, _scope, spirit::_1)] |
			_uuid				[phx::bind(&VariantLoadScope::set_uuid, _scope, spirit::_1)] |
			_bool				[phx::bind(&VariantLoadScope::set_bool, _scope, spirit::_1)] |

			_dateTimeScope		[phx::bind(&VariantLoadScope::set_datetime, _scope, spirit::_1)] |
			_dateScope			[phx::bind(&VariantLoadScope::set_date, _scope, spirit::_1)] |
			_timeScope			[phx::bind(&VariantLoadScope::set_time, _scope, spirit::_1)] |

			_float				[phx::bind(&VariantLoadScope::set_float, _scope, spirit::_1)] |
			_double				[phx::bind(&VariantLoadScope::set_double, _scope, spirit::_1)] |
			_integer			[phx::bind(&VariantLoadScope::set_integer, _scope, spirit::_1)]
			;








		//////////////////////////////////////////////////////////////////////////
		_null.name("null");
		_null = lit("null");


		//////////////////////////////////////////////////////////////////////////
		_string.name("string");
		_string =
			(
				lit('"') >
				*((lit('\\') >> char_('"')) | (char_ - char_('"'))) >
 				lit('"')
			)|
			(
				lit('\'') >
				*((lit('\\') >> char_('\'')) | (char_ - char_('\''))) >
 				lit('\'')
			);

		//////////////////////////////////////////////////////////////////////////
		_float.name("float");
		_float =
			real_parser<float, strict_real_policies<float> >() >>lit('f');

		//////////////////////////////////////////////////////////////////////////
		_double.name("double");
		_double =
			real_parser<double, strict_real_policies<double> >();

		//////////////////////////////////////////////////////////////////////////
		_integer.name("integer");
		_integer =
			(
				ascii::string("0x") > //непонятный баг в недрах спирита (ubuntu64 10, gcc 4.6, boost 1.42)
                //ascii::string("0x") >>
				+_hexDigit >>
				-(char_('u') >>
					-(ascii::string("8")|ascii::string("16")|ascii::string("32")|ascii::string("64")
					)
				)
			)|
			(
				+char_("0-9") >>
				-((ascii::string("ui") | char_('u') | char_('i')) >>
					-(ascii::string("8")|ascii::string("16")|ascii::string("32")|ascii::string("64")
					)
				)
			)|
			(
				char_('-') >>
				+char_("0-9") >>
				-(char_('i') >>
					-(ascii::string("8")|ascii::string("16")|ascii::string("32")|ascii::string("64")
					)
				)
			);










		//////////////////////////////////////////////////////////////////////////
		_dateScope.name("dateScope");
		_dateScope =
			_dateTemplate[_val = spirit::_1] >
			_validDate(_val)[Checker()];

		_dateTemplate.name("dateTemplate");
		_dateTemplate =
			uint_parser<unsigned, 10, 4, 4>()>> lit('-') >>
			uint_parser<unsigned, 10, 2, 2>()>> lit('-') >>
			uint_parser<unsigned, 10, 2, 2>();

		_validDate.name("validDate");
		_validDate =
			eps[_val=_r1];









		//////////////////////////////////////////////////////////////////////////
		_timeScope.name("timeScope");
		_timeScope =
			_timeTemplate[_val = spirit::_1] >
			_validTime(_val)[Checker()];

		//////////////////////////////////////////////////////////////////////////
		_timeTemplate.name("timeTemplate");
		_timeTemplate =
			int_parser<int, 10, 2, 2>()>> lit(':') >>
			uint_parser<unsigned, 10, 2, 2>()>> lit(':') >>
			uint_parser<unsigned, 10, 2, 2>()>>
			-(
				lit('.') > _timeTemplate_microseconds
			);

		_timeTemplate_microseconds.name("microseconds(6 decimal digits)");
		_timeTemplate_microseconds =
			uint_parser<unsigned, 10, 6, 6>();

		_validTime.name("validTime");
		_validTime =
			eps[_val=_r1];




		//////////////////////////////////////////////////////////////////////////
		_dateTimeScope.name("dateTimeScope");
		_dateTimeScope =
			_dateTimeTemplate[_val = spirit::_1] >
			_validDateTime(_val)[Checker()];


		_dateTimeTemplate.name("dateTimeTemplate");
		_dateTimeTemplate =
			_dateTemplate[_val=spirit::_1] >>
			(lit(' ')|'T') >>
			_timeTemplate[_val=spirit::_1];

		_validDateTime.name("validDateTime");
		_validDateTime =
			eps[_val=_r1];


		//////////////////////////////////////////////////////////////////////////
		_bool.name("bool");
		_bool = no_case[_boolSymbols];

		_boolSymbols.add
			("true", true)
			("t", true)
			("on", true)
			("false", false)
			("f", false)
			("off", false);

		//////////////////////////////////////////////////////////////////////////
		_uuid.name("uuid");
		_uuid =
			repeat(8)[_hexDigit] >> lit('-') >>
			repeat(4)[_hexDigit] >> lit('-') >>
			repeat(4)[_hexDigit] >> lit('-') >>
			repeat(4)[_hexDigit] >> lit('-') >>
			repeat(12)[_hexDigit];

		//////////////////////////////////////////////////////////////////////////
		_ident.name("ident");
		_ident =
			((alpha | char_('_')) >> *(alnum | char_('_')));



		_hexDigit.name("hexDigit");
		_hexDigit =
			char_("0-9a-fA-F");





		//////////////////////////////////////////////////////////////////////////
		_map.name("map");
		_map =
			lit("{")[phx::bind(&VariantLoadScope::map_start, _scope)] >
			-(
				_mapPair >>
				*(lit(",") >> _mapPair) >>
				-lit(",")
			) >
			lit("}")[phx::bind(&VariantLoadScope::map_stop, _scope)];

		_mapPair.name("mapPair");
		_mapPair =
			(_ident | _string)[phx::bind(&VariantLoadScope::map_key, _scope, spirit::_1)] >
			(lit(':')|lit('=')) >
			_variant[phx::bind(&VariantLoadScope::map_push, _scope)];

		//////////////////////////////////////////////////////////////////////////
		_array.name("array");
		_array =
			lit("[")[phx::bind(&VariantLoadScope::array_start, _scope)] >
			-(
				_variant[phx::bind(&VariantLoadScope::array_push, _scope)] >>
				*(lit(",") >> _variant[phx::bind(&VariantLoadScope::array_push, _scope)]) >>
				-lit(",")
			) >
			lit("]")[phx::bind(&VariantLoadScope::array_stop, _scope)];

		//////////////////////////////////////////////////////////////////////////
		_include.name("include");
		_include =
			lit("include") >
			_includePath[CheckerInclude(_scope)];

		_includePath.name("includePath");
		_includePath %=
			_string;

		//////////////////////////////////////////////////////////////////////////
		_variant.name("variant");
		_variant =
			_map |
			_array |
			_include |
			_scalar;

		_start.name("variant");
		_start =
			eps > _variant;

		//////////////////////////////////////////////////////////////////////////
		on_error<fail>
		(
			_start,
			phx::bind(&VariantLoadScope::error, _scope, spirit::_1, spirit::_2, spirit::_3, spirit::_4)
		);

	}

}
