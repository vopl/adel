#include "variantConvertMatrix.hpp"
#include <boost/exception/all.hpp>

//warning C4127: conditional expression is constant
#pragma warning (push)
#pragma warning (disable: 4127)

namespace utils
{
	namespace
	{
		//////////////////////////////////////////////////////////////////////////
		inline bool operator <(const std::tm &v1,const std::tm &v2)
		{
			std::tm cv1(v1),cv2(v2);
			return mktime(&cv1) < mktime(&cv2);
		}

		//////////////////////////////////////////////////////////////////////////
		template <size_t N>
		bool operator <(const std::bitset<N> &v1,const std::bitset<N> &v2)
		{
			if(N <= sizeof(unsigned long))
			{
				return v1.to_ulong() < v2.to_ulong();
			}
			else
			{
				for(size_t i(N-1); i<N; i--)
				{
					if(!v1[i] && v2[i]) return true;
					if(v1[i] && !v2[i]) return false;
				}
				return false;
			}
		}

		//////////////////////////////////////////////////////////////////////////
		inline bool operator ==(const std::tm &v1,const std::tm &v2)
		{
			return 0 == memcmp(&v1, &v2, sizeof(std::tm));
		}

	}


	//////////////////////////////////////////////////////////////////////////
	class VariantImpl
		: public Variant
	{
		//////////////////////////////////////////////////////////////////////////
		//выделение блока памяти под данные
		template <class T> void alloc()
		{
			if(sizeof(T) <= sizeof(_data))
			{
				//для autoexp.dat
#ifdef UTILS_VARIANT_DBGDATA
				_dbgData = &as<T>();
#endif
				return;
			}
			as<T*>() = (T*)new char[sizeof(T)];
			//для autoexp.dat
#ifdef UTILS_VARIANT_DBGDATA
			_dbgData = &as<T>();
#endif
		}
		
		//////////////////////////////////////////////////////////////////////////
		template <class T> void free()
		{
			//для autoexp.dat
#ifdef UTILS_VARIANT_DBGDATA
			_dbgData = NULL;
#endif
			if(sizeof(T) <= sizeof(_data))
			{
				return;
			}
			delete [] (char *)as<T*>();
		}

	public:
		//////////////////////////////////////////////////////////////////////////
		template <class T>
		void validateType() const
		{
			if(etUnknown == Type2Enum<T>::et)
			{
				throw std::bad_cast();
			}
		}

		//////////////////////////////////////////////////////////////////////////
		template <class T>
		void validateValue() const
		{
			if(type() != Type2Enum<T>::et || isNull())
			{
				throw std::bad_cast();
			}
		}

		//////////////////////////////////////////////////////////////////////////
		void construct()
		{
			_et = -(ETypeStorage)etVoid;
		}

		//////////////////////////////////////////////////////////////////////////
		template <class T>
		void construct();

		//////////////////////////////////////////////////////////////////////////
		template <class T>
		void construct(const T &v);

		//////////////////////////////////////////////////////////////////////////
		void construct(const char *v)
		{
			if(v)
			{
				_et = Type2Enum<String>::et;
				alloc<String>();
				new (&as<String>()) String(v);
			}
			else
			{
				_et = -(ETypeStorage)Type2Enum<String>::et;
			}
		}

		//////////////////////////////////////////////////////////////////////////
		void destruct()
		{
			switch(type())
			{
			case etVoid: break;
#define ENUM_VARIANT_TYPE(n) case et ## n: destruct< n >(); break;
				ENUM_VARIANT_TYPES
#undef ENUM_VARIANT_TYPE
			default:
				assert(!"bad et");
				throw "bad et";
			}
		}

		//////////////////////////////////////////////////////////////////////////
		template <class T>
		void destruct()
		{
			assert(type() == (ETypeStorage)Type2Enum<T>::et);

			if(isNull())
			{
				return;
			}

			(&as<T>())->~T();
			free<T>();
			_et = -(ETypeStorage)Type2Enum<T>::et;
		}

		//////////////////////////////////////////////////////////////////////////
		template <class T>
		void assign(const T &v);
		
		
		
		
		


		//////////////////////////////////////////////////////////////////////////
		void assign(const char *v)
		{
			if(is<String>())
			{
				setNull(false);
				as<String>() = v;
				return;
			}

			destruct();
			construct(v);
		}

		//////////////////////////////////////////////////////////////////////////
		bool isNull() const
		{
			return _et < 0;
		}

		//////////////////////////////////////////////////////////////////////////
		void setNull(bool n);

		//////////////////////////////////////////////////////////////////////////
		EType type() const
		{
			return (EType)(_et<0?-_et:_et);
		}

		//////////////////////////////////////////////////////////////////////////
		const void *data() const
		{
			if(isNull())
			{
				return NULL;
			}

			switch(type())
			{
			case etVoid: return NULL;
#define ENUM_VARIANT_TYPE(n) case et ## n: return &as<n>();
				ENUM_VARIANT_TYPES
#undef ENUM_VARIANT_TYPE
			default:
				assert(!"bad et");
				throw "bad et";
			}

			//never here
			return NULL;
		}

		//////////////////////////////////////////////////////////////////////////
		void *data()
		{
			if(isNull())
			{
				return NULL;
			}

			switch(type())
			{
			case etVoid: return NULL;
#define ENUM_VARIANT_TYPE(n) case et ## n: return &as<n>();
				ENUM_VARIANT_TYPES
#undef ENUM_VARIANT_TYPE
			default:
				assert(!"bad et");
				throw "bad et";
			}

			//never here
			return NULL;
		}

		//////////////////////////////////////////////////////////////////////////
		template <class T>
		T &as()
		{
			if(sizeof(T) <= sizeof(_data))
			{
				return *(T*)_data;
			}
			return **(T**)(void *)_data;
		}

		//////////////////////////////////////////////////////////////////////////
		template <class T>
		const T &as() const
		{
			if(sizeof(T) <= sizeof(_data))
			{
				return *(T*)_data;
			}
			return **(T**)(void *)_data;
		}

		//////////////////////////////////////////////////////////////////////////
		template<typename T> 
		bool is() const
		{
			return type() == Type2Enum<T>::et;
		}

		//////////////////////////////////////////////////////////////////////////
		template<typename T> 
		void setType()
		{
			if(is<T>())
			{
				return;
			}

			destruct();
			_et = -(ETypeStorage)Type2Enum<T>::et;
		}
		
		//////////////////////////////////////////////////////////////////////////
		void setType(EType et)
		{
			if(et == type())
			{
				return;
			}

			destruct();
			_et = -(ETypeStorage)et;
		}

		//////////////////////////////////////////////////////////////////////////
		bool less(const Variant &v) const
		{
			if(type() < v.type())
			{
				return true;
			}
			if(type() > v.type())
			{
				return false;
			}

			if(isNull())
			{
				if(!v.isNull())
				{
					return true;
				}
				return false;
			}
			else
			{
				if(v.isNull())
				{
					return false;
				}

				switch(type())
				{
				case etVoid: return false;
#define ENUM_VARIANT_TYPE(n) case et ## n: return as<n>() < v.as<n>();
					ENUM_VARIANT_TYPES
#undef ENUM_VARIANT_TYPE
				default:
					assert(!"bad et");
					throw "bad et";
				}

			}


			//never here
			return false;
		}

		//////////////////////////////////////////////////////////////////////////
		bool equal(const Variant &v) const
		{
			if(type() != v.type() || isNull() != v.isNull())
			{
				return false;
			}

			if(isNull())
			{
				if(v.isNull())
				{
					return true;
				}
				return false;
			}
			else
			{
				if(v.isNull())
				{
					return false;
				}
				switch(type())
				{
				case etVoid: return true;
#define ENUM_VARIANT_TYPE(n) case et ## n: return as<n>() == v.as<n>();
					ENUM_VARIANT_TYPES
#undef ENUM_VARIANT_TYPE
				default:
					assert(!"bad et");
					throw "bad et";
				}
			}

			//never here
			return false;
		}

		//////////////////////////////////////////////////////////////////////////
		template <class Dst>
		bool convert(Dst &dst) const
		{
			if(isNull())
			{
				return false;
			}
			switch(type())
			{
			case etVoid: return false;
#define ENUM_VARIANT_TYPE(n) case et ## n: return impl::variantConvertMatrix(dst, as<n>());
				ENUM_VARIANT_TYPES
#undef ENUM_VARIANT_TYPE
			default:
				assert(!"bad et");
				throw "bad et";
			}
			return true;
		}

		//////////////////////////////////////////////////////////////////////////
		void containerDereference(Variant &dst, const std::string &key) const
		{
			if(isNull())
			{
				dst.setNull();
				return;
			}
			switch(type())
			{
			case etMapStringVariant:
				{
					const MapStringVariant &m = as<MapStringVariant>();
					MapStringVariant::const_iterator iter = m.find(key);
					if(m.end() != iter)
					{
						dst = iter->second;
						return;
					}
				}
				break;
			case etMapVariantVariant:
				{
					const MapVariantVariant &m = as<MapVariantVariant>();
					MapVariantVariant::const_iterator iter = m.find(key);
					if(m.end() != iter)
					{
						dst = iter->second;
						return;
					}
				}
				break;
			case etMultimapStringVariant:
				{
					const MultimapStringVariant &m = as<MultimapStringVariant>();
					MultimapStringVariant::const_iterator iter = m.find(key);
					if(m.end() != iter)
					{
						dst = iter->second;
						return;
					}
				}
				break;
			case etMultimapVariantVariant:
				{
					const MultimapVariantVariant &m = as<MultimapVariantVariant>();
					MultimapVariantVariant::const_iterator iter = m.find(key);
					if(m.end() != iter)
					{
						dst = iter->second;
						return;
					}
				}
				break;
			default:
				assert(0);
				break;
			}
			dst.setNull();
		}

		//////////////////////////////////////////////////////////////////////////
		void containerDereference(Variant &dst, const Variant &key) const
		{
			if(isNull())
			{
				dst.setNull();
				return;
			}
			switch(type())
			{
			case etMapStringVariant:
				{
					const MapStringVariant &m = as<MapStringVariant>();
					MapStringVariant::const_iterator iter = m.find(key);
					if(m.end() != iter)
					{
						dst = iter->second;
						return;
					}
				}
				break;
			case etMapVariantVariant:
				{
					const MapVariantVariant &m = as<MapVariantVariant>();
					MapVariantVariant::const_iterator iter = m.find(key);
					if(m.end() != iter)
					{
						dst = iter->second;
						return;
					}
				}
				break;
			case etMultimapStringVariant:
				{
					const MultimapStringVariant &m = as<MultimapStringVariant>();
					MultimapStringVariant::const_iterator iter = m.find(key);
					if(m.end() != iter)
					{
						dst = iter->second;
						return;
					}
				}
				break;
			case etMultimapVariantVariant:
				{
					const MultimapVariantVariant &m = as<MultimapVariantVariant>();
					MultimapVariantVariant::const_iterator iter = m.find(key);
					if(m.end() != iter)
					{
						dst = iter->second;
						return;
					}
				}
				break;
			case etVectorVariant:
				{
					const VectorVariant &m = as<VectorVariant>();
					size_t idx = key;
					if(idx < m.size())
					{
						dst = m[idx];
						return;
					}
				}
				break;
			case etDequeVariant:
				{
					const DequeVariant &m = as<DequeVariant>();
					size_t idx = key;
					if(idx < m.size())
					{
						dst = m[idx];
						return;
					}
				}
				break;
			default:
				assert(0);
			}
			dst.setNull();
		}


		//////////////////////////////////////////////////////////////////////////
		void containerDereference(Variant &dst, size_t idx) const
		{
			if(isNull())
			{
				dst.setNull();
				return;
			}
			switch(type())
			{
			case etVectorVariant:
				{
					const VectorVariant &m = as<VectorVariant>();
					if(idx < m.size())
					{
						dst = m[idx];
						return;
					}
				}
				break;
			case etDequeVariant:
				{
					const DequeVariant &m = as<DequeVariant>();
					if(idx < m.size())
					{
						dst = m[idx];
						return;
					}
				}
				break;
			default:
				assert(0);
				break;
			}
			dst.setNull();
		}
	};
	
	
	
	
	
	
	
	
	
	
		
		
		
		
		
		
		
		
		








