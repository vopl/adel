#ifndef _NET_HTTP_VERSION_HPP_
#define _NET_HTTP_VERSION_HPP_


namespace net { namespace http
{
	struct Version
	{
		unsigned short _hi;
		unsigned short _lo;

		Version()
			: _hi()
			, _lo()
		{
		}

		Version(unsigned short hi, unsigned short lo)
			: _hi(hi)
			, _lo(lo)
		{
		}

		bool operator>(const Version &v) const
		{
			if(_hi > v._hi) return true;
			return _lo > v._lo;
		}
		bool operator>=(const Version &v) const
		{
			if(_hi >= v._hi) return true;
			return _lo >= v._lo;
		}
		bool operator<(const Version &v) const
		{
			if(_hi < v._hi) return true;
			return _lo < v._lo;
		}
		bool operator<=(const Version &v) const
		{
			if(_hi <= v._hi) return true;
			return _lo <= v._lo;
		}

		bool operator==(const Version &v) const
		{
			return _hi == v._hi && _lo == v._lo;
		}
		bool operator!=(const Version &v) const
		{
			return !operator==(v);
		}
};
}}
#endif
