#ifndef _UTILS_IMPL_VARIANTCONVERTMATRIX_HPP_
#define _UTILS_IMPL_VARIANTCONVERTMATRIX_HPP_

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/foreach.hpp>
#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "utils/ntoa.hpp"
#include "utils/xton.hpp"
#include "utils/fixEndian.hpp"

namespace utils
{
	namespace impl
	{
		namespace convertor
		{
			//////////////////////////////////////////////////////////////////////////
			//default
			template <class Dst, class Src>
			bool exec(Dst &dst, const Src &src)
			{
			    //assert(0);
				return false;
			}
		}
	}
}





//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
namespace utils
{
	namespace impl
	{
		namespace convertor
		{

// 			//////////////////////////////////////////////////////////////////////////
// 			//identity, except VariantPtr
// 			template <class T>
// 				typename boost::enable_if<
// 					boost::mpl::not_<boost::is_same<Variant::VariantPtr, T> >
// 					, bool>::type
// 				exec(T &dst, const T &src)
// 			{
// 				dst = src;
// 				return true;
// 			}

			//warning C4244: '=' : conversion from 'const utils::Variant::UInt16' to 'utils::Variant::UInt8', possible loss of data
#pragma warning (disable: 4244)
			//warning C4267: '=' : conversion from 'size_t' to 'utils::Variant::Int8', possible loss of data
#pragma warning (disable: 4267)
			//warning C4800: 'const utils::Variant::Char' : forcing value to bool 'true' or 'false' (performance warning)
#pragma warning (disable: 4800)

			//////////////////////////////////////////////////////////////////////////
			//integer to integer
			//unsigned <- unsigned
			//unsigned <- signed
			//signed <- unsigned
			//signed <- signed
			//real to real
			//integer to real
			//real to integer
#define ASSIGN(DST, SRC)	template <>	bool exec(Variant::DST &dst, const Variant::SRC &src){dst = src;return true;}

#define MASSIGN(SRC)\
	ASSIGN(Bool, SRC)\
	ASSIGN(Char, SRC)\
	ASSIGN(Int8, SRC)\
	ASSIGN(Int16, SRC)\
	ASSIGN(Int32, SRC)\
	ASSIGN(Int64, SRC)\
	ASSIGN(UInt8, SRC)\
	ASSIGN(UInt16, SRC)\
	ASSIGN(UInt32, SRC)\
	ASSIGN(UInt64, SRC)\
	ASSIGN(Float, SRC)\
	ASSIGN(Double, SRC)\

			MASSIGN(Bool)
			MASSIGN(Char)
			MASSIGN(Int8)
			MASSIGN(Int16)
			MASSIGN(Int32)
			MASSIGN(Int64)
			MASSIGN(UInt8)
			MASSIGN(UInt16)
			MASSIGN(UInt32)
			MASSIGN(UInt64)
			MASSIGN(Float)
			MASSIGN(Double)

			//container to integer - size
			//container to real - size
#define CONTAINERSIZE(DST, SRC)	template <>	bool exec(Variant::DST &dst, const Variant::SRC &src){dst = src.size();return true;}
#define MCONTAINERSIZE(SRC)\
	CONTAINERSIZE(Bool, SRC)\
	CONTAINERSIZE(Char, SRC)\
	CONTAINERSIZE(Int8, SRC)\
	CONTAINERSIZE(Int16, SRC)\
	CONTAINERSIZE(Int32, SRC)\
	CONTAINERSIZE(Int64, SRC)\
	CONTAINERSIZE(UInt8, SRC)\
	CONTAINERSIZE(UInt16, SRC)\
	CONTAINERSIZE(UInt32, SRC)\
	CONTAINERSIZE(UInt64, SRC)\
	CONTAINERSIZE(Float, SRC)\
	CONTAINERSIZE(Double, SRC)\