	//////////////////////////////////////////////////////////////////////////
	template <>
	void VariantImpl::construct(const Variant &v)
	{
		if(v.isNull())
		{
			_et = -(ETypeStorage)v.type();
			return;
		}
		else
		{
			_et = (ETypeStorage)v.type();
		}

		switch(type())
		{
		case etVoid: break;

#define ENUM_VARIANT_TYPE(n) case et ## n: alloc<n>(); new (&as<n>()) n(v.as<n>()); break;
			ENUM_VARIANT_TYPES
#undef ENUM_VARIANT_TYPE
		default:
			assert(!"bad et");
			throw "bad et";
		}
	}

		
	//////////////////////////////////////////////////////////////////////////
	template <>
	void VariantImpl::construct<Variant::Tm>()
	{
		_et = Type2Enum<Tm>::et;
		alloc<Tm>();
		new (&as<Tm>()) Tm;
	}

	//////////////////////////////////////////////////////////////////////////
	template <>
	void VariantImpl::construct<Variant::Uuid>()
	{
		_et = Type2Enum<Uuid>::et;
		alloc<Uuid>();
		new (&as<Uuid>()) Uuid;
	}

	//////////////////////////////////////////////////////////////////////////
	template <>
	void VariantImpl::assign(const Variant &v)
	{
		if(v.type() == type())
		{
			if(v.isNull())
			{
				setNull(true);
			}
			else
			{
				switch(type())
				{
				case etVoid: destruct(); break;
#define ENUM_VARIANT_TYPE(n) case et ## n: assign<n>(v.as<n>()); break;
					ENUM_VARIANT_TYPES
#undef ENUM_VARIANT_TYPE
				default:
					assert(!"bad et");
					throw "bad et";
				}
			}
			return;
		}

		Variant stub(v);
		swap(stub);
	}		
	//////////////////////////////////////////////////////////////////////////
	template<> 
	void VariantImpl::setType<Variant::Void>()
	{
		if(is<Void>())
		{
			return;
		}

		destruct();
		_et = -(ETypeStorage)Type2Enum<Void>::et;
	}
	
	
	
	
	
	
	
