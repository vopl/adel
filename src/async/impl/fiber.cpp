#include "pch.hpp"
#include "async/impl/fiber.hpp"
#include "async/impl/fiberRoot.hpp"
#include "async/impl/worker.hpp"
#include "async/exception.hpp"
#include "async/log.hpp"


namespace async { namespace impl
{
	//////////////////////////////////////////////////////////////////////////
	Fiber::Fiber(size_t stacksize)
		: _stacksize(stacksize)
#if defined(HAVE_WINFIBER)
		, _context(NULL)
#elif defined(HAVE_UCONTEXT_H)
#else
#   error Unknown context type for fibers
#endif
		, _isLocked(false)
	{
#if defined(HAVE_WINFIBER)
#elif defined(HAVE_UCONTEXT_H)
		memset(&_context, 0, sizeof(ucontext_t));
#else
#   error Unknown context type for fibers
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	Fiber::~Fiber()
	{
		assert(!_isLocked);
		assert(_current != this);
#if defined(HAVE_WINFIBER)
		if(_context)
		{
			DeleteFiber(_context);
			_context = NULL;
		}
#elif defined(HAVE_UCONTEXT_H)
		if(_context.uc_stack.ss_sp)
		{
			free(_context.uc_stack.ss_sp);
		}
#else
#   error Unknown context type for fibers
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	bool Fiber::initialize()
	{
#if defined(HAVE_WINFIBER)
		assert(!_context);
		if(!_context)
		{
			_context = CreateFiberEx(_stacksize, _stacksize, 0, &Fiber::s_fiberProc, this);
			if(!_context)
			{
				FLOG(__FUNCTION__<<", CreateFiberEx failed, "<<GetLastError());
				//throw exception("CreateFiber failed");
				return false;
			}
		}
#elif defined(HAVE_UCONTEXT_H)
		assert(!_context.uc_stack.ss_sp);
		if(getcontext(&_context))
		{
			FLOG(__FUNCTION__<<", getcontext failed");
			//throw exception("getcontext failed");
			return false;
		}
		_context.uc_link = NULL;
		_context.uc_stack.ss_sp = (char *)malloc(_stacksize);
		_context.uc_stack.ss_size = _stacksize;

#if PVOID_SIZE == INT_SIZE
		int ithis = (int)(this);
		makecontext(&_context, (void (*)(void))&Fiber::s_fiberProc, 1, ithis);
#elif PVOID_SIZE == INT_SIZE*2
		boost::uint64_t ithis = (boost::uint64_t)(this);
		int param1 = (unsigned int)(ithis&0xffffffff);
		int param2 = (unsigned int)((ithis>>32)&0xffffffff);
		makecontext(&_context, (void (*)(void))&Fiber::s_fiberProc, 2, param1, param2);
#else
#	error PVOID_SIZE not equal INT_SIZE or INT_SIZE*2
#endif

#else
#	error Unknown context type for fibers
#endif

		assert(!_isLocked);

// 		static int cnt(0);
// 		TLOG(__FUNCTION__<<", "<<cnt++);
		return true;
	}



	//////////////////////////////////////////////////////////////////////////
	Fiber *Fiber::current()
	{
		Fiber *fiber = _current;
		return fiber;
	}

	//////////////////////////////////////////////////////////////////////////
	bool Fiber::execute(const boost::function<void()> &code)
	{
		if(!enter())
		{
			return false;
		}

		assert(_current != this);

		assert(!_code);
		assert(code);
		_code = code;


		return activate(true);
	}

	//////////////////////////////////////////////////////////////////////////
	bool Fiber::activate(bool alreadyLocked)
	{
		if(!alreadyLocked)
		{
			if(!enter())
			{
				return false;
			}
		}

		Fiber *prev = _current.get();
		assert(prev);
		assert(prev != this);

		_current = this;
#if defined(HAVE_WINFIBER)
		SwitchToFiber(_context);
#elif defined(HAVE_UCONTEXT_H)
		if(swapcontext(&prev->_context, &_context))
		{
			FLOG(__FUNCTION__<<", swapcontext failed");
			throw exception("swapcontext failed");
		}
#else
#   error Unknown context type for fibers
#endif
		leave();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool Fiber::enter()
	{
		boost::mutex::scoped_lock sl(_mtx);
		if(_isLocked)
		{
			return false;
		}
		_isLocked = true;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void Fiber::leave()
	{
		boost::mutex::scoped_lock sl(_mtx);
		assert(_isLocked);
		_isLocked = false;
	}


	//////////////////////////////////////////////////////////////////////////
#if defined(HAVE_WINFIBER)
	VOID WINAPI Fiber::s_fiberProc(LPVOID param)
	{
		((Fiber*)param)->fiberProc();
	}
#elif defined(HAVE_UCONTEXT_H)
#	if PVOID_SIZE == INT_SIZE
	void Fiber::s_fiberProc(int param)
	{
		((Fiber*)param)->fiberProc();
	}
#	elif PVOID_SIZE == INT_SIZE*2
	void Fiber::s_fiberProc(int param1, int param2)
	{
		boost::uint64_t ithis = ((unsigned int)param1) | (((boost::uint64_t)param2)<<32);

		((Fiber*)ithis)->fiberProc();
	}
#	else
#		error PVOID_SIZE not equal INT_SIZE or INT_SIZE*2
#	endif
#else
#	error Unknown context type for fibers
#endif

	//////////////////////////////////////////////////////////////////////////
	void Fiber::fiberProc()
	{
		for(;;)
		{
			assert(_code);
			assert(Fiber::current() == this);

			try
			{
				_code();
			}
			catch(...)
			{
				ELOG(__FUNCTION__<<", exception catched: "<<boost::current_exception_diagnostic_information());
			}
			assert(_code);
			assert(Fiber::current() == this);

			boost::function<void()>().swap(_code);

			Worker::current()->fiberExecuted(this);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	ThreadLocalStorage<Fiber *> Fiber::_current;

}}
