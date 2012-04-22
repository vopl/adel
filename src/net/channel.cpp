#include "pch.hpp"
#include "net/channel.hpp"
#include "net/impl/channel.hpp"

namespace net
{
	//////////////////////////////////////////////////////////////////////////
	Channel::Channel()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	Channel::~Channel()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	boost::signals2::connection Channel::connectOnReceive(const TOnReceive &f)
	{
		return _impl->connectOnReceive(f);
	}

	//////////////////////////////////////////////////////////////////////////
	void Channel::listen(size_t amount)
	{
		return _impl->listen(amount);
	}

	//////////////////////////////////////////////////////////////////////////
	async::Future2<boost::system::error_code, Packet> Channel::receive(size_t maxSize)
	{
		return _impl->receive(maxSize);
	}

	//////////////////////////////////////////////////////////////////////////
	async::Future<boost::system::error_code> Channel::send(const Packet &p)
	{
		return _impl->send(p);
	}

	//////////////////////////////////////////////////////////////////////////
	void Channel::close()
	{
		return _impl->close();
	}
}
