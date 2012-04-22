#include "pch.hpp"
#include "utils/variant.hpp"
#include "utils/fixEndian.hpp"

#include <boost/crc.hpp>
#include <boost/foreach.hpp>
#include <boost/date_time.hpp>

namespace utils
{
	using namespace boost;
	namespace
	{
		template <uint16_t signature>
		class Serializer
		{
			typedef std::map<VariantPtr, uint64_t> TMPtrKey;
			TMPtrKey _mPtrKey;

			typedef std::map<uint64_t, VariantPtr> TMKeyPtr;
			TMKeyPtr _mKeyPtr;

#pragma pack (push, 1)
			struct Header
			{
				uint16_t	_signature;//некая магическая константа
				uint16_t	_crc;//покрывает _size и все данные
				uint64_t	_size;//размер данных
			};
#pragma pack (pop)

			char *_bufIter;
			char *_bufEnd;

		public:
			//////////////////////////////////////////////////////////////////////////
			Serializer()
				: _bufIter(NULL)
				, _bufEnd(NULL)
			{

			}

			//////////////////////////////////////////////////////////////////////////
			~Serializer()
			{

			}

			//////////////////////////////////////////////////////////////////////////
			shared_array<char> serialize(const Variant &v, size_t &size)
			{
				//первый проход, вычислить длину
				uint64_t vsize = calcSize(v);
				_mPtrKey.clear();

				shared_array<char> data(new char[sizeof(Header) + vsize]);

				Header *ph = (Header *)data.get();
				ph->_signature = litEndian(signature);
				ph->_size = litEndian(vsize);

				_bufIter = data.get() + sizeof(Header);
				_bufEnd = _bufIter + vsize;
				write(v);
				_mPtrKey.clear();
				assert(_bufIter == _bufEnd);
				_bufIter = NULL;
				_bufEnd = NULL;

				crc_16_type		crcCalculator;
				crcCalculator.process_bytes(data.get()+4, vsize+4);
				ph->_crc = litEndian(crcCalculator.checksum());
				size = vsize+sizeof(Header);

				return data;
			}

			//////////////////////////////////////////////////////////////////////////
			bool deserialize(Variant &v, shared_array<char> data, size_t size)
			{
				if(size < sizeof(Header)+2)
				{
					assert(!"size mismatch");
					return false;
				}

				Header *ph = (Header *)data.get();

				if(litEndian(ph->_signature) != signature)
				{
					assert(!"signature mismatch");
					return false;
				}

				int64_t vsize = litEndian(ph->_size);
				if(vsize != size - sizeof(Header))
				{
					assert(!"size mismatch");
					return false;
				}

				crc_16_type		crcCalculator;
				crcCalculator.process_bytes(data.get()+4, vsize+4);
				if(litEndian(ph->_crc) != crcCalculator.checksum())
				{
					assert(!"crc mismatch");
					return false;
				}

				_bufIter = data.get() + sizeof(Header);
				_bufEnd = _bufIter + vsize;
				assert(_mKeyPtr.empty());
				read(v);
				_mKeyPtr.clear();
				if(_bufIter != _bufEnd)
				{
					assert(!"data corrupted");
					return false;
				}
				_bufIter = NULL;
				_bufEnd = NULL;

				return true;
			}

