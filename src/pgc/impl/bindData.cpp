#include "utils/fixEndian.hpp"
#include "utils/ntoa.hpp"
#include "utils/julian.hpp"
#include "pgc/impl/bindData.hpp"
#include "pgc/impl/pgDataType.h"
#include "pgc/log.hpp"

namespace pgc { namespace impl
{
	//////////////////////////////////////////////////////////////////////////
	void BindData::resizeArrays(size_t s)
	{
		if(s > _typ.size())
		{
			_typ.resize(s, 0);
			_val.resize(s, NULL);
			_len.resize(s, 0);
			_fmt.resize(s, 0);
			_own.resize(s, 0);
		}
		else if(s < _typ.size())
		{
			for(size_t i(s); i<_typ.size(); i++)
			{
				if(_own[i])
				{
					delete _val[i];
				}
			}
			_typ.resize(s);
			_val.resize(s);
			_len.resize(s);
			_fmt.resize(s);
			_own.resize(s);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	template <class Sequence>
	void BindData::bindSeq(const Sequence &v)
	{
		resizeArrays(v.size());

		typename Sequence::const_iterator iter = v.begin();
		typename Sequence::const_iterator end = v.end();

		for(size_t idx(0); iter!=end; ++iter, ++idx)
		{
			if(!bindFiller(*iter, idx))
			{
				assert(0);
				ELOG("bind seq element failed: "<<idx);
				resizeArrays(0);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	template <class Associative>
	void BindData::bindAssoc(const Associative &v)
	{
		assert(0);
	}

	//////////////////////////////////////////////////////////////////////////
	void BindData::bindScalar(const Variant &v)
	{
		resizeArrays(1);
		if(!bindFiller(v, 0))
		{
			assert(0);
			ELOG("bind scalar failed");
			resizeArrays(0);
		}
	}


	//////////////////////////////////////////////////////////////////////////
	BindData::BindData(const Variant &data, ConnectionHolderPtr con)
		: _con(con)
		, typ(_typ)
		, val(_val)
		, len(_len)
		, fmt(_fmt)
	{
		switch(data.type())
		{


		case Variant::etVectorVariant:
			bindSeq(data.as<Variant::VectorVariant>());
			break;
		case Variant::etSetVariant:
			bindSeq(data.as<Variant::SetVariant>());
			break;
		case Variant::etMultisetVariant:
			bindSeq(data.as<Variant::MultisetVariant>());
			break;
		case Variant::etDequeVariant:
			bindSeq(data.as<Variant::DequeVariant>());
			break;
		case Variant::etListVariant:
			bindSeq(data.as<Variant::ListVariant>());
			break;





		case Variant::etMapStringVariant:
			bindAssoc(data.as<Variant::MapStringVariant>());
			break;
		case Variant::etMapVariantVariant:
			bindAssoc(data.as<Variant::MapVariantVariant>());
			break;
		case Variant::etMultimapVariantVariant:
			bindAssoc(data.as<Variant::MultimapVariantVariant>());
			break;
		case Variant::etMultimapStringVariant:
			bindAssoc(data.as<Variant::MultimapStringVariant>());
			break;



		default:
			bindScalar(data);
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	BindData::~BindData()
	{
		resizeArrays(0);
	}

	//////////////////////////////////////////////////////////////////////////
	namespace 
	{
		template <size_t N>
		void bitset2Bits(const std::bitset<N> &bs, boost::uint8_t *bits)
		{
			BOOST_STATIC_ASSERT(N%8 == 0);

			for(size_t i(0); i<N/8; i++)
			{
				bits[i]=
					(bs[i*8+0]?0x80:0) |
					(bs[i*8+1]?0x40:0) |
					(bs[i*8+2]?0x20:0) |
					(bs[i*8+3]?0x10:0) |
					(bs[i*8+4]?0x08:0) |
					(bs[i*8+5]?0x04:0) |
					(bs[i*8+6]?0x02:0) |
					(bs[i*8+7]?0x01:0);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool BindData::bindFiller(const Variant &v, size_t idx)
	{
		assert(idx < _typ.size());
		Oid &typ = _typ[idx];
		char *&val = _val[idx];
		int &len = _len[idx];
		int &fmt = _fmt[idx];
		char &own = _own[idx];

		if(v.isNull())
		{
			val = NULL;
			len = 0;
			own = false;
		}

		switch(v.type())
		{
		case Variant::etString:
			typ = 0;//untyped literal string
			fmt = 1;//binary
			if(!v.isNull())
			{
				len = (int)v.as<Variant::String>().size();
				val = new char[len];
				std::copy(v.as<Variant::String>().begin(), v.as<Variant::String>().end(), val);
				own = true;//own val wil be deleted later
			}
			break;
		case Variant::etBool:
			typ = 16;//bool
			fmt = 1;//binary
			if(!v.isNull())
			{
				len = 1;
				val = new char[len];
				*val = v.as<Variant::Bool>();
				own = true;
			}
			break;
		case Variant::etFloat:
			typ = 700;//float4
			fmt = 1;
			if(!v.isNull())
			{
				BOOST_STATIC_ASSERT(4 == sizeof(Variant::Float));
				len = 4;
				val = new char[len];
				*(float *)val = bigEndian(v.as<Variant::Float>());
				own = true;
			}
			break;
		case Variant::etDouble:
			typ = 701;//float8
			fmt = 1;
			if(!v.isNull())
			{
				BOOST_STATIC_ASSERT(8 == sizeof(Variant::Double));
				len = 8;
				val = new char[len];
				*(double *)val = bigEndian(v.as<Variant::Double>());
				own = true;
			}
			break;
		case Variant::etInt8:
			typ = 21;//int2
			fmt = 1;
			if(!v.isNull())
			{
				len = 2;
				val = new char[len];
				*(Variant::Int16 *)val = bigEndian((Variant::Int16)v.as<Variant::Int8>());
				own = true;
			}
			break;
		case Variant::etInt16:
			typ = 21;//int2
			fmt = 1;
			if(!v.isNull())
			{
				len = 2;
				val = new char[len];
				*(Variant::Int16 *)val = bigEndian(v.as<Variant::Int16>());
				own = true;
			}
			break;
		case Variant::etInt32:
			typ = 23;//int4
			fmt = 1;
			if(!v.isNull())
			{
				len = 4;
				val = new char[len];
				*(Variant::Int32 *)val = bigEndian(v.as<Variant::Int32>());
				own = true;
			}
			break;
		case Variant::etInt64:
			typ = 20;//int8
			fmt = 1;
			if(!v.isNull())
			{
				len = 8;
				val = new char[len];
				*(Variant::Int64 *)val = bigEndian(v.as<Variant::Int64>());
				own = true;
			}
			break;
		case Variant::etUInt8:
			typ = 21;//int2
			fmt = 1;
			if(!v.isNull())
			{
				len = 2;
				val = new char[len];
				*(Variant::UInt16 *)val = bigEndian((Variant::UInt16)v.as<Variant::UInt8>());
				own = true;
			}
			break;
		case Variant::etUInt16:
			typ = 21;//int2
			fmt = 1;
			if(!v.isNull())
			{
				len = 2;
				val = new char[len];
				*(Variant::UInt16 *)val = bigEndian(v.as<Variant::UInt16>());
				own = true;
			}
			break;
		case Variant::etUInt32:
			typ = 23;//int4
			fmt = 1;
			if(!v.isNull())
			{
				len = 4;
				val = new char[len];
				*(Variant::UInt32 *)val = bigEndian(v.as<Variant::UInt32>());
				own = true;
			}
			break;
		case Variant::etUInt64:
			{
				typ = 20;//int8
				fmt = 1;
				if(!v.isNull())
				{
					len = 8;
					val = new char[len];
					*(Variant::UInt64 *)val = bigEndian(v.as<Variant::UInt64>());
					own = true;
				}
			}
			break;
		case Variant::etTm:
			{
				typ = 1114;//timestamp
				fmt = 1;

				if(!v.isNull())
				{
					const std::tm &stm = v.as<Variant::Tm>();

					boost::int64_t ts =
						time2t(stm.tm_hour, stm.tm_min, stm.tm_sec, 0) +
						(date2j(stm.tm_year+1900, stm.tm_mon+1, stm.tm_mday) - POSTGRES_EPOCH_JDATE) * USECS_PER_DAY;

					len = 8;
					val = new char[len];

					if(_con->integerDatetimes())
					{
						*(boost::uint64_t *)val = bigEndian(ts);
					}
					else
					{
						*(double *)val = bigEndian(double(ts/1000000)+double(ts%1000000)/1000000);
					}
					own = true;
				}
			}
			break;
		case Variant::etDate:
			{
				typ = 1082;//date
				fmt = 1;
				if(!v.isNull())
				{
					const boost::gregorian::date &bgd = v.as<Variant::Date>();

					boost::int32_t d =
						date2j(bgd.year(), bgd.month(), bgd.day()) - POSTGRES_EPOCH_JDATE;

					len = 4;
					val = new char[len];
					*(boost::uint32_t *)val = bigEndian(d);
					own = true;
				}
			}
			break;
		case Variant::etDatetime:
			{
				typ = 1114;//timestamp
				fmt = 1;
				if(!v.isNull())
				{
					const boost::posix_time::ptime &ptm = v.as<Variant::Datetime>();

					boost::gregorian::date bgd = ptm.date();
					boost::posix_time::time_duration pt = ptm.time_of_day();

					boost::int64_t ts =
						pt.total_microseconds() +
						(date2j(bgd.year(), bgd.month(), bgd.day()) - POSTGRES_EPOCH_JDATE) * USECS_PER_DAY;

					len = 8;
					val = new char[len];
					if(_con->integerDatetimes())
					{
						*(boost::uint64_t *)val = bigEndian(ts);
					}
					else
					{
						*(double *)val = bigEndian(double(ts/1000000)+double(ts%1000000)/1000000);
					}
					own = true;
				}
			}
			break;
		case Variant::etDateDuration:
			{
				typ = 1186;//interval
				fmt = 1;
				if(!v.isNull())
				{
					const boost::gregorian::date_duration &bgdd = v.as<Variant::DateDuration>();

					len = sizeof(PG_Interval);
					val = new char[len];
					PG_Interval &pgi = *(PG_Interval *)val;
					own = true;

					pgi.month = bigEndian((boost::uint32_t)(bgdd.days() / DAYS_PER_MONTH));
					pgi.day = bigEndian((boost::uint32_t)(bgdd.days() % DAYS_PER_MONTH));
					pgi.time = 0;
				}
			}
			break;
		case Variant::etTimeDuration:
			{
				typ = 1186;//interval
				fmt = 1;
				if(!v.isNull())
				{
					const boost::posix_time::time_duration &bptd = v.as<Variant::TimeDuration>();

					len = sizeof(PG_Interval);
					val = new char[len];
					PG_Interval &pgi = *(PG_Interval *)val;
					own = true;

					pgi.month = 0;
					pgi.day = 0;
					if(_con->integerDatetimes())
					{
						pgi.time = bigEndian(bptd.total_microseconds());
					}
					else
					{
						boost::int64_t tms = bptd.total_microseconds();
						*(double *)(void *)&pgi.time = bigEndian(double(tms/1000000)+double(tms%1000000)/1000000);
					}
				}
			}
			break;
		case Variant::etDateTimeDuration:
			{
				typ = 1186;//interval
				fmt = 1;
				if(!v.isNull())
				{
					const DateTimeDuration &dtd = v.as<Variant::DateTimeDuration>();

					val = new char[sizeof(PG_Interval)];
					PG_Interval &pgi = *(PG_Interval *)val;
					len = sizeof(PG_Interval);
					own = true;

					pgi.month = bigEndian((boost::uint32_t)(dtd._dd.days() / DAYS_PER_MONTH));
					pgi.day = bigEndian((boost::uint32_t)(dtd._dd.days() % DAYS_PER_MONTH));

					if(_con->integerDatetimes())
					{
						pgi.time = bigEndian(dtd._td.total_microseconds());
					}
					else
					{
						boost::int64_t tms = dtd._td.total_microseconds();
						*(double *)(void *)&pgi.time = bigEndian(double(tms/1000000)+double(tms%1000000)/1000000);
					}
				}
			}
			break;

		case Variant::etBitset8:
			{
				typ = 1562;//varbit
				fmt = 1;
				if(!v.isNull())
				{
					const std::bitset<8> &bs = v.as<Variant::Bitset8>();

					len = sizeof(PG_VarBit)-1+1;
					val = new char[len];
					PG_VarBit &vb = *(PG_VarBit *)val;
					own = true;

					vb.amount = bigEndian((boost::uint32_t)8);
					bitset2Bits(bs, vb.bits);
				}
			}
			break;
		case Variant::etBitset16:
			{
				typ = 1562;//varbit
				fmt = 1;
				if(!v.isNull())
				{
					const std::bitset<16> &bs = v.as<Variant::Bitset16>();

					len = sizeof(PG_VarBit)-1+2;
					val = new char[len];
					PG_VarBit &vb = *(PG_VarBit *)val;
					own = true;

					vb.amount = bigEndian((boost::uint32_t)16);
					bitset2Bits(bs, vb.bits);
				}
			}
			break;
		case Variant::etBitset32:
			{
				typ = 1562;//varbit
				fmt = 1;
				if(!v.isNull())
				{
					const std::bitset<32> &bs = v.as<Variant::Bitset32>();

					len = sizeof(PG_VarBit)-1+4;
					val = new char[len];
					PG_VarBit &vb = *(PG_VarBit *)val;
					own = true;

					vb.amount = bigEndian((boost::uint32_t)32);
					bitset2Bits(bs, vb.bits);
				}
			}
			break;
		case Variant::etBitset64:
			{
				typ = 1562;//varbit
				fmt = 1;
				if(!v.isNull())
				{
					const std::bitset<64> &bs = v.as<Variant::Bitset64>();

					len = sizeof(PG_VarBit)-1+8;
					val = new char[len];
					PG_VarBit &vb = *(PG_VarBit *)val;
					own = true;

					vb.amount = bigEndian((boost::uint32_t)64);
					bitset2Bits(bs, vb.bits);
				}
			}
			break;
		case Variant::etBitset128:
			{
				typ = 1562;//varbit
				fmt = 1;

				if(!v.isNull())
				{
					const std::bitset<128> &bs = v.as<Variant::Bitset128>();

					len = sizeof(PG_VarBit)-1+16;
					val = new char[len];
					PG_VarBit &vb = *(PG_VarBit *)val;
					own = true;

					vb.amount = bigEndian((boost::uint32_t)128);
					bitset2Bits(bs, vb.bits);
				}
			}
			break;
		case Variant::etBitset256:
			{
				typ = 1562;//varbit
				fmt = 1;
				if(!v.isNull())
				{
					const std::bitset<256> &bs = v.as<Variant::Bitset256>();

					len = sizeof(PG_VarBit)-1+32;
					val = new char[len];
					PG_VarBit &vb = *(PG_VarBit *)val;
					own = true;

					vb.amount = bigEndian((boost::uint32_t)256);
					bitset2Bits(bs, vb.bits);
				}
			}
			break;
		case Variant::etBitset512:
			{
				typ = 1562;//varbit
				fmt = 1;
				if(!v.isNull())
				{
					const std::bitset<512> &bs = v.as<Variant::Bitset512>();

					len = sizeof(PG_VarBit)-1+64;
					val = new char[len];
					PG_VarBit &vb = *(PG_VarBit *)val;
					own = true;

					vb.amount = bigEndian((boost::uint32_t)512);
					bitset2Bits(bs, vb.bits);
				}
			}
			break;
		case Variant::etVectorChar:
			{
				typ = 17;//bytea
				fmt = 1;
				if(!v.isNull())
				{
					const std::vector<char> &vec = v.as<Variant::VectorChar>();
					len = (int)vec.size();
					val = new char[len];
					std::copy(vec.begin(), vec.end(), val);
					own = true;
				}
			}
			break;
		case Variant::etUuid:
			{
				typ = 2950;//uuid
				fmt = 1;
				if(!v.isNull())
				{
					const boost::uuids::uuid &u = v.as<Variant::Uuid>();
					len = 16;
					val = new char[len];
					std::copy(u.begin(), u.end(), val);
					own = true;
				}
			}
			break;

		case Variant::etChar:
			{
				typ = 18;//char
				fmt = 1;
				if(!v.isNull())
				{
					const char &c = v.as<Variant::Char>();
					len = 1;
					val = new char[1];
					*(char *)val = bigEndian(c);
					own = true;
				}
			}
			break;
		default:
			{
				return false;
				assert(!"unknown type for bind");
				typ = 0;
				fmt = 0;
				val = NULL;
				len = 0;
				own = false;
			}
			break;

		}

		return true;
	}



}}
