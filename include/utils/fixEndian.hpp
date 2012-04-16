#ifndef _UTILS_FIXENDIAN_H_
#define _UTILS_FIXENDIAN_H_

#include <cstring>
#include <boost/cstdint.hpp>

namespace utils
{
	namespace impl
	{
		//////////////////////////////////////////////////////////////////////////
		template <bool le>
		struct FixEndian
		{
			template <class T>
			static T call(const T &v);
		};

		//////////////////////////////////////////////////////////////////////////
		template <>
		template <class T>
		inline T FixEndian<true>::call(const T &v)
		{
			T res = v;
			unsigned char *data = (unsigned char *)&v;
			int len = sizeof(T);
			size_t len_2 = len/2;
			for(size_t i=0; i<len_2; i++)
			{
				size_t i2 = len - i - 1;
				unsigned char b = data[i];
				data[i] = data[i2];
				data[i2] = b;
			}
			return v;
		}


		//////////////////////////////////////////////////////////////////////////
		template <>
		template <>
		inline boost::int16_t FixEndian<true>::call<boost::int16_t>(const boost::int16_t &v)
		{
			unsigned char const *s = (unsigned char const *)&v;
			return
				(boost::int16_t)s[0] << 8 |
				(boost::int16_t)s[1];
		}
		template <>
		template <>
		inline boost::uint16_t FixEndian<true>::call<boost::uint16_t>(const boost::uint16_t &v)
		{
			return (boost::uint16_t)call((const boost::int16_t &)v);
		}

		//////////////////////////////////////////////////////////////////////////
		template <>
		template <>
		inline boost::int32_t FixEndian<true>::call<boost::int32_t>(const boost::int32_t &v)
		{
			unsigned char const *s = (unsigned char const *)&v;
			return
				(boost::int32_t)s[0] << 24 |
				(boost::int32_t)s[1] << 16 |
				(boost::int32_t)s[2] << 8 |
				(boost::int32_t)s[3];
		}
		template <>
		template <>
		inline boost::uint32_t FixEndian<true>::call<boost::uint32_t>(const boost::uint32_t &v)
		{
			return (boost::uint32_t)call((const boost::int32_t &)v);
		}

		//////////////////////////////////////////////////////////////////////////
		template <>
		template <>
		inline boost::int64_t FixEndian<true>::call<boost::int64_t>(const boost::int64_t &v)
		{

			unsigned char const *s = (unsigned char const *)&v;
			return
				(boost::int64_t)s[0] << 56 |
				(boost::int64_t)s[1] << 48 |
				(boost::int64_t)s[2] << 40 |
				(boost::int64_t)s[3] << 32 |
				(boost::int64_t)s[4] << 24 |
				(boost::int64_t)s[5] << 16 |
				(boost::int64_t)s[6] << 8 |
				(boost::int64_t)s[7];
		}
		template <>
		template <>
		inline boost::uint64_t FixEndian<true>::call<boost::uint64_t>(const boost::uint64_t &v)
		{
			return (boost::uint64_t)call((const boost::int64_t &)v);
		}

		//////////////////////////////////////////////////////////////////////////
		template <>
		template <>
		inline float FixEndian<true>::call<float>(const float &v)
		{
			unsigned char const *s = (unsigned char const *)&v;
			boost::int32_t bytes =
				(boost::int32_t)s[0] << 24 |
				(boost::int32_t)s[1] << 16 |
				(boost::int32_t)s[2] << 8 |
				(boost::int32_t)s[3];

			return *(float *)&bytes;
		}

		//////////////////////////////////////////////////////////////////////////
		template <>
		template <>
		inline double FixEndian<true>::call<double>(const double &v)
		{
			unsigned char const *s = (unsigned char const *)&v;
			boost::int64_t bytes =
				(boost::int64_t)s[0] << 56 |
				(boost::int64_t)s[1] << 48 |
				(boost::int64_t)s[2] << 40 |
				(boost::int64_t)s[3] << 32 |
				(boost::int64_t)s[4] << 24 |
				(boost::int64_t)s[5] << 16 |
				(boost::int64_t)s[6] << 8 |
				(boost::int64_t)s[7];

			return *(double *)&bytes;
		}


		template <>
		struct FixEndian<false>
		{
			template <class T>
			static T call(const T &v)
			{
				return v;
			}
		};
	}

	//////////////////////////////////////////////////////////////////////////
	static const bool isLitEndian = ((boost::uint8_t)((boost::uint16_t)1)) ? true : false;
	static const bool isBigEndian = !isLitEndian;

	//////////////////////////////////////////////////////////////////////////
	template <class T>
	inline T bigEndian(const T &v)
	{
		return impl::FixEndian<isLitEndian>::call(v);
	}

	//////////////////////////////////////////////////////////////////////////
	template <class T>
	inline T litEndian(const T &v)
	{
		return impl::FixEndian<!isLitEndian>::call(v);
	}

	//////////////////////////////////////////////////////////////////////////
	template <class T>
	inline T fixEndian(const T &v)
	{
		return impl::FixEndian<true>::call(v);
	}
}
#endif
