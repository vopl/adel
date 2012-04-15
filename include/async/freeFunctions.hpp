#ifndef _ASYNC_FREEFUNCTIONS_HPP_
#define _ASYNC_FREEFUNCTIONS_HPP_

#include "async/future.hpp"
#include "async/service.hpp"

#include <boost/asio/io_service.hpp>

namespace async
{
	void spawn(const boost::function<void ()> &code);
	Future<boost::system::error_code> timeout(size_t millisec);
	void exec(const boost::function<void ()> &code);
	void yield();
	boost::asio::io_service &io();
	bool serviceExists();
	Service service();
}

#include "async/asioBridge.hpp"

namespace async
{
    template <class Handler>
	AsioBridge<Handler> bridge(const Handler &handler)
    {
		return AsioBridge<Handler>(handler);
    }

}

#endif
