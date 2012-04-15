#ifndef _ASYNC_IMPL_THREADLOCALSTORAGE_HPP_
#define _ASYNC_IMPL_THREADLOCALSTORAGE_HPP_


#include <boost/noncopyable.hpp>
#include <boost/utility/enable_if.hpp>
#include <assert.h>

#ifdef _WIN32
#	include <windows.h>
#else
#	include <pthread.h>
#endif


namespace async { namespace impl
{

	template <class T>
	class ThreadLocalStorageBase
		: boost::noncopyable
	{
	public:
		ThreadLocalStorageBase()
		{
#ifdef _WIN32
			m_key = TlsAlloc();
#else
			int rc = pthread_key_create(&m_key, NULL);
#endif
			assert(m_key);
		}

		~ThreadLocalStorageBase()
		{
#ifdef _WIN32
			TlsFree(m_key);
#else
			pthread_key_delete(m_key);
#endif
		}

		typename boost::enable_if_c<sizeof(T) <= sizeof(void *)>::type set(const T &t)
		{
#ifdef _WIN32
			TlsSetValue(m_key, (LPVOID)t);
#else
			pthread_setspecific(m_key, (const void *)t);
#endif
		}

		T get() const
		{
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable: 4800)
			DWORD lastError = GetLastError();
			LPVOID result = TlsGetValue(m_key);
			SetLastError(lastError);
			return (T)result;
#pragma warning(pop)
#else
			return (T)pthread_getspecific(m_key);
#endif
		}

		operator T() const { return get(); }

	private:
#ifdef _WIN32
		DWORD m_key;
#else
		pthread_key_t m_key;
#endif
	};

	template <class T>
	class ThreadLocalStorage : public ThreadLocalStorageBase<T>
	{
	public:
		T operator =(T t) { ThreadLocalStorageBase<T>::set(t); return t; }
	};

	template <class T>
	class ThreadLocalStorage<T *> : public ThreadLocalStorageBase<T *>
	{
	public:
		T * operator =(T *const t) { ThreadLocalStorageBase<T *>::set(t); return t; }
		T & operator*() { return *ThreadLocalStorageBase<T *>::get(); }
		T * operator->() { return ThreadLocalStorageBase<T *>::get(); }
	};

}}

#endif
