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

		bool isOpen() const;

		typedef boost::function<void(boost::system::error_code ec, Packet p)> TOnReceive;
		boost::signals2::connection onReceive(const TOnReceive &f);
		void listen(size_t amount=(size_t)-1);

		async::Future2<boost::system::error_code, Packet> receive(size_t maxSize=1024);
		async::Future<boost::system::error_code> send(const Packet &p);
		async::Future<boost::system::error_code> send(
			const std::vector<std::pair<const char *, size_t> > &buffers,
			const std::vector<Packet> &packets4keep);

		void close();

		//millisec
		size_t getTimeout();
		void setTimeout(size_t ms);

		boost::asio::ip::tcp::endpoint endpointRemote();
		boost::asio::ip::tcp::endpoint endpointLocal();
	};
}
#endif
