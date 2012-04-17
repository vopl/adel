#include "pch.hpp"
#include "variantLoadScope.hpp"
#include "utils/ntoa.hpp"
#include "utils/aton.hpp"
#include "utils/xton.hpp"
#include "utils/fixEndian.hpp"

#include "boost/date_time/posix_time/posix_time.hpp"

namespace utils
{
	//////////////////////////////////////////////////////////////////////////
	VariantLoadScope::VariantLoadScope(
		boost::filesystem::path fileName, 
		const char *first,
		const char *last,
		std::string *errors)
		: _fileName(fileName)
		, _first(first)
		, _last(last)
		, _errors(errors)
		, _errorWas(false)
	{
		_stack.push_back(Variant());
	}

	//////////////////////////////////////////////////////////////////////////
	VariantLoadScope::~VariantLoadScope()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	Variant &VariantLoadScope::getValue()
	{
		assert(!_errorWas);
		assert(_stack.size() == 1);
		return _stack.back();
	}


	//////////////////////////////////////////////////////////////////////////
	bool VariantLoadScope::errorWas()
	{
		return _errorWas;
	}


	//////////////////////////////////////////////////////////////////////////
	void VariantLoadScope::error(
		const char *first, const char *last, 
		const char *errorPos, 
		const boost::spirit::info &what)
	{
		_errorWas = true;
		_stack.clear();

		if(!_errors)
		{
			return;
		}
// 		if(_first != first || _last!=last)
// 		{
// 			return;
// 		}
		first = _first;
		last = _last;

		std::string &msg = *_errors;

		const char *firstContext=first, *lastContext=errorPos;
		size_t line = 1;
		for(const char *iter=first; iter!=errorPos; iter++)
		{
			if('\r' == *iter && iter+1 != last && '\n' == *(iter+1))
			{
				line++;
				firstContext = iter+2;
				iter++;
			}
			else if('\n' == *iter || '\r' == *iter)
			{
				line++;
				firstContext = iter+1;
			}
		}

		size_t col = std::distance(firstContext, errorPos)+1;

		for(; lastContext!=last; lastContext++)
		{
			if('\r' == *lastContext && lastContext+1 != last && '\n' == *(lastContext+1))
			{
				break;
			}
			else if('\n' == *lastContext || '\r' == *lastContext)
			{
				break;
			}
		}
		std::string(firstContext, lastContext);

		msg += _fileName.string();
		msg += ", line ";
		char tmp[32];
		msg += _ntoa(line, tmp);
		msg += " column ";
		msg += _ntoa(col, tmp);

		std::stringstream ss;
		ss<<what;
		msg += ": expected "+ss.str()+"\n";

		msg += std::string(firstContext, lastContext);
		msg += "\n";
		for(size_t i(0); i<col-1; i++)
		{
			msg += firstContext[i]=='\t'?'\t':' ';
		}
		msg += "^\n";

		return;


// 		"error at line %d, %d: " what
// 			context

// 		std::cout
// 			<< phx::val("Error! Expecting ")
// 			<< _4                               // what failed?
// 			<< phx::val(" here: \"")
// 			<< phx::construct<std::string>(_3, _2)   // iterators to error-pos, end
// 			<< phx::val("\"")
// 			<< std::endl

	}

	//////////////////////////////////////////////////////////////////////////
	void VariantLoadScope::set_null()
	{
		if(_errorWas) return;
		assert(!_stack.empty());

		_stack.back().setNull<void>(true);
	}

	//////////////////////////////////////////////////////////////////////////
	void VariantLoadScope::set_string(const std::string &content)
	{
		if(_errorWas) return;
		assert(!_stack.empty());

		_stack.back() = content;
	}

	//////////////////////////////////////////////////////////////////////////
	void VariantLoadScope::set_double(const double &content)
	{
		if(_errorWas) return;
		assert(!_stack.empty());

		_stack.back() = content;
	}

	//////////////////////////////////////////////////////////////////////////
	void VariantLoadScope::set_float(const float &content)
	{
		if(_errorWas) return;
		assert(!_stack.empty());

		_stack.back() = content;
	}

	//////////////////////////////////////////////////////////////////////////
	void VariantLoadScope::set_integer(const std::string &content)
	{
		if(_errorWas) return;
		assert(!_stack.empty());

		size_t size = content.size();
		
		bool unsigned_ = false;
		size_t volume = 32;
		
		size_t posUI = content.find_first_of("ui");
		if(std::string::npos != posUI)
		{
			size_t pos = posUI;
			while(pos != size)
			{
				switch(content[pos])
				{
				case 'u':
					unsigned_ = true;
				case 'i':
					break;
				case '8':
					volume = 8;
					break;
				case '1':
					pos++;
					assert(content[pos]=='6');
					volume = 16;
					break;
				case '3':
					pos++;
					assert(content[pos]=='2');
					volume = 32;
					break;
				case '6':
					pos++;
					assert(content[pos]=='4');
					volume = 64;
					break;
				default:
					assert(0);
				}
				pos++;
				continue;
			}
			size = posUI;
		}

		const char *begin = content.data();

		bool radix16 = false;

		if(size>2 && begin[0] == '0' && begin[1]=='x')
		{
			radix16 = true;
			begin += 2;
			size -= 2;
		}

		switch(volume)
		{
		case 8:
			if(unsigned_)
			{
				boost::uint8_t n;
				if(radix16)	_xtoun(begin, n);
				else		_atoun(begin, n);
				_stack.back() = n;
			}
			else
			{
				boost::int8_t n;
				if(radix16)	_xtoun(begin, n);
				else		_aton(begin, n);
				_stack.back() = n;
			}
			break;
		case 16:
			if(unsigned_)
			{
				boost::uint16_t n;
				if(radix16)	_xtoun(begin, n);
				else		_atoun(begin, n);
				_stack.back() = n;
			}
			else
			{
				boost::int16_t n;
				if(radix16)	_xtoun(begin, n);
				else		_aton(begin, n);
				_stack.back() = n;
			}
			break;
		case 32:
			if(unsigned_)
			{
				boost::uint32_t n;
				if(radix16)	_xtoun(begin, n);
				else		_atoun(begin, n);
				_stack.back() = n;
			}
			else
			{
				boost::int32_t n;
				if(radix16)	_xtoun(begin, n);
				else		_aton(begin, n);
				_stack.back() = n;
			}
			break;
		case 64:
			if(unsigned_)
			{
				boost::uint64_t n;
				if(radix16)	_xtoun(begin, n);
				else		_atoun(begin, n);
				_stack.back() = n;
			}
			else
			{
				boost::int64_t n;
				if(radix16)	_xtoun(begin, n);
				else		_aton(begin, n);
				_stack.back() = n;
			}
			break;
		}
	}