		private:
			//////////////////////////////////////////////////////////////////////////
			size_t calcSize(const Variant &v)
			{
				//_et
				size_t size = 2;

				//размер размера
				static const size_t S = sizeof(uint64_t);

				if(v.isNull())
				{
					//ничего
				}
				else
				{
					switch(v.type())
					{
					case Variant::etUnknown:
						assert(!"bad et");
						break;
					case Variant::etVoid:
						break;
					case Variant::etString:
						size += S;
						size += v.as<Variant::String>().size();
						break;
					case Variant::etFloat:
						size += 4;
						break;
					case Variant::etDouble:
						size += 8;
						break;
					case Variant::etInt8:
						size += 1;
						break;
					case Variant::etInt16:
						size += 2;
						break;
					case Variant::etInt32:
						size += 4;
						break;
					case Variant::etInt64:
						size += 8;
						break;
					case Variant::etUInt8:
						size += 1;
						break;
					case Variant::etUInt16:
						size += 2;
						break;
					case Variant::etUInt32:
						size += 4;
						break;
					case Variant::etUInt64:
						size += 8;
						break;
					case Variant::etVectorChar:
						size += S;
						size += v.as<Variant::VectorChar>().size();
						break;
					case Variant::etDate:
						size += 4;
						break;
					case Variant::etDatetime:
						size += 12;
						break;
					case Variant::etVectorVariant:
						size += S;
						BOOST_FOREACH(const Variant &el, v.as<Variant::VectorVariant>())
						{
							size += calcSize(el);
						}
						break;
					case Variant::etMapStringVariant:
						size += S;
						BOOST_FOREACH(const Variant::MapStringVariant::value_type &el, v.as<Variant::MapStringVariant>())
						{
							//key
							size += S;
							size += el.first.size();
							//value
							size += calcSize(el.second);
						}
						break;
					case Variant::etBool:
						size += 1;
						break;
					case Variant::etTm:
						size += 9*4;
						break;
					case Variant::etBitset8:
						size += 1;
						break;
					case Variant::etBitset16:
						size += 2;
						break;
					case Variant::etBitset32:
						size += 4;
						break;
					case Variant::etBitset64:
						size += 8;
						break;
					case Variant::etBitset128:
						size += 16;
						break;
					case Variant::etBitset256:
						size += 32;
						break;
					case Variant::etBitset512:
						size += 64;
						break;
					case Variant::etDateDuration:
						size += 4;
						break;
					case Variant::etTimeDuration:
						size += 8;
						break;
					case Variant::etDateTimeDuration:
						size += 12;
						break;
					case Variant::etMapVariantVariant:
						size += S;
						BOOST_FOREACH(const Variant::MapVariantVariant::value_type &el, v.as<Variant::MapVariantVariant>())
						{
							//key
							size += calcSize(el.first);
							//value
							size += calcSize(el.second);
						}
						break;
					case Variant::etMultimapStringVariant:
						size += S;
						BOOST_FOREACH(const Variant::MultimapStringVariant::value_type &el, v.as<Variant::MultimapStringVariant>())
						{
							//key
							size += S;
							size += el.first.size();
							//value
							size += calcSize(el.second);
						}
						break;
					case Variant::etMultimapVariantVariant:
						size += S;
						BOOST_FOREACH(const Variant::MultimapVariantVariant::value_type &el, v.as<Variant::MultimapVariantVariant>())
						{
							//key
							size += calcSize(el.first);
							//value
							size += calcSize(el.second);
						}
						break;
					case Variant::etSetVariant:
						size += S;
						BOOST_FOREACH(const Variant &el, v.as<Variant::SetVariant>())
						{
							size += calcSize(el);
						}
						break;
					case Variant::etMultisetVariant:
						size += S;
						BOOST_FOREACH(const Variant &el, v.as<Variant::MultisetVariant>())
						{
							size += calcSize(el);
						}
						break;
					case Variant::etDequeVariant:
						size += S;
						BOOST_FOREACH(const Variant &el, v.as<Variant::DequeVariant>())
						{
							size += calcSize(el);
						}
						break;
					case Variant::etListVariant:
						size += S;
						BOOST_FOREACH(const Variant &el, v.as<Variant::ListVariant>())
						{
							size += calcSize(el);
						}
						break;
					case Variant::etChar:
						size += 1;
						break;
					case Variant::etUuid:
						size += 16;
						break;
					case Variant::etVariantPtr:
						size += 4;//key
						{
							const Variant::VariantPtr &p = v.as<Variant::VariantPtr>();
							if(p)
							{
								TMPtrKey::iterator iter = _mPtrKey.find(p);
								if(_mPtrKey.end() == iter)
								{
									//сформировать новый ключ и запомнить его в карте
									_mPtrKey.insert(std::make_pair(p, _mPtrKey.size()+1));
									size += calcSize(*p);
								}
								else
								{
									//уже был
								}
							}
						}
						break;
					default:
						assert(0);
					}
				}
				return size;
			}

