#ifndef _UTILS_IMPLACCESS_HPP_
#define _UTILS_IMPLACCESS_HPP_

namespace utils
{
	//////////////////////////////////////////////////////////////////////////
	template <class Base>
	class ImplAccess
		: public Base
	{
	public:
		ImplAccess(const Base &base);
		ImplAccess(const typename Base::ImplPtr &impl);

		typename Base::ImplPtr impl();
		operator typename Base::ImplPtr();
	};


	//////////////////////////////////////////////////////////////////////////
	template <class Base>
	ImplAccess<Base>::ImplAccess(const Base &base)
		: Base(base)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	template <class Base>
	ImplAccess<Base>::ImplAccess(const typename Base::ImplPtr &impl)
	{
		Base::_impl = impl;
	}

	//////////////////////////////////////////////////////////////////////////
	template <class Base>
	typename Base::ImplPtr ImplAccess<Base>::impl()
	{
		return boost::dynamic_pointer_cast<typename Base::ImplPtr::value_type>(Base::_impl);
	}

	//////////////////////////////////////////////////////////////////////////
	template <class Base>
	ImplAccess<Base>::operator typename Base::ImplPtr()
	{
		return boost::dynamic_pointer_cast<typename Base::ImplPtr::value_type>(Base::_impl);
	}
}

#endif
