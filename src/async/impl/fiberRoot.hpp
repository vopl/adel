#ifndef _ASYNC_IMPL_FIBERROOT_HPP_
#define _ASYNC_IMPL_FIBERROOT_HPP_

#include "async/impl/fiber.hpp"

namespace async { namespace impl
{
	class FiberRoot
		: public Fiber
	{
	public:
		FiberRoot();
		~FiberRoot();

		bool initialize();

		virtual bool enter(){return true;}
		virtual void leave(){}
	};
	typedef boost::shared_ptr<FiberRoot> FiberRootPtr;
}}

#endif
