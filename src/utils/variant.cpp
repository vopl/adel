#include "pch.hpp"
#include "utils/variant.hpp"
#include <new>

#include "variantImpl.hpp"

#define IMPL (static_cast<VariantImpl *>(this))
#define CIMPL (static_cast<const VariantImpl *>(this))

namespace utils
{
	//////////////////////////////////////////////////////////////////////////
	Variant::~Variant()
	{
		IMPL->destruct();
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::Variant()
	{
		IMPL->construct();
	}














	//////////////////////////////////////////////////////////////////////////
#define ENUM_VARIANT_TYPE(n)		\
	Variant::Variant(const n &v)	\
	{								\
			IMPL->validateType<n>();\
			IMPL->construct(v);		\
	}								//\

ENUM_VARIANT_TYPES
#undef ENUM_VARIANT_TYPE
	Variant::Variant(const Variant &v)
	{
		IMPL->construct(v);
	}
	Variant::Variant(const char *v)
	{
		IMPL->construct(v);
	}

//////////////////////////////////////////////////////////////////////////
#define ENUM_VARIANT_TYPE(n)			\
	Variant &Variant::operator=(const n &v)	\
	{										\
		IMPL->validateType<n>();			\
		IMPL->assign(v);					\
		return *this;						\
	}										//\

ENUM_VARIANT_TYPES
#undef ENUM_VARIANT_TYPE
	Variant &Variant::operator=(const Variant &v)
	{
		IMPL->assign(v);
		return *this;
	}

Variant &Variant::operator=(const char * v)
{
	IMPL->assign(v);
	return *this;
}

//////////////////////////////////////////////////////////////////////////
#define ENUM_VARIANT_TYPE(n)	\
	Variant::operator n ()const		\
	{								\
		CIMPL->validateType<n>();	\
		if(is<n>())					\
		{							\
			return as<n>();			\
		}							\
		n res;						\
		CIMPL->convert<n>(res);		\
		return res;					\
	}								//\

ENUM_VARIANT_TYPES
#undef ENUM_VARIANT_TYPE



	//////////////////////////////////////////////////////////////////////////
	void Variant::swap(Variant & v)
	{
		if(this != &v)
		{
			static const size_t s = sizeof(*this);
			char tmp[s];
			memcpy(tmp, this, s);
			memcpy(this, &v, s);
			memcpy(&v, tmp, s);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool Variant::isNull() const
	{
		return CIMPL->isNull();
	}

	//////////////////////////////////////////////////////////////////////////
	void Variant::setNull(bool n)
	{
		return IMPL->setNull(n);
	}

	//////////////////////////////////////////////////////////////////////////
	template<typename T> void Variant::setNull(bool n)
	{
		IMPL->validateType<T>();
		IMPL->setType<T>();
		return IMPL->setNull(n);
	}
#define ENUM_VARIANT_TYPE(n) template void Variant::setNull<Variant::n>(bool n);
	ENUM_VARIANT_TYPES
#undef ENUM_VARIANT_TYPE
template void Variant::setNull<Variant::Void>(bool n);

	//////////////////////////////////////////////////////////////////////////
	Variant::EType Variant::type() const
	{
		return CIMPL->type();
	}

	//////////////////////////////////////////////////////////////////////////
	void *Variant::data()
	{
		return IMPL->data();
	}

	//////////////////////////////////////////////////////////////////////////
	void const *Variant::data() const
	{
		return CIMPL->data();
	}

	//////////////////////////////////////////////////////////////////////////
	template<typename T> T &Variant::as(bool forceType)
	{
		IMPL->validateType<T>();

		if(forceType)
		{
			IMPL->setType<T>();
			IMPL->setNull(false);
		}
		else
		{
			IMPL->validateValue<T>();
		}

		return IMPL->as<T>();
	}
#define ENUM_VARIANT_TYPE(n) template Variant::n &Variant::as<Variant::n>(bool setType);
ENUM_VARIANT_TYPES
#undef ENUM_VARIANT_TYPE


	//////////////////////////////////////////////////////////////////////////
	template<typename T> T const &Variant::as() const
	{
		CIMPL->validateType<T>();
		CIMPL->validateValue<T>();
		return CIMPL->as<T>();
	}
#define ENUM_VARIANT_TYPE(n) template Variant::n const &Variant::as<Variant::n>() const;
ENUM_VARIANT_TYPES
#undef ENUM_VARIANT_TYPE


	//////////////////////////////////////////////////////////////////////////
	template<typename T> T &Variant::as()
	{
		CIMPL->validateType<T>();
		CIMPL->validateValue<T>();
		return IMPL->as<T>();
	}
#define ENUM_VARIANT_TYPE(n) template Variant::n &Variant::as<Variant::n>();
ENUM_VARIANT_TYPES
#undef ENUM_VARIANT_TYPE

	//////////////////////////////////////////////////////////////////////////
	template<typename T> bool Variant::is() const
	{
		CIMPL->validateType<T>();
		return CIMPL->is<T>();
	}
#define ENUM_VARIANT_TYPE(n) template bool Variant::is<Variant::n>() const;
ENUM_VARIANT_TYPES
#undef ENUM_VARIANT_TYPE
template bool Variant::is<Variant::Void>() const;

	//////////////////////////////////////////////////////////////////////////
	template<typename T> void Variant::setType()
	{
		IMPL->validateType<T>();
		return IMPL->setType<T>();
	}
#define ENUM_VARIANT_TYPE(n) template void Variant::setType<Variant::n>();
ENUM_VARIANT_TYPES
#undef ENUM_VARIANT_TYPE
template void Variant::setType<Variant::Void>();

	//////////////////////////////////////////////////////////////////////////
	void Variant::setType(EType et)
	{
		return IMPL->setType(et);
	}

	//////////////////////////////////////////////////////////////////////////
	bool Variant::isMap() const
	{
		switch(type())
		{
		case etMapStringVariant:
		case etMapVariantVariant:
		case etMultimapStringVariant:
		case etMultimapVariantVariant:
			return true;
		default:
			break;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool Variant::isArray() const
	{
		switch(type())
		{
		case etVectorVariant:
		case etDequeVariant:
			return true;
		default:
			break;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool Variant::isScalar() const
	{
		switch(type())
		{
		case etString:
		case etFloat:
		case etDouble:
		case etInt8:
		case etInt16:
		case etInt32:
		case etInt64:
		case etUInt8:
		case etUInt16:
		case etUInt32:
		case etUInt64:
		case etVectorChar:
		case etDate:
		case etDatetime:
		case etBool:
		case etTm:
		case etBitset8:
		case etBitset16:
		case etBitset32:
		case etBitset64:
		case etBitset128:
		case etBitset256:
		case etBitset512:
		case etDateDuration:
		case etTimeDuration:
		case etDateTimeDuration:
		case etChar:
		case etUuid:
			return true;
		default:
			break;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	template<typename T> T Variant::to() const
	{
		CIMPL->validateType<T>();
		T res;
		CIMPL->convert<T>(res);
		return res;
	}
#define ENUM_VARIANT_TYPE(n) template Variant::n Variant::to<Variant::n>() const;
ENUM_VARIANT_TYPES
#undef ENUM_VARIANT_TYPE


	//////////////////////////////////////////////////////////////////////////
	template<typename T> void Variant::to(T &dst) const
	{
		CIMPL->validateType<T>();
		if(!CIMPL->convert<T>(dst))
		{
			dst = T();
		}
	}
#define ENUM_VARIANT_TYPE(n) template void Variant::to<Variant::n>(n &dst) const;
ENUM_VARIANT_TYPES
#undef ENUM_VARIANT_TYPE

	//////////////////////////////////////////////////////////////////////////
	template<typename T> void Variant::convert()
	{
		CIMPL->validateType<T>();
		if(is<T>())
		{
			return;
		}
		T dst;
		CIMPL->convert<T>(dst);
		*this = dst;
	}
	template<> void Variant::convert<Variant::Void>()
	{
		setType<Variant::Void>();
	}

	//////////////////////////////////////////////////////////////////////////
	void Variant::convert(EType et)
	{
		switch(et)
		{
		case etVoid: convert<Void>();
#define ENUM_VARIANT_TYPE(n) case et##n: convert<n>(); break;
ENUM_VARIANT_TYPES
#undef ENUM_VARIANT_TYPE
		default:
			assert(!"bad et");
			throw "bad et";
		}
	}

	//////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
	template<typename T> bool Variant::canConvert() const
	{
		T stub;
		return CIMPL->convert<T>(stub);
	}
	template<> bool Variant::canConvert<void>() const
	{
		return false;
	}

	bool Variant::canConvert(EType et) const
	{
		switch(et)
		{
		case etVoid: return canConvert<Void>();
#define ENUM_VARIANT_TYPE(n) case et##n: return canConvert<n>();
ENUM_VARIANT_TYPES
#undef ENUM_VARIANT_TYPE
		default:
			assert(!"bad et");
			throw "bad et";
		}
	}
#endif



	//////////////////////////////////////////////////////////////////////////
	Variant Variant::operator[](const std::string &key)
	{
		Variant v;
		CIMPL->containerDereference(v, key);
		return v;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant Variant::operator[](const char *key)
	{
		Variant v;
		CIMPL->containerDereference(v, std::string(key));
		return v;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant Variant::operator[](const Variant &key)
	{
		Variant v;
		CIMPL->containerDereference(v, key);
		return v;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant Variant::operator[](size_t idx)
	{
		Variant v;
		CIMPL->containerDereference(v, idx);
		return v;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant Variant::operator[](int idx)
	{
		Variant v;
		CIMPL->containerDereference(v, (size_t)idx);
		return v;
	}


	//////////////////////////////////////////////////////////////////////////
	bool Variant::operator <(const Variant &v) const
	{
		return CIMPL->less(v);
	}

	//////////////////////////////////////////////////////////////////////////
	bool Variant::operator ==(const Variant &v) const
	{
		return CIMPL->equal(v);
	}

	//////////////////////////////////////////////////////////////////////////
	bool Variant::operator !=(const Variant &v) const
	{
		return !CIMPL->equal(v);
	}
}


#include "variantDumper.hpp"
