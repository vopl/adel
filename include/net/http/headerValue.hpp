#ifndef _NET_HTTP_HEADERVALUE_HPP_
#define _NET_HTTP_HEADERVALUE_HPP_

#include "net/message.hpp"
#include <string>

namespace net { namespace http
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

		HeaderValue(const Message::Segment &src)
			: _isCorrect(parse(src))
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

		bool isCorrect() const
		{
			return _isCorrect;
		}

		HeaderValue &operator=(const Message::Segment &source)
		{
			_value = Value();
			_isCorrect = parse(source);
			return *this;
		}

		HeaderValue &operator=(const Value &value)
		{
			_value = value;
			_isCorrect = true;
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
		bool parse(const Message::Segment &src);

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
}}


namespace net { namespace http
{
	struct Date
	{
		typedef time_t Value;
	};


}}
#endif
