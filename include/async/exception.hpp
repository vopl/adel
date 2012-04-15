#ifndef _ASYNC_EXCEPTION_HPP_
#define _ASYNC_EXCEPTION_HPP_

#include <stdexcept>

//////////////////////////////////////////////////////////////////////////
namespace async
{
	class exception
		: public std::runtime_error
	{
	public:
		exception(const std::string &arg)
			: std::runtime_error(arg)
		{
		}
	};
}

#endif
