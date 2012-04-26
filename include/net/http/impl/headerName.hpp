#ifndef _NET_HTTP_HN_IMPL_HPP_
#define _NET_HTTP_HN_IMPL_HPP_

#include <boost/mpl/string.hpp>
#include <boost/mpl/fold.hpp>
#include <boost/mpl/size_t.hpp>
#include <boost/functional/hash.hpp>
#include <string>

namespace net { namespace http { namespace impl { namespace hn
{
	//http://arcticinteractive.com/2009/04/18/compile-time-string-hashing-boost-mpl/

	using namespace boost;
#pragma warning(push)
// disable addition overflow warning
#pragma warning(disable:4307)

	template <typename Seed, typename Value>
	struct hash_combine
	{
	  typedef mpl::size_t<
		Seed::value ^ (static_cast<std::size_t>(Value::value)
		  + 0x9e3779b9 + (Seed::value << 6) + (Seed::value >> 2))
	  > type;
	};

#pragma warning(pop)

	// Hash any sequence of integral wrapper types
	template <typename Sequence>
	struct hash_sequence
	  : mpl::fold<
			Sequence
		  , mpl::size_t<0>
		  , hash_combine<mpl::_1, mpl::_2>
		>::type
	{};

	// For hashing std::strings et al that don't include the zero-terminator
	template <typename String>
	struct hash_string
	  : hash_sequence<String>
	{};

	// Hash including terminating zero for char arrays
	template <typename String>
	struct hash_cstring
	  : hash_combine<
			hash_sequence<String>
		  , mpl::size_t<0>
		>::type
	{};



	/////////////////////////////////////////////////////////////////////////////
	template <class id>
	struct stringHolder
	{
		static const std::string str;
		static const std::string strlc;
	};
	template <class id> const std::string stringHolder<id>::str = id::csz();
	template <class id> const std::string stringHolder<id>::strlc = id::cszlc();

}}}}

#define NET_HTTP_HN_INSTANCE(id_, value_, ...)												\
namespace net { namespace http { namespace hn {												\
	struct id_																				\
	{																						\
		typedef boost::mpl::string< __VA_ARGS__ > cts;										\
		static const std::size_t hash = net::http::impl::hn::hash_cstring< cts >::value;	\
		static const char *csz()															\
		{																					\
			return value_;																	\
		}																					\
		static const char *cszlc()															\
		{																					\
			return boost::mpl::c_str< cts >::value;											\
		}																					\
		static const std::string &str()														\
		{																					\
			return net::http::impl::hn::stringHolder<id_>::str;								\
		}																					\
		static const std::string &strlc()													\
		{																					\
			return net::http::impl::hn::stringHolder<id_>::strlc;							\
		}																					\
	};																						\
}}}


#endif