			//////////////////////////////////////////////////////////////////////////
			void writeBinary(const char *data, size_t size)
			{
				assert(_bufEnd-_bufIter >= (int)size);
				memcpy(_bufIter, data, size);
				_bufIter += size;
			}

			//////////////////////////////////////////////////////////////////////////
			template <class T>
			void writeIntegral(const T &value)
			{
				assert(_bufEnd-_bufIter >= sizeof(T));
				T v2 = litEndian(value);
				writeBinary((const char *)&v2, sizeof(T));
			}

			//////////////////////////////////////////////////////////////////////////
			void write(const std::string &value)
			{
				uint64_t size = (uint64_t)value.size();
				writeIntegral(size);
				if(size)
				{
					writeBinary(value.data(), size);
				}
			}

			//////////////////////////////////////////////////////////////////////////
			template <size_t N>
			void writeBitset(const std::bitset<N> &value)
			{
				assert(_bufEnd-_bufIter >= N/8);

				uint8_t buf[N/8] = {};
				for(size_t i(0); i<N; i++)
				{
					if(value.test(i))
					{
						buf[i/8] |= 1<<(i%8);
					}
				}
				writeBinary((const char *)buf, N/8);
			}

			//////////////////////////////////////////////////////////////////////////
			template <class Map>
			void writeMap(const Map &value)
			{
				uint64_t size = (uint64_t)value.size();
				writeIntegral(size);
				if(size)
				{
					BOOST_FOREACH(const typename Map::value_type &el, value)
					{
						//key
						write(el.first);
						//value
						write(el.second);
					}
				}
			}

			//////////////////////////////////////////////////////////////////////////
			template <class Seq>
			void writeSeq(const Seq &value)
			{
				uint64_t size = (uint64_t)value.size();
				writeIntegral(size);
				if(size)
				{
					BOOST_FOREACH(const typename Seq::value_type &el, value)
					{
						write(el);
					}
				}
			}


