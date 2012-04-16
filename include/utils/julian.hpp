#ifndef _UTILS_JULIANUTILS_HPP_
#define _UTILS_JULIANUTILS_HPP_

namespace utils
{
	//////////////////////////////////////////////////////////////////////////
	int
		inline date2j(int y, int m, int d)
	{
		int			julian;
		int			century;

		if (m > 2)
		{
			m += 1;
			y += 4800;
		}
		else
		{
			m += 13;
			y += 4799;
		}

		century = y / 100;
		julian = y * 365 - 32167;
		julian += y / 4 - century + century / 4;
		julian += 7834 * m / 256 + d;

		return julian;
	}	/* date2j() */

	void
		inline j2date(int jd, int *year, int *month, int *day)
	{
		unsigned int julian;
		unsigned int quad;
		unsigned int extra;
		int			y;

		julian = jd;
		julian += 32044;
		quad = julian / 146097;
		extra = (julian - quad * 146097) * 4 + 3;
		julian += 60 + quad * 3 + extra / 146097;
		quad = julian / 1461;
		julian -= quad * 1461;
		y = julian * 4 / 1461;
		julian = ((y != 0) ? (julian + 305) % 365 : (julian + 306) % 366) + 123;
		y += quad * 4;
		*year = y - 4800;
		quad = julian * 2141 / 65536;
		*day = julian - 7834 * quad / 256;
		*month = (quad + 10) % 12 + 1;

		return;
	}	/* j2date() */

	static const boost::int32_t JULIAN_MINYEAR		= (-4713);
	static const boost::int32_t JULIAN_MINMONTH		= (11);
	static const boost::int32_t JULIAN_MINDAY		= (24);
	static const boost::int32_t JULIAN_MAXYEAR		= (5874898);

	static const boost::int32_t DAYS_PER_MONTH		= 30;
	static const boost::int32_t SECS_PER_YEAR		= (36525 * 864);
	static const boost::int32_t SECS_PER_DAY		= 86400;
	static const boost::int32_t SECS_PER_HOUR		= 3600;
	static const boost::int32_t SECS_PER_MINUTE		= 60;
	static const boost::int32_t MINS_PER_HOUR		= 60;

	static const boost::int64_t USECS_PER_DAY		= 86400000000LL;
	static const boost::int64_t USECS_PER_HOUR		= 3600000000LL;
	static const boost::int64_t USECS_PER_MINUTE	= 60000000LL;
	static const boost::int64_t USECS_PER_SEC		= 1000000LL;

	static const boost::int64_t POSTGRES_EPOCH_JDATE	= 2451545LL; /* == date2j(2000, 1, 1) */


	template<class Y, class M, class D>
	bool IS_VALID_JULIAN(const Y &y, const M &m, const D &d)
	{
		return 
			((((y) > JULIAN_MINYEAR) 
			|| (((y) == JULIAN_MINYEAR) && (((m) > JULIAN_MINMONTH) 
			|| (((m) == JULIAN_MINMONTH) && ((d) >= JULIAN_MINDAY))))) 
			&& ((y) < JULIAN_MAXYEAR));
	}



	template <class T1, class T2, class T3>
	void TMODULO(T1 &t, T2 &q, const T3 &u)
	{
		do
		{
			(q) = ((t) / (u));
			if ((q) != 0) (t) -= ((q) * (u));
		} while(0);
	}

	boost::int64_t
		inline time2t(const int hour, const int min, const int sec, const int fsec)
	{
		return (((((hour * MINS_PER_HOUR) + min) * SECS_PER_MINUTE) + sec) * USECS_PER_SEC) + fsec;
	}

	void
		inline dt2time(boost::int64_t jd, int *hour, int *min, int *sec, int *fsec)
	{
		boost::int64_t		time;

		time = jd;
		*hour = (int)(time / USECS_PER_HOUR);
		time -= (*hour) * USECS_PER_HOUR;
		*min = (int)(time / USECS_PER_MINUTE);
		time -= (*min) * USECS_PER_MINUTE;
		*sec = (int)(time / USECS_PER_SEC);
		*fsec = (int)(time - (*sec * USECS_PER_SEC));
	}	/* dt2time() */

}

#endif