			MCONTAINERSIZE(VectorChar)
			MCONTAINERSIZE(VectorVariant)
			MCONTAINERSIZE(MapStringVariant)
			MCONTAINERSIZE(MapVariantVariant)
			MCONTAINERSIZE(MultimapStringVariant)
			MCONTAINERSIZE(MultimapVariantVariant)
			MCONTAINERSIZE(SetVariant)
			MCONTAINERSIZE(MultisetVariant)
			MCONTAINERSIZE(DequeVariant)
			MCONTAINERSIZE(ListVariant)

















			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			struct BitsetHelper
			{
				//////////////////////////////////////////////////////////////////////////
				//bs < bs
				//////////////////////////////////////////////////////////////////////////
				template <size_t NDst, size_t NSrc>
				static
					typename boost::enable_if_c<
						(NDst == NSrc)
						, bool>::type
					bs2bs(std::bitset<NDst> &dst, const std::bitset<NSrc> &src)
				{
					dst = src;
					return true;
				}

				//////////////////////////////////////////////////////////////////////////
				template <size_t NDst, size_t NSrc>
				static
					typename boost::enable_if_c<
						(NDst < NSrc)
						, bool>::type
					bs2bs(std::bitset<NDst> &dst, const std::bitset<NSrc> &src)
				{
					for(size_t i(0); i<NDst; i++)
					{
						dst.set(i, src.test(i));
					}
					return true;
				}
				//////////////////////////////////////////////////////////////////////////
				template <size_t NDst, size_t NSrc>
				static
					typename boost::enable_if_c<
						(NDst > NSrc)
						, bool>::type
					bs2bs(std::bitset<NDst> &dst, const std::bitset<NSrc> &src)
				{
					dst.reset();
					for(size_t i(0); i<NSrc; i++)
					{
						dst.set(i, src.test(i));
					}
					return true;
				}

				//////////////////////////////////////////////////////////////////////////
				//int < bs
				//////////////////////////////////////////////////////////////////////////
				template <class Int, size_t NSrc>
				static bool bs2int(Int &dst, const std::bitset<NSrc> &src)
				{
					dst = 0;
					for(size_t i(0); i<std::min(sizeof(Int)*sizeof(char), NSrc); i++)
					{
						if(src.test(i))
						{
							dst |= Int(1)<<i;
						}
					}
					return true;
				}

				//////////////////////////////////////////////////////////////////////////
				//bs < int
				//////////////////////////////////////////////////////////////////////////
				template <size_t NDst, class Int>
				static bool int2bs(std::bitset<NDst> &dst, const Int &src)
				{
					dst.reset();
					for(size_t i(0); i<std::min(NDst, sizeof(Int)*sizeof(char)); i++)
					{
						dst.set(i, src & (Int(1)<<i));
					}
					return true;
				}






				//////////////////////////////////////////////////////////////////////////
				//bool < bs
				//////////////////////////////////////////////////////////////////////////
				template <size_t NSrc>
				static bool bs2int(Variant::Bool &dst, const std::bitset<NSrc> &src)
				{
					dst = src.test(0);
					return true;
				}

				//////////////////////////////////////////////////////////////////////////
				//bs < bool
				//////////////////////////////////////////////////////////////////////////
				template <size_t NDst>
				static bool int2bs(std::bitset<NDst> &dst, const Variant::Bool &src)
				{
					dst.reset();
					dst.set(0, src);
					return true;
				}

			};


			//bitset to bitset
#define BITSET2BITSET(DST, SRC)	template <>	bool exec(Variant::DST &dst, const Variant::SRC &src){return BitsetHelper::bs2bs(dst, src);}

#define MBITSET2BITSET(SRC)\
	BITSET2BITSET(Bitset8, SRC)\
	BITSET2BITSET(Bitset16, SRC)\
	BITSET2BITSET(Bitset32, SRC)\
	BITSET2BITSET(Bitset64, SRC)\
	BITSET2BITSET(Bitset128, SRC)\
	BITSET2BITSET(Bitset256, SRC)\
	BITSET2BITSET(Bitset512, SRC)\