			//////////////////////////////////////////////////////////////////////////
			void write(const Variant &v)
			{
				int16_t et = v.type();
				if(v.isNull())
				{
					et = -et;
				}
				writeIntegral(et);

				if(!v.isNull())
				{
					switch(v.type())
					{
					case Variant::etUnknown:
						assert(!"bad et");
						break;
					case Variant::etVoid:
						break;
					case Variant::etString:
						write(v.as<Variant::String>());
						break;
					case Variant::etFloat:
						writeIntegral(v.as<Variant::Float>());
						break;
					case Variant::etDouble:
						writeIntegral(v.as<Variant::Double>());
						break;
					case Variant::etInt8:
						writeIntegral(v.as<Variant::Int8>());
						break;
					case Variant::etInt16:
						writeIntegral(v.as<Variant::Int16>());
						break;
					case Variant::etInt32:
						writeIntegral(v.as<Variant::Int32>());
						break;
					case Variant::etInt64:
						writeIntegral(v.as<Variant::Int64>());
						break;
					case Variant::etUInt8:
						writeIntegral(v.as<Variant::UInt8>());
						break;
					case Variant::etUInt16:
						writeIntegral(v.as<Variant::UInt16>());
						break;
					case Variant::etUInt32:
						writeIntegral(v.as<Variant::UInt32>());
						break;
					case Variant::etUInt64:
						writeIntegral(v.as<Variant::UInt64>());
						break;
					case Variant::etVectorChar:
						{
							const Variant::VectorChar &raw = v.as<Variant::VectorChar>();
							uint64_t size = (uint64_t)raw.size();
							writeIntegral(size);
							if(size)
							{
								writeBinary(&raw[0], size);
							}
						}
						break;
					case Variant::etDate:
						{
							const Variant::Date &raw = v.as<Variant::Date>();
							uint32_t ui4 =
								raw.year()*10000 +
								raw.month()*100 +
								raw.day();
							writeIntegral(ui4);
						}
						break;
					case Variant::etDatetime:
						{
							const Variant::Datetime &raw = v.as<Variant::Datetime>();
							const Variant::Date d = raw.date();
							const Variant::TimeDuration td = raw.time_of_day();

							uint32_t ui4_d =
								d.year()*10000 +
								d.month()*100 +
								d.day();
							writeIntegral(ui4_d);

							writeIntegral((int64_t)td.total_microseconds());
						}
						break;
					case Variant::etVectorVariant:
						writeSeq(v.as<Variant::VectorVariant>());
						break;
					case Variant::etMapStringVariant:
						writeMap(v.as<Variant::MapStringVariant>());
						break;
					case Variant::etBool:
						writeIntegral((char)v.as<Variant::Bool>());
						break;
					case Variant::etTm:
						{
							const Variant::Tm &raw = v.as<Variant::Tm>();

							writeIntegral((int32_t)raw.tm_sec);     /* seconds after the minute - [0,59] */
							writeIntegral((int32_t)raw.tm_min);     /* minutes after the hour - [0,59] */
							writeIntegral((int32_t)raw.tm_hour);    /* hours since midnight - [0,23] */
							writeIntegral((int32_t)raw.tm_mday);    /* day of the month - [1,31] */
							writeIntegral((int32_t)raw.tm_mon);     /* months since January - [0,11] */
							writeIntegral((int32_t)raw.tm_year);    /* years since 1900 */
							writeIntegral((int32_t)raw.tm_wday);    /* days since Sunday - [0,6] */
							writeIntegral((int32_t)raw.tm_yday);    /* days since January 1 - [0,365] */
							writeIntegral((int32_t)raw.tm_isdst);   /* daylight savings time flag */
						}
						break;
					case Variant::etBitset8:
						writeBitset(v.as<Variant::Bitset8>());
						break;
					case Variant::etBitset16:
						writeBitset(v.as<Variant::Bitset16>());
						break;
					case Variant::etBitset32:
						writeBitset(v.as<Variant::Bitset32>());
						break;
					case Variant::etBitset64:
						writeBitset(v.as<Variant::Bitset64>());
						break;
					case Variant::etBitset128:
						writeBitset(v.as<Variant::Bitset128>());
						break;
					case Variant::etBitset256:
						writeBitset(v.as<Variant::Bitset256>());
						break;
					case Variant::etBitset512:
						writeBitset(v.as<Variant::Bitset512>());
						break;
					case Variant::etDateDuration:
						writeIntegral((int32_t)v.as<Variant::DateDuration>().days());
						break;
					case Variant::etTimeDuration:
						writeIntegral((int64_t)v.as<Variant::TimeDuration>().total_microseconds());
						break;
					case Variant::etDateTimeDuration:
						{
							const Variant::DateTimeDuration &raw = v.as<Variant::DateTimeDuration>();
							writeIntegral((int32_t)raw._dd.days());
							writeIntegral((int64_t)raw._td.total_microseconds());
						}
						break;
					case Variant::etMapVariantVariant:
						writeMap(v.as<Variant::MapVariantVariant>());
						break;
					case Variant::etMultimapStringVariant:
						writeMap(v.as<Variant::MultimapStringVariant>());
						break;
					case Variant::etMultimapVariantVariant:
						writeMap(v.as<Variant::MultimapVariantVariant>());
						break;
					case Variant::etSetVariant:
						writeSeq(v.as<Variant::SetVariant>());
						break;
					case Variant::etMultisetVariant:
						writeSeq(v.as<Variant::MultisetVariant>());
						break;
					case Variant::etDequeVariant:
						writeSeq(v.as<Variant::DequeVariant>());
						break;
					case Variant::etListVariant:
						writeSeq(v.as<Variant::ListVariant>());
						break;
					case Variant::etChar:
						writeIntegral(v.as<Variant::Char>());
						break;
					case Variant::etUuid:
						{
							const Variant::Uuid &raw = v.as<Variant::Uuid>();
							writeBinary((char *)raw.data, 16);
						}
						break;
					case Variant::etVariantPtr:
						{
							const Variant::VariantPtr &p = v.as<Variant::VariantPtr>();
							if(p)
							{
								TMPtrKey::iterator iter = _mPtrKey.find(p);
								if(_mPtrKey.end() == iter)
								{
									//сформировать новый ключ и запомнить его в карте
									uint64_t key = (uint64_t)_mPtrKey.size()+1;
									_mPtrKey.insert(std::make_pair(p,key));

									writeIntegral(key);
									write(*p);
								}
								else
								{
									//уже был
									writeIntegral(iter->second);
								}
							}
							else
							{
								uint64_t key = 0;
								writeIntegral(key);
							}
						}
						break;
					default:
						assert(0);
					}
				}
			}

























			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			bool readBinary(char *data, size_t size)
			{
				if(_bufEnd-_bufIter < (int)size)
				{
					assert(0);
					return false;
				}

				memcpy(data, _bufIter, size);
				_bufIter += size;

				return true;
			}

