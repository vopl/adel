#ifndef _NET_CHANNEL_HPP_
#define _NET_CHANNEL_HPP_

#include <boost/cstdint.hpp>
#include <boost/function.hpp>
#include <boost/system/error_code.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>
#include "async/future.hpp"
#include "net/packet.hpp"

namespace net
{
	//////////////////////////////////////////////////////////////////////////
	namespace impl
	{
		class Channel;
		typedef boost::shared_ptr<Channel> ChannelPtr;

	}

	//////////////////////////////////////////////////////////////////////////
	class Channel
	{
	protected:
		typedef impl::ChannelPtr ImplPtr;
		ImplPtr	_impl;

	public:
		Channel();
		~Channel();

		typedef boost::function<void(const boost::system::error_code &ec, const Packet &p)> TOnReceive;
		boost::signals2::connection connectOnReceive(const TOnReceive &f);
		void listen(size_t amount=(size_t)-1);
		async::Future<boost::system::error_code> send(const Packet &p);

		void close();
	};
}
#endif