			MBITSET2BITSET(Bitset8)
			MBITSET2BITSET(Bitset16)
			MBITSET2BITSET(Bitset32)
			MBITSET2BITSET(Bitset64)
			MBITSET2BITSET(Bitset128)
			MBITSET2BITSET(Bitset256)
			MBITSET2BITSET(Bitset512)

			//bitset to integer
			//integer to bitset
#define BITSET2INT(DST, SRC)\
	template <>	bool exec(Variant::DST &dst, const Variant::SRC &src){return BitsetHelper::bs2int(dst, src);}\
	template <>	bool exec(Variant::SRC &dst, const Variant::DST &src){return BitsetHelper::int2bs(dst, src);}\


#define MBITSET2INT(DST)\
	BITSET2INT(DST, Bitset8)\
	BITSET2INT(DST, Bitset16)\
	BITSET2INT(DST, Bitset32)\
	BITSET2INT(DST, Bitset64)\
	BITSET2INT(DST, Bitset128)\
	BITSET2INT(DST, Bitset256)\
	BITSET2INT(DST, Bitset512)\

			MBITSET2INT(Bool)
			MBITSET2INT(Char)
			MBITSET2INT(Int8)
			MBITSET2INT(Int16)
			MBITSET2INT(Int32)
			MBITSET2INT(Int64)
			MBITSET2INT(UInt8)
			MBITSET2INT(UInt16)
			MBITSET2INT(UInt32)
			MBITSET2INT(UInt64)












			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//sequence to sequence
			struct SequenceHelper
			{
				//////////////////////////////////////////////////////////////////////////
				template <class Dst, class Src>
				static
					typename boost::enable_if<
						boost::is_same<Dst, Src>
						, bool>::type
					convert(Dst &dst, const Src &src)
				{
					dst = src;
					return true;
				}

				//////////////////////////////////////////////////////////////////////////
				template <class Dst, class Src>
				static
					typename boost::enable_if<
						boost::mpl::and_<
							boost::mpl::not_<boost::is_same<Dst, Src> >,
							boost::mpl::not_<
								boost::mpl::or_<
									boost::is_same<Dst, Variant::SetVariant>,
									boost::is_same<Dst, Variant::MultisetVariant>
								>
							>
						>
						, bool>::type
					convert(Dst &dst, const Src &src)
				{
					dst.clear();
					std::copy (src.begin(), src.end(), std::back_inserter(dst));
					return true;
				}
				//////////////////////////////////////////////////////////////////////////
				template <class Dst, class Src>
				static
					typename boost::enable_if<
						boost::mpl::and_<
							boost::mpl::not_<boost::is_same<Dst, Src> >,
							boost::mpl::or_<
								boost::is_same<Dst, Variant::SetVariant>,
								boost::is_same<Dst, Variant::MultisetVariant>
							>
						>
						, bool>::type
					convert(Dst &dst, const Src &src)
				{
					dst.clear();
					std::copy (src.begin(), src.end(), std::inserter(dst, dst.begin()));
					return true;
				}
			};

#define SEQUENCE2SEQUENCE(DST, SRC)\
	template <>	bool exec(Variant::DST &dst, const Variant::SRC &src){return SequenceHelper::convert(dst, src);}\

#define MSEQUENCE2SEQUENCE(SRC)\
	SEQUENCE2SEQUENCE(SRC, VectorVariant)\
	SEQUENCE2SEQUENCE(SRC, SetVariant)\
	SEQUENCE2SEQUENCE(SRC, MultisetVariant)\
	SEQUENCE2SEQUENCE(SRC, DequeVariant)\
	SEQUENCE2SEQUENCE(SRC, ListVariant)\