			//////////////////////////////////////////////////////////////////////////
			template <class T>
			bool readIntegral(T &value)
			{
				if(_bufEnd-_bufIter < (ptrdiff_t)sizeof(T))
				{
					assert(0);
					return false;
				}

				readBinary((char *)&value, sizeof(T));
				value = litEndian(value);

				return true;
			}

			//////////////////////////////////////////////////////////////////////////
			bool read(std::string &value)
			{
				uint64_t size;
				if(!readIntegral(size))
				{
					return false;
				}
				if(size)
				{
					value.resize(size);
					if(!readBinary(const_cast<char *>(value.data()), size))
					{
						return false;
					}
				}
				else
				{
					value.clear();
				}
				return true;
			}

			//////////////////////////////////////////////////////////////////////////
			template <size_t N>
			bool readBitset(std::bitset<N> &value)
			{
				uint8_t buf[N/8];
				if(!readBinary((char *)buf, N/8))
				{
					return false;
				}

				value.reset();
				for(size_t i(0); i<N; i++)
				{
					if(buf[i/8] & 1<<(i%8))
					{
						value.set(i);
					}
				}

				return true;
			}

			//////////////////////////////////////////////////////////////////////////
			template <class Map>
			bool readMap(Map &value)
			{
				value.clear();
				uint64_t size;
				if(!readIntegral(size))
				{
					return false;
				}

				while(size)
				{
					std::pair<typename Map::key_type, typename Map::mapped_type> el;
					if(!read(el.first) || !read(el.second))
					{
						return false;
					}

					value.insert(el);
					size--;
				}
				return true;
			}

			//////////////////////////////////////////////////////////////////////////
			template <class Seq>
			bool readSeq_insertable(Seq &value)
			{
				value.clear();
				uint64_t size;
				if(!readIntegral(size))
				{
					return false;
				}
				while(size)
				{
					typename Seq::iterator iter = value.insert(value.end(), Variant());
					if(!read((Variant &)*iter))
					{
						return false;
					}
					size--;
				}

				return true;
			}


			//////////////////////////////////////////////////////////////////////////
			template <class Seq>
			bool readSeq_indexable(Seq &value)
			{
				uint64_t size;
				if(!readIntegral(size))
				{
					return false;
				}

				value.resize(size);
				for(size_t i(0); i<size; i++)
				{
					if(!read(value[i]))
					{
						return false;
					}
				}
				return true;
			}