	//////////////////////////////////////////////////////////////////////////
	void VariantLoadScope::set_bool(bool b)
	{
		if(_errorWas) return;
		assert(!_stack.empty());

		_stack.back() = b;
	}

	//////////////////////////////////////////////////////////////////////////
	void VariantLoadScope::set_uuid(const std::string &content)
	{
		if(_errorWas) return;
		assert(!_stack.empty());

		assert(32 == content.size());

		boost::uuids::uuid u;
		for(size_t i(0); i<16; i++)
		{
			boost::uint8_t &b = *(u.begin()+i);
			b = x2n(content[i*2])<<4 | x2n(content[i*2+1]);
		}

		*(boost::uint32_t *)(u.begin()+0) = fixEndian(*(boost::uint32_t *)(u.begin()+0));
		*(boost::uint16_t *)(u.begin()+4) = fixEndian(*(boost::uint16_t *)(u.begin()+4));
		*(boost::uint16_t *)(u.begin()+6) = fixEndian(*(boost::uint16_t *)(u.begin()+6));

		_stack.back() = u;
	}

	//////////////////////////////////////////////////////////////////////////
	void VariantLoadScope::set_date(const SDate &d)
	{
		if(_errorWas) return;
		assert(!_stack.empty());

		boost::gregorian::date gd(d._year, d._month, d._day);

		_stack.back() = gd;
	}

	//////////////////////////////////////////////////////////////////////////
	void VariantLoadScope::set_time(const STime &t)
	{
		if(_errorWas) return;
		assert(!_stack.empty());

		boost::posix_time::time_duration ptd(
			t._hour, 
			t._minute, 
			t._second);
		ptd += boost::posix_time::microseconds(t._microsec);

		_stack.back() = ptd;
	}

	//////////////////////////////////////////////////////////////////////////
	void VariantLoadScope::set_datetime(const SDateTime &dt)
	{
		if(_errorWas) return;
		assert(!_stack.empty());

		boost::gregorian::date gd(dt._year, dt._month, dt._day);

		boost::posix_time::time_duration ptd(
			dt._hour, 
			dt._minute, 
			dt._second);
		ptd += boost::posix_time::microseconds(dt._microsec);

		_stack.back() = boost::posix_time::ptime(gd, ptd);
	}



	//////////////////////////////////////////////////////////////////////////
	void VariantLoadScope::array_start()
	{
		if(_errorWas) return;
		assert(!_stack.empty());

		_stack.back().as<Variant::DequeVariant>(true);
		_stack.push_back(Variant());
	}

	//////////////////////////////////////////////////////////////////////////
	void VariantLoadScope::array_push()
	{
		if(_errorWas) return;
		assert(!_stack.empty());

		Variant v;
		v.swap(_stack.back());
		_stack[_stack.size()-2].as<Variant::DequeVariant>().push_back(v);
	}

	//////////////////////////////////////////////////////////////////////////
	void VariantLoadScope::array_stop()
	{
		if(_errorWas) return;
		assert(!_stack.empty());

		_stack.pop_back();
	}

	//////////////////////////////////////////////////////////////////////////
	void VariantLoadScope::map_start()
	{
		if(_errorWas) return;
		assert(!_stack.empty());

		_stack.back().as<Variant::MapStringVariant>(true);
		_stack.push_back(Variant());

	}

	//////////////////////////////////////////////////////////////////////////
	void VariantLoadScope::map_key(const std::string &content)
	{
		if(_errorWas) return;
		assert(!_stack.empty());
		
		_mapKey = content;
	}

	//////////////////////////////////////////////////////////////////////////
	void VariantLoadScope::map_push()
	{
		if(_errorWas) return;
		assert(!_stack.empty());

		_stack[_stack.size()-2].as<Variant::MapStringVariant>()[_mapKey].swap(_stack.back());
		_stack.back().setNull<void>(true);
	}

	//////////////////////////////////////////////////////////////////////////
	void VariantLoadScope::map_stop()
	{
		if(_errorWas) return;
		assert(!_stack.empty());

		_stack.pop_back();
	}

	//////////////////////////////////////////////////////////////////////////
	bool VariantLoadScope::do_include(const std::string &path)
	{
		if(_errorWas) return false;
		assert(!_stack.empty());

		boost::filesystem::path bpath(path);
		if(bpath.is_absolute())
		{
			//use this path
		}
		else
		{
			bpath = _fileName.parent_path() / bpath;
		}

		std::string erorros;
		if(!_stack.back().load(bpath.string().c_str(), &erorros))
		{
			_errorWas = true;
			if(_errors)
			{
				*_errors += erorros;
			}

			return false;
		}

		return true;
	}



}
