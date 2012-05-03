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
		size_t asPod() const
		{
			return _hi<<8|_lo;
		}

		bool operator>(const Version &v) const
		{
			return asPod() > v.asPod();
		}
		bool operator>=(const Version &v) const
		{
			return asPod() >= v.asPod();
		}
		bool operator<(const Version &v) const
		{
			return asPod() < v.asPod();
		}
		bool operator<=(const Version &v) const
		{
			return asPod() <= v.asPod();
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
