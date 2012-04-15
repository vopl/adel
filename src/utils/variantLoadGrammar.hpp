#ifndef _UTILS_VARIANTLOADGRAMMAR_HPP_
#define _UTILS_VARIANTLOADGRAMMAR_HPP_

#include "variantLoadScope.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/repository/include/qi_confix.hpp>


namespace utils
{
	namespace spirit = boost::spirit;
	namespace qi = boost::spirit::qi;


	struct SkipGrammar
		: public qi::grammar<const char *>
    {
		SkipGrammar()
			: SkipGrammar::base_type(_skip)
        {
			_skip
				= qi::space
				| spirit::repository::confix(qi::lit("//")|'#', (qi::eol|qi::eoi))[*(qi::char_ - (qi::eol|qi::eoi))]
				| spirit::repository::confix("/*", "*/")[*(qi::char_ - "*/")]
            ;
        }

		qi::rule<const char *> _skip;
    };

	//////////////////////////////////////////////////////////////////////////
	struct VariantLoadGrammar
		: qi::grammar<const char *, SkipGrammar>
	{

	public:
		VariantLoadGrammar(VariantLoadScope &scope);

		qi::rule<const char *, SkipGrammar> _start;
		qi::rule<const char *, SkipGrammar> _variant;

		qi::rule<const char *> _scalar;
		qi::rule<const char *, SkipGrammar> _map;
		qi::rule<const char *, SkipGrammar> _mapPair;
		qi::rule<const char *, SkipGrammar> _array;
		qi::rule<const char *, SkipGrammar> _include;
		qi::rule<const char *, std::string(), SkipGrammar> _includePath;

		qi::rule<const char *> _null;
		qi::rule<const char *, std::string()> _string;
		qi::rule<const char *, float()> _float;
		qi::rule<const char *, double()> _double;
		qi::rule<const char *, std::string()> _integer;

		qi::rule<const char *, VariantLoadScope::SDate()> _dateScope;
		qi::rule<const char *, VariantLoadScope::SDate()> _dateTemplate;
		qi::rule<const char *, VariantLoadScope::SDate(VariantLoadScope::SDate)> _validDate;

		qi::rule<const char *, VariantLoadScope::STime()> _timeScope;
		qi::rule<const char *, VariantLoadScope::STime()> _timeTemplate;
		qi::rule<const char *, unsigned()> _timeTemplate_microseconds;
		qi::rule<const char *, VariantLoadScope::STime(VariantLoadScope::STime)> _validTime;


		qi::rule<const char *, VariantLoadScope::SDateTime()> _dateTimeScope;
		qi::rule<const char *, VariantLoadScope::SDateTime()> _dateTimeTemplate;
		qi::rule<const char *, VariantLoadScope::SDateTime(VariantLoadScope::SDateTime)> _validDateTime;


		qi::rule<const char *, bool()> _bool;
		qi::symbols<char, bool> _boolSymbols;
		qi::rule<const char *, std::string()> _uuid;

		qi::rule<const char *, std::string()> _ident;
		qi::rule<const char *, char()> _hexDigit;

		VariantLoadScope &_scope;
	};
}

#endif
