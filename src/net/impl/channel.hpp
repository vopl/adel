#ifndef _NET_IMPL_CHANNEL_HPP_
#define _NET_IMPL_CHANNEL_HPP_
#include "net/channel.hpp"
#include "async/service.hpp"
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/thread.hpp>

namespace net { namespace impl
{
	using namespace async;

	typedef boost::asio::ip::tcp::socket TSocket;
	typedef boost::shared_ptr<TSocket> TSocketPtr;

	typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> TSocketSsl;
	typedef boost::shared_ptr<TSocketSsl> TSocketSslPtr;

	typedef boost::asio::ssl::context TSslContext;
	typedef boost::shared_ptr<TSslContext> TSslContextPtr;

	//////////////////////////////////////////////////////////////////////////
	class Channel;
	typedef boost::shared_ptr<Channel> ChannelPtr;

	class Channel
		: public boost::enable_shared_from_this<Channel>
	{
		struct Sock
		{
			TSocketPtr			_socket;
			TSocketSslPtr		_socketSsl;
			TSslContextPtr		_sslContext;

			typedef boost::asio::io_service::strand TStrand;
			typedef boost::shared_ptr<TStrand> TStrandPtr;
			TStrandPtr			_sslStrand;

			Sock(TSocketPtr socket);
			Sock(TSocketSslPtr socketSsl, TSslContextPtr sslContext);
			~Sock();

			template <class Buffer, class Handler>
			void read(const Buffer &b, const Handler &h);

			template <class Buffer, class Handler>
			void write(const Buffer &b, const Handler &h);

			void close();

		private:
			static void onSslShutdown(
				const boost::system::error_code &ec,
				TSocketSslPtr socketSsl,
				TSslContextPtr sslContextHolder);
		} _sock;


	private:

		typedef std::deque<std::pair<Future<boost::system::error_code>, Packet> > TSends;
		typedef boost::function<void(boost::system::error_code ec, Packet p)> TOnReceive;
		typedef size_t TReceive;
		typedef std::deque<TReceive> TReceives;


		void receive_f(async::Future2<boost::system::error_code, Packet> res, size_t maxSize);
		void receiveLoop_f();
		boost::mutex	_mtxReceive;
		TReceives		_receives;

		boost::mutex	_mtxSends;
		TSends			_sends;
		bool			_sendInProcess;

		boost::signals2::signal<void(boost::system::error_code ec, Packet p)> _onReceive;
	private:
		void send_f();

		void onReceive(boost::system::error_code ec, Packet p);

	public:
		Channel(TSocketPtr socket);
		Channel(TSocketSslPtr socket, TSslContextPtr sslContext);
		~Channel();

		boost::signals2::connection connectOnReceive(const TOnReceive &f);

		void listen(size_t amount);

		virtual async::Future2<boost::system::error_code, Packet> receive(size_t maxSize);
		virtual Future<boost::system::error_code> send(const Packet &p);

		void close();
	};
}}

#endif
