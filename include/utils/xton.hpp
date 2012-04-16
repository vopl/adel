#ifndef _UTILS_XTON_HPP_
#define _UTILS_XTON_HPP_

//////////////////////////////////////////////////////////////////////////
namespace utils
{
	//////////////////////////////////////////////////////////////////////////
	inline signed char x2n(char c)
	{
		if(c>='0' && c <='9')	return c - '0';
		if(c>='a' && c <='f')	return c - 'a' + 10;
		if(c>='A' && c <='F')	return c - 'A' + 10;
		return -1;
	}

	//////////////////////////////////////////////////////////////////////////
	template <typename integral>
	void _xtoun(const char *x, integral &num)
	{
		num = 0;
		for(signed char n = x2n(*x); *x && n>=0; x++, n=x2n(*x))
		{
			num *= 16;
			num += n;
		}
	}



	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	inline unsigned char _xtouc(const char *a)
	{
		unsigned char res;
		_xtoun(a, res);
		return res;
	}
	inline unsigned short _xtous(const char *a)
	{
		unsigned short res;
		_xtoun(a, res);
		return res;
	}
	inline unsigned int _xtoui(const char *a)
	{
		unsigned int res;
		_xtoun(a, res);
		return res;
	}
	inline unsigned long _xtoul(const char *a)
	{
		unsigned long res;
		_xtoun(a, res);
		return res;
	}
	inline unsigned long long _xtoull(const char *a)
	{
		unsigned long long res;
		_xtoun(a, res);
		return res;
	}
	inline size_t _xtost(const char *a)
	{
		size_t res;
		_xtoun(a, res);
		return res;
	}

}

#endif
