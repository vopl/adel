#include "pch.hpp"
#include "utils/variant.hpp"
#include <fstream>

#include "variantLoadScope.hpp"
#include "variantLoadGrammar.hpp"

namespace utils
{
	namespace qi = boost::spirit::qi;
	namespace spirit = boost::spirit;

	//////////////////////////////////////////////////////////////////////////
	bool Variant::load(const char *fileName, std::string *errors)
	{
		boost::filesystem::path p = boost::filesystem::absolute(fileName);
		std::ifstream in(p.string().c_str(), std::ios::binary);
		if(!in)
		{
			if(errors)
			{
				*errors += "file not found: ";
				*errors += fileName;
				*errors += "\n";
			}
			return false;
		}

		bool parseResult;

		{
			std::vector<char> buffer;
			in.seekg(0, std::ios::end);
			buffer.resize(in.tellg());
			in.seekg(0, std::ios::beg);


			const char *first;
			const char *last;
			if(!buffer.empty())
			{
				in.read(&buffer[0], buffer.size());
				in.close();
				first = &buffer[0];
				last = first + buffer.size();
			}
			else
			{
				first = "";
				last = first + 0;
			}

			VariantLoadScope scope(fileName, first, last, errors);
			VariantLoadGrammar vlg(scope);
			SkipGrammar sg;

			parseResult = qi::phrase_parse(
				first,
				last,
				vlg,
				sg);

			if(parseResult)
			{
				if(first != last)
				{
					boost::spirit::info what(std::string("end"));
					scope.error(&buffer[0], last, first, what);
				}
			}

			if(scope.errorWas())
			{
				parseResult = false;
			}
			if(parseResult)
			{
				this->swap(scope.getValue());
			}
		}


		return parseResult;
	}


}