			MSEQUENCE2SEQUENCE(VectorVariant)
			MSEQUENCE2SEQUENCE(SetVariant)
			MSEQUENCE2SEQUENCE(MultisetVariant)
			MSEQUENCE2SEQUENCE(DequeVariant)
			MSEQUENCE2SEQUENCE(ListVariant)













			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//assoc to assoc
			struct AssocHelper
			{
				//////////////////////////////////////////////////////////////////////////
				template <class Dst, class Src>
				static
					typename boost::enable_if<
						boost::is_same<Dst, Src>
						, bool>::type
					convert(Dst &dst, const Src &src)
				{
					dst = src;
					return true;
				}

				//////////////////////////////////////////////////////////////////////////
				template <class Dst, class Src>
				static
					typename boost::enable_if<
						boost::mpl::not_<boost::is_same<Dst, Src> >
						, bool>::type
					convert(Dst &dst, const Src &src)
				{
					dst.clear();
					std::copy (src.begin(), src.end(), std::inserter(dst, dst.begin()));
					return true;
				}
			};
#define ASSOC2ASSOC(DST, SRC)\
	template <>	bool exec(Variant::DST &dst, const Variant::SRC &src){return AssocHelper::convert(dst, src);}\

#define MASSOC2ASSOC(SRC)\
	ASSOC2ASSOC(MapStringVariant, SRC)\
	ASSOC2ASSOC(MapVariantVariant, SRC)\
	ASSOC2ASSOC(MultimapStringVariant, SRC)\
	ASSOC2ASSOC(MultimapVariantVariant, SRC)\

			MASSOC2ASSOC(MapStringVariant)
			MASSOC2ASSOC(MapVariantVariant)
			MASSOC2ASSOC(MultimapStringVariant)
			MASSOC2ASSOC(MultimapVariantVariant)
















			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//assoc to seq
			//seq to assoc
			struct Assoc2SeqHelper
			{
				//////////////////////////////////////////////////////////////////////////
				template <class Dst, class Src>
				static bool fwd(Dst &dst, const Src &src)
				{
					dst.clear();
					std::transform(src.begin(), src.end(),
						std::inserter(dst, dst.begin()),
						boost::bind(&Src::value_type::second,_1));
					return true;
				}
				//////////////////////////////////////////////////////////////////////////
				template <class Dst, class Src>
				static bool bwd(Dst &dst, const Src &src)
				{
					dst.clear();

					size_t idx(0);
					BOOST_FOREACH(typename Src::const_reference s, src)
					{
						char tmp[32];
						dst.insert(std::make_pair(utils::_ntoa(idx++, tmp), s));
					}
					return true;
				}
			};
#define ASSOC2SEQ(DST, SRC)\
	template <>	bool exec(Variant::DST &dst, const Variant::SRC &src){return Assoc2SeqHelper::fwd(dst, src);}\
	template <>	bool exec(Variant::SRC &dst, const Variant::DST &src){return Assoc2SeqHelper::bwd(dst, src);}\

#define MASSOC2SEQ(SRC)\
	ASSOC2SEQ(VectorVariant, SRC)\
	ASSOC2SEQ(SetVariant, SRC)\
	ASSOC2SEQ(MultisetVariant, SRC)\
	ASSOC2SEQ(DequeVariant, SRC)\
	ASSOC2SEQ(ListVariant, SRC)\

			MASSOC2SEQ(MapStringVariant)
			MASSOC2SEQ(MapVariantVariant)
			MASSOC2SEQ(MultimapStringVariant)
			MASSOC2SEQ(MultimapVariantVariant)










			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//ptr to some
			template <class Dst>
				typename boost::enable_if<
					boost::mpl::not_<boost::is_same<Dst, Variant::VariantPtr> >
					, bool>::type
				exec(Dst &dst, const Variant::VariantPtr &src)
			{
				if(src)
				{
					dst = (const Dst&)*src;
					return true;
				}
				return false;
			}
			//some to ptr
			template <class Src>
				typename boost::enable_if<
					boost::mpl::not_<boost::is_same<Variant::VariantPtr, Src> >
					, bool>::type
				exec(Variant::VariantPtr &dst, const Src &src)
			{
				if(dst)
				{
					*dst = src;
				}
				else
				{
					dst.reset(new Variant(src));
				}
				return true;
			}
			//ptr to ptr
			template <> bool exec<Variant::VariantPtr, Variant::VariantPtr>(Variant::VariantPtr &dst, const Variant::VariantPtr &src)
			{
				if(src)
				{
					if(dst)
					{
						*dst = *src;
					}
					else
					{
						dst.reset(new Variant(*src));
					}
				}
				else
				{
					dst.reset();
				}
				return true;
			}

