	//////////////////////////////////////////////////////////////////////////
	template <class T>
	void VariantImpl::construct()
	{
		_et = Type2Enum<T>::et;
		alloc<T>();
		new (&as<T>()) T();
	}

	//////////////////////////////////////////////////////////////////////////
	template <class T>
	void VariantImpl::construct(const T &v)
	{
		_et = Type2Enum<T>::et;
		alloc<T>();
		new (&as<T>()) T(v);
	}
	
	//////////////////////////////////////////////////////////////////////////
	template <class T>
	void VariantImpl::assign(const T &v)
	{
		if(is<T>())
		{
			setNull(false);
			as<T>() = v;
			return;
		}

		destruct();
		construct(v);
	}

	//////////////////////////////////////////////////////////////////////////
	void VariantImpl::setNull(bool n)
	{
		if(n)
		{
			if(!isNull())
			{
				destruct();
			}
		}
		else
		{
			if(isNull())
			{
				switch(type())
				{
				case etVoid: break;

#define ENUM_VARIANT_TYPE(n) case et ## n: construct<n>(); break;
					ENUM_VARIANT_TYPES
#undef ENUM_VARIANT_TYPE
				default:
					assert(!"bad et");
					throw "bad et";
				}
			}
		}
	}
	
}

#pragma warning (pop)