			//////////////////////////////////////////////////////////////////////////
			bool read(Variant &v)
			{
				int16_t et;
				if(!readIntegral(et))
				{
					return false;
				}

				if(et<0)
				{
					switch(-et)
					{
					case Variant::etVoid: v.setNull(); return true;
#define ENUM_VARIANT_TYPE(T) case Variant::et##T: v.setNull<Variant::T>(); return true;
						ENUM_VARIANT_TYPES
#undef ENUM_VARIANT_TYPE
					default:
						assert(!"bad et");
						return false;
					}
					return false;
				}

				switch(et)
				{
				case Variant::etUnknown:
					assert(!"bad et");
					return false;
				case Variant::etVoid:
					v.setNull<Variant::Void>(true);
					return false;
				case Variant::etString:
					return read(v.as<Variant::String>(true));
				case Variant::etFloat:
					return readIntegral(v.as<Variant::Float>(true));
				case Variant::etDouble:
					return readIntegral(v.as<Variant::Double>(true));
				case Variant::etInt8:
					return readIntegral(v.as<Variant::Int8>(true));
				case Variant::etInt16:
					return readIntegral(v.as<Variant::Int16>(true));
				case Variant::etInt32:
					return readIntegral(v.as<Variant::Int32>(true));
				case Variant::etInt64:
					return readIntegral(v.as<Variant::Int64>(true));
				case Variant::etUInt8:
					return readIntegral(v.as<Variant::UInt8>(true));
				case Variant::etUInt16:
					return readIntegral(v.as<Variant::UInt16>(true));
				case Variant::etUInt32:
					return readIntegral(v.as<Variant::UInt32>(true));
				case Variant::etUInt64:
					return readIntegral(v.as<Variant::UInt64>(true));
				case Variant::etVectorChar:
					{
						uint64_t size;
						if(!readIntegral(size))
						{
							return false;
						}
						Variant::VectorChar &raw = v.as<Variant::VectorChar>(true);
						raw.resize(size);
						if(size)
						{
							if(!readBinary(&raw[0], size))
							{
								return false;
							}
						}
					}
					return true;
				case Variant::etDate:
					{
						uint32_t ui4;
						if(!readIntegral(ui4))
						{
							return false;
						}

						uint16_t y,m,d;
						y = ui4/10000;
						ui4 %= 10000;
						m = ui4/100;
						ui4 %= 100;
						d = ui4;

						try
						{
							v = Variant::Date(y,m,d);
						}
						catch(...)
						{
							return false;
						}
					}
					return true;
				case Variant::etDatetime:
					{
						uint32_t ui4_d;
						int64_t i8_t;
						if(!readIntegral(ui4_d) || !readIntegral(i8_t))
						{
							return false;
						}

						uint16_t y,m,d;
						y = ui4_d/10000;
						ui4_d %= 10000;
						m = ui4_d/100;
						ui4_d %= 100;
						d = ui4_d;

						try
						{
							v = Variant::Datetime(
								Variant::Date(y,m,d),
								posix_time::microseconds(i8_t));
						}
						catch(...)
						{
							return false;
						}
					}
					return true;
				case Variant::etVectorVariant:
					return readSeq_indexable(v.as<Variant::VectorVariant>(true));
				case Variant::etMapStringVariant:
					return readMap(v.as<Variant::MapStringVariant>(true));
				case Variant::etBool:
					{
						char c;
						if(!readIntegral(c))
						{
							return false;
						}
						v = c?true:false;
					}
					return true;
				case Variant::etTm:
					{
						int32_t data[9];

						if(
							!readIntegral(data[0]) ||
							!readIntegral(data[1]) ||
							!readIntegral(data[2]) ||
							!readIntegral(data[3]) ||
							!readIntegral(data[4]) ||
							!readIntegral(data[5]) ||
							!readIntegral(data[6]) ||
							!readIntegral(data[7]) ||
							!readIntegral(data[8]))
						{
							return false;
						}

						Variant::Tm &raw = v.as<Variant::Tm>(true);
						raw.tm_sec		= data[0];
						raw.tm_min		= data[1];
						raw.tm_hour		= data[2];
						raw.tm_mday		= data[3];
						raw.tm_mon		= data[4];
						raw.tm_year		= data[5];
						raw.tm_wday		= data[6];
						raw.tm_yday		= data[7];
						raw.tm_isdst	= data[8];
					}
					return true;
				case Variant::etBitset8:
					return readBitset(v.as<Variant::Bitset8>(true));
				case Variant::etBitset16:
					return readBitset(v.as<Variant::Bitset16>(true));
				case Variant::etBitset32:
					return readBitset(v.as<Variant::Bitset32>(true));
				case Variant::etBitset64:
					return readBitset(v.as<Variant::Bitset64>(true));
				case Variant::etBitset128:
					return readBitset(v.as<Variant::Bitset128>(true));
				case Variant::etBitset256:
					return readBitset(v.as<Variant::Bitset256>(true));
				case Variant::etBitset512:
					return readBitset(v.as<Variant::Bitset512>(true));
				case Variant::etDateDuration:
					{
						int32_t i4;
						if(!readIntegral(i4))
						{
							return false;
						}
						v = Variant::DateDuration(i4);
					}
					return true;
				case Variant::etTimeDuration:
					{
						int64_t i8;
						if(!readIntegral(i8))
						{
							return false;
						}
						v = posix_time::microseconds(i8);
					}
					return true;
				case Variant::etDateTimeDuration:
					{
						int32_t i4_d;
						int64_t i8_t;
						if(!readIntegral(i4_d) || !readIntegral(i8_t))
						{
							return false;
						}
						Variant::DateTimeDuration &raw = v.as<Variant::DateTimeDuration>(true);

						raw._dd = Variant::DateDuration(i4_d);
						raw._td = posix_time::microseconds(i8_t);
					}
					return true;
				case Variant::etMapVariantVariant:
					return readMap(v.as<Variant::MapVariantVariant>(true));
				case Variant::etMultimapStringVariant:
					return readMap(v.as<Variant::MultimapStringVariant>(true));
				case Variant::etMultimapVariantVariant:
					return readMap(v.as<Variant::MultimapVariantVariant>(true));
				case Variant::etSetVariant:
					return readSeq_insertable(v.as<Variant::SetVariant>(true));
				case Variant::etMultisetVariant:
					return readSeq_insertable(v.as<Variant::MultisetVariant>(true));
				case Variant::etDequeVariant:
					return readSeq_indexable(v.as<Variant::DequeVariant>(true));
				case Variant::etListVariant:
					return readSeq_insertable(v.as<Variant::ListVariant>(true));
				case Variant::etChar:
					return readIntegral(v.as<Variant::Char>(true));
				case Variant::etUuid:
					{
						Variant::Uuid &raw = v.as<Variant::Uuid>(true);
						if(!readBinary((char *)raw.data, 16))
						{
							return false;
						}
					}
					return true;
				case Variant::etVariantPtr:
					{
						uint64_t key;
						if(!readIntegral(key))
						{
							return false;
						}

						if(key)
						{
							TMKeyPtr::iterator iter = _mKeyPtr.find(key);
							if(_mKeyPtr.end() == iter)
							{
								Variant::VariantPtr &p = v.as<Variant::VariantPtr>(true);
								p.reset(new Variant);
								_mKeyPtr.insert(std::make_pair(key, p));

								if(!read(*p))
								{
									return false;
								}
							}
							else
							{
								v.as<Variant::VariantPtr>(true) = iter->second;
							}
						}
						else
						{
							v.as<Variant::VariantPtr>(true).reset();
						}
					}
					return true;
				default:
					assert(!"bad et");
					return false;
				}

				assert(!"never here");
				return false;
			}
		};
	}

	//////////////////////////////////////////////////////////////////////////
	shared_array<char> Variant::serialize(size_t &size) const
	{
		Serializer<0x472e> s;
		return s.serialize(*this, size);
	}

	//////////////////////////////////////////////////////////////////////////
	bool Variant::deserialize(shared_array<char> data, size_t size)
	{
		if(data)
		{
			Variant tmp;
			Serializer<0x472e> s;
			if(!s.deserialize(tmp, data, size))
			{
				return false;
			}
			swap(tmp);
			return true;
		}

		return false;
	}

}