			//datetime to datetime
// 			Date;
// 			Datetime;
// 			Tm;
//
// 			DateDuration;
// 			TimeDuration;
// 			DateTimeDuration;

#define CNVTCODE(DST, SRC, CODE) template <> bool exec(Variant::DST &dst, const Variant::SRC &src){CODE;return true;}
			CNVTCODE(Date, Datetime, dst = src.date())
			CNVTCODE(Date, Tm, try{dst = boost::gregorian::date_from_tm(src);}catch(const std::out_of_range &){dst=Variant::Date();})

			CNVTCODE(Datetime, Date, dst = Variant::Datetime(src))
			CNVTCODE(Datetime, Tm, try{dst = boost::posix_time::ptime_from_tm(src);}catch(const std::out_of_range &){dst=Variant::Datetime();})

			CNVTCODE(Tm, Date, try{dst = boost::gregorian::to_tm(src);}catch(const std::out_of_range &){dst=Variant::Tm();})
			CNVTCODE(Tm, Datetime, try{dst = boost::posix_time::to_tm(src);}catch(const std::out_of_range &){dst=Variant::Tm();})


			CNVTCODE(DateDuration, DateTimeDuration, dst = src._dd)
			CNVTCODE(TimeDuration, DateTimeDuration, dst = src._td)

			CNVTCODE(DateTimeDuration, DateDuration, dst._dd = src;dst._td=Variant::TimeDuration())
			CNVTCODE(DateTimeDuration, TimeDuration, dst._td = src;dst._dd=Variant::DateDuration())






			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//some to string
			//string to some
#define SOME2STRING(SRC)\
	template <>	bool exec(Variant::String &dst, const Variant::SRC &src){try{dst = boost::lexical_cast<Variant::String>(src);}catch(const boost::bad_lexical_cast&){dst.clear();};return true;}\
	template <>	bool exec(Variant::SRC &dst, const Variant::String &src){if(src.empty()){dst=Variant::SRC();return true;}try{dst = boost::lexical_cast<Variant::SRC>(src);}catch(const boost::bad_lexical_cast&){dst=Variant::SRC();};return true;}\

			SOME2STRING(Char)
			SOME2STRING(Int8)
			SOME2STRING(Int16)
			SOME2STRING(Int32)
			SOME2STRING(Int64)
			SOME2STRING(UInt8)
			SOME2STRING(UInt16)
			SOME2STRING(UInt32)
			SOME2STRING(UInt64)
			SOME2STRING(Float)
			SOME2STRING(Double)
			SOME2STRING(Date)
			SOME2STRING(Datetime)
			SOME2STRING(Bool)
			SOME2STRING(Bitset8)
			SOME2STRING(Bitset16)
			SOME2STRING(Bitset32)
			SOME2STRING(Bitset64)
			SOME2STRING(Bitset128)
			SOME2STRING(Bitset256)
			SOME2STRING(Bitset512)
			SOME2STRING(DateDuration)
			SOME2STRING(TimeDuration)
			//SOME2STRING(DateTimeDuration)
			//SOME2STRING(Uuid)

			//dateTimeDuration
			template <> bool exec(Variant::String &dst, const Variant::DateTimeDuration &src)
			{
				std::ostringstream os;
				os<<src._dd<<" "<<src._td;
				dst = os.str();
				return true;
			}

