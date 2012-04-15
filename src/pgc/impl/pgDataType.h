#ifndef _PGC_PGDATATYPES_H_
#define _PGC_PGDATATYPES_H_

namespace pgc
{
#pragma pack (push, 1)
	typedef struct PG_NumericData
	{
		short ndigits;
		short weight;
		short sign;
		short dscale;
		short NumericDigits[1];
	} PG_NumericData;


	typedef struct PG_Interval
	{
		boost::uint64_t time;
		boost::uint32_t day;
		boost::uint32_t month;
	} PG_Interval;



	typedef struct PG_VarBit
	{
		boost::uint32_t amount;
		boost::uint8_t bits[1];
	} PG_VarBit;
#pragma pack (pop)



}
#endif
