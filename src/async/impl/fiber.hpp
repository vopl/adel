#ifndef _ASYNC_IMPL_FIBER_HPP_
#define _ASYNC_IMPL_FIBER_HPP_

#include "async/impl/threadLocalStorage.hpp"

#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>

#include "config.h"
//#define HAVE_UCONTEXT_H 1
//#define HAVE_WINFIBER 1

#if defined(HAVE_WINFIBER)
#	include <windows.h>
#elif defined(HAVE_UCONTEXT_H)
#	include <ucontext.h>
#else
#   error Unknown context type for fibers
#endif


#if defined(HAVE_VALGRIND)
#	define USE_VALGRIND 1
#endif


//////////////////////////////////////////////////////////////////////////
namespace async { namespace impl
{
	class Worker;
	class Fiber;
	typedef boost::shared_ptr<Fiber> FiberPtr;

	class Fiber
		: public boost::enable_shared_from_this<Fiber>
	{
	public:
		Fiber(size_t stacksize = 1024*64);
		virtual ~Fiber();

		bool initialize();
		static Fiber *current();

		bool execute(const boost::function<void()> &code);
		bool activate(bool alreadyLocked = false);

	private:
#if defined(HAVE_WINFIBER)
	static VOID WINAPI s_fiberProc(LPVOID param);
#elif defined(HAVE_UCONTEXT_H)
#	if PVOID_SIZE == INT_SIZE
        static void s_fiberProc(int param);
#	elif PVOID_SIZE == INT_SIZE*2
        static void s_fiberProc(int param1, int param2);
#	else
#		error PVOID_SIZE not equal INT_SIZE or INT_SIZE*2
#	endif
#else
#	error Unknown context type for fibers
#endif
		void fiberProc();

	protected:
		virtual bool enter();
		virtual void leave();

	protected:

		size_t _stacksize;
#if defined(HAVE_WINFIBER)
		LPVOID	_context;
#elif defined(HAVE_UCONTEXT_H)
		ucontext_t	_context;
#else
#   error Unknown context type for fibers
#endif

		boost::function<void()>	_code;
		boost::mutex			_mtx;
		bool					_isLocked;

		static ThreadLocalStorage<Fiber *>
								_current;

#if defined(USE_VALGRIND)
		int _valgrindStackId;
#endif
	};
}}

#endif