			template <> bool exec(Variant::DateTimeDuration &dst, const Variant::String &src)
			{
				std::istringstream is(src);
				is>>dst._dd>>dst._td;
				return true;
			}

			//tm
			template <> bool exec(Variant::String &dst, const Variant::Tm &src)
			{
				Variant::Datetime dt;
				exec(dt, src);
				return exec(dst, dt);
			}

			template <> bool exec(Variant::Tm &dst, const Variant::String &src)
			{
				Variant::Datetime dt;
				exec(dt, src);
				return exec(dst, dt);
			}

			//uuid
			template <> bool exec(Variant::String &dst, const Variant::Uuid &src)
			{
				boost::uint32_t time_low = litEndian(*(boost::uint32_t *)(src.begin()+0));
				boost::uint16_t time_mid = litEndian(*(boost::uint16_t *)(src.begin()+4));
				boost::uint16_t time_hi_and_version = litEndian(*(boost::uint16_t *)(src.begin()+6));

				char buf[37];
				sprintf(buf, "%8.8x-%4.4x-%4.4x-%2.2x%2.2x-",
					time_low, time_mid, time_hi_and_version,
					src.begin()[8], src.begin()[9]);

				for(size_t i = 0; i < 6; i++)
				{
					sprintf(buf+24+i*2, "%2.2x", src.begin()[10+i]);
				}
				dst = buf;
				return true;
			}

			template <> bool exec(Variant::Uuid &dst, const Variant::String &src)
			{
				Variant::Uuid u;
				if(src.size() == 36)
				{
					for(size_t i(0), si(0); i<16; i++)
					{
						switch(i)
						{
						case 4:
						case 6:
						case 8:
						case 10:
							si++;
						}

						boost::uint8_t &b = *(u.begin()+i);

						signed char d1 = x2n(src[si++]);
						signed char d2 = x2n(src[si++]);

						if(d1<0 || d2 < 0)
						{
							return false;
						}

						b = d1<<4 | d2;
					}
				}
				else if(src.size() == 32)
				{
					for(size_t i(0); i<16; i++)
					{
						boost::uint8_t &b = *(u.begin()+i);

						signed char d1 = x2n(src[i*2]);
						signed char d2 = x2n(src[i*2+1]);

						if(d1<0 || d2 < 0)
						{
							return false;
						}

						b = d1<<4 | d2;
					}
				}

				*(boost::uint32_t *)(u.begin()+0) = fixEndian(*(boost::uint32_t *)(u.begin()+0));
				*(boost::uint16_t *)(u.begin()+4) = fixEndian(*(boost::uint16_t *)(u.begin()+4));
				*(boost::uint16_t *)(u.begin()+6) = fixEndian(*(boost::uint16_t *)(u.begin()+6));

				dst = u;
				return true;
			}



			//vectorChar
			template <> bool exec(Variant::String &dst, const Variant::VectorChar &src)
			{
				dst.assign(src.begin(), src.end());
				return true;
			}

			template <> bool exec(Variant::VectorChar &dst, const Variant::String &src)
			{
				dst.assign(src.begin(), src.end());
				return true;
			}







#define CNVTIDENTITY(T) template <> bool exec(Variant::T &dst, const Variant::T &src){dst=src;return true;}
	CNVTIDENTITY(String)
	CNVTIDENTITY(VectorChar)
	CNVTIDENTITY(Date)
	CNVTIDENTITY(Datetime)
	CNVTIDENTITY(DateDuration)
	CNVTIDENTITY(TimeDuration)
	CNVTIDENTITY(DateTimeDuration)
	CNVTIDENTITY(Tm)
	CNVTIDENTITY(Uuid)
		}
	}
}


namespace utils
{
	namespace impl
	{
		//////////////////////////////////////////////////////////////////////////
		template <class Dst, class Src>
		bool variantConvertMatrix(Dst &dst, const Src &src)
		{
			return convertor::exec(dst, src);
		}
	}
}

#endif
