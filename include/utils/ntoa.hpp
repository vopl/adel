#ifndef _UTILS_NTOA_HPP_
#define _UTILS_NTOA_HPP_

#include <cstring>

//////////////////////////////////////////////////////////////////////////
namespace utils
{
	namespace impl
	{
		template <int maxDecs, typename integral>
		char *_ntoa(integral num, char *res)
		{
			if(num)
			{
				bool negative = num<0;
				int pos=maxDecs;

				res[pos] = 0;

				if(negative)
				{
					while(num)
					{
						res[--pos] = (char)('0' - (num % 10));
						num /= 10;
					}
					res[--pos] = '-';
				}
				else
				{
					while(num)
					{
						res[--pos] = (char)('0' + (num % 10));
						num /= 10;
					}
				}
				if(pos)
				{
					memmove(res, res+pos, maxDecs-pos+1);
				}
				return res;
			}
			res[0] = '0';
			res[1] = 0;
			return res;
		}
		template <int maxDecs, typename integral>
		char *_untoa(integral num, char *res)
		{
			if(num)
			{
				int pos=maxDecs;
				res[pos] = 0;

				while(num)
				{
					res[--pos] = (char)('0' + (num % 10));
					num /= 10;
				}
				if(pos)
				{
					memmove(res, res+pos, maxDecs-pos+1);
				}
				return res;
			}
			res[0] = '0';
			res[1] = 0;
			return res;
		}
	}

	inline
		char *_ntoa(long long num, char *res)
	{
		return impl::_ntoa<24>(num, res);
	}
	inline
		char *_ntoa(unsigned long long num, char *res)
	{
		return impl::_untoa<25>(num, res);
	}
	inline
		char *_ntoa(long num, char *res)
	{
		return impl::_ntoa<12>(num, res);
	}
	inline
		char *_ntoa(unsigned long num, char *res)
	{
		return impl::_untoa<13>(num, res);
	}
	inline
		char *_ntoa(int num, char *res)
	{
		return impl::_ntoa<12>(num, res);
	}
	inline
		char *_ntoa(unsigned int num, char *res)
	{
		return impl::_untoa<13>(num, res);
	}
	inline
		char *_ntoa(short num, char *res)
	{
		return impl::_ntoa<6>(num, res);
	}
	inline
		char *_ntoa(unsigned short num, char *res)
	{
		return impl::_untoa<7>(num, res);
	}
	inline
		char *_ntoa(char num, char *res)
	{
		return impl::_ntoa<3>(num, res);
	}
	inline
		char *_ntoa(unsigned char num, char *res)
	{
		return impl::_untoa<4>(num, res);
	}

}

#endif
