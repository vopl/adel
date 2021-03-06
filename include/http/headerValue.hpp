#ifndef _HTTP_HEADERVALUE_HPP_
#define _HTTP_HEADERVALUE_HPP_

#include "http/inputMessage.hpp"
#include "http/transferEncoding.hpp"
#include "http/contentEncoding.hpp"
#include "http/connection.hpp"
#include <string>

namespace http
{

    template <class Tag>
    class HeaderValue
    {
    public:
    	typedef typename Tag::Value Value;

	public:
		HeaderValue()
			: _value()
			, _isCorrect(false)
		{
		}

		HeaderValue(const InputMessage::Segment &src)
			: _isCorrect(parse(src))
		{
		}

		HeaderValue(const InputMessage::Segment *src)
			: _isCorrect(src?parse(*src):false)
		{
		}

		HeaderValue(const Value &value)
			: _value(value)
			, _isCorrect(true)
		{
		}

		const Value &value() const
		{
			return _value;
		}

		Value &value()
		{
			return _value;
		}

		bool isCorrect() const
		{
			return _isCorrect;
		}

		HeaderValue &operator=(const InputMessage::Segment &source)
		{
			_value = Value();
			_isCorrect = parse(source);
			return *this;
		}

		HeaderValue &operator=(const InputMessage::Segment *source)
		{
			_value = Value();
			if(source)
			{
				_isCorrect = parse(*source);
			}
			else
			{
				_isCorrect = false;
			}
			return *this;
		}

		HeaderValue &operator=(const Value &value)
		{
			_value = value;
			_isCorrect = true;
			return *this;
		}

		HeaderValue &setIsCorrect(bool isCorrect)
		{
			_isCorrect = isCorrect;
			return *this;
		}

		operator std::string() const
		{
			std::string res;
			std::back_insert_iterator<std::string> out(res);
			bool b = generate(out);
			assert(b);
			(void)b;

			return res;
		}

	public:
		bool parse(const InputMessage::Segment &src);

		template <class Iterator>
		bool generate(Iterator &outIterator) const;

	protected:
		Value	_value;
		bool	_isCorrect;
	};

    template <class Tag>
    std::string operator+(const char *csz, const HeaderValue<Tag> &hv)
    {
    	return csz + hv.operator std::string();
    }

    template <class Tag>
    std::string operator+(const std::string &str, const HeaderValue<Tag> &hv)
    {
    	return str + hv.operator std::string();
    }
}


namespace http
{
	struct Date
	{
		typedef time_t Value;
	};

	struct Unsigned
	{
		typedef size_t Value;
	};

	struct Connection
	{
		typedef EConnection Value;
	};

	struct TransferEncoding
	{
		typedef int Value;//ETransferEncoding bits
	};

	struct ContentEncoding
	{
		typedef int Value;//EContentEncoding bits
	};

}
#endif
