#include "pch.hpp"
#include "net/pch.hpp"
#include "net/impl/channel.hpp"
#include "utils/fixEndian.hpp"
#include <boost/bind.hpp>

namespace net { namespace impl
{
	using namespace boost;
	using namespace boost::asio;
	using namespace boost::system;
	using namespace async;

	//////////////////////////////////////////////////////////////////////////
	Channel::Sock::Sock(TSocketPtr socket)
		: _socket(socket)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	Channel::Sock::Sock(TSocketSslPtr socketSsl, TSslContextPtr sslContext)
		: _socketSsl(socketSsl)
		, _sslContext(sslContext)
		, _sslStrand(new TStrand(socketSsl->get_io_service()))
	{
	}

	//////////////////////////////////////////////////////////////////////////
	Channel::Sock::~Sock()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool Channel::Sock::isOpen() const
	{
		if(_socket)
		{
			return _socket->is_open();
		}
		return _socketSsl->lowest_layer().is_open();
	}


	//////////////////////////////////////////////////////////////////////////
	template <class Buffer, class Handler>
	void Channel::Sock::read(const Buffer &b, const Handler &h)
	{
		if(_socket)
		{
			_socket->async_read_some(b, async::bridge(h));
		}
		else
		{
			typedef asio::detail::wrapped_handler<
				asio::io_service::strand,
				async::AsioBridge<Handler> > WrappedHandler;
			_sslStrand->dispatch(
				bind(&TSocketSsl::async_read_some<Buffer, WrappedHandler>, _socketSsl.get(), b, _sslStrand->wrap(async::bridge(h))));
		}
	}

	//////////////////////////////////////////////////////////////////////////
	template <class Buffer, class Handler>
	void Channel::Sock::write(const Buffer &b, const Handler &h)
	{
		if(_socket)
		{
			_socket->async_write_some(b, async::bridge(h));
		}
		else
		{
			typedef asio::detail::wrapped_handler<
				asio::io_service::strand,
				async::AsioBridge<Handler> > WrappedHandler;
			_sslStrand->dispatch(
				bind(&TSocketSsl::async_write_some<Buffer, WrappedHandler>, _socketSsl.get(), b, _sslStrand->wrap(async::bridge(h))));
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void Channel::Sock::close()
	{
		error_code ec;
		if(_socket)
		{
			_socket->lowest_layer().shutdown(asio::socket_base::shutdown_both, ec);
			_socket->lowest_layer().close(ec);
		}
		else
		{
			_socketSsl->lowest_layer().shutdown(asio::socket_base::shutdown_both, ec);
			_socketSsl->lowest_layer().close(ec);

// 			typedef function<void(const error_code &)> TOnShutdown;
// 			TOnShutdown onShutdown = boost::bind(&Sock::onSslShutdown, _1, _socketSsl, _sslContext);
//
// 			_sslStrand->dispatch(
// 				bind(&TSocketSsl::async_shutdown<TOnShutdown>, _socketSsl, onShutdown)
// 				);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void Channel::Sock::onSslShutdown(
		const error_code &ec,
		TSocketSslPtr socketSsl,
		TSslContextPtr sslContextHolder)
	{
		error_code ecl;
		socketSsl->lowest_layer().shutdown(asio::socket_base::shutdown_both, ecl);
		socketSsl->lowest_layer().close(ecl);
	}


	//////////////////////////////////////////////////////////////////////////
	void Channel::receive_f(async::Future2<boost::system::error_code, Packet> res, size_t maxSize)
	{
		Packet				packet;

		packet._size = maxSize;
		packet._data.reset(new char[packet._size]);

		Future2<error_code, size_t> readRes;
		_sock.read(
			buffer(packet._data.get(), packet._size),
			readRes);

		readRes.wait();
		packet._size = readRes.data2NoWait();

		res(readRes.data1NoWait(), packet);
	}

	//////////////////////////////////////////////////////////////////////////
	void Channel::receiveLoop_f()
	{
		TReceive receive;
		receive = 0;

		for(;;)
		{
			while(!receive)
			{
				mutex::scoped_lock sl(_mtxReceive);
				if(_receives.empty())
				{
					return;
				}

				std::swap(receive, _receives[0]);
				_receives.erase(_receives.begin());
			}
			receive--;

			error_code			ec;

			//данные
			Packet				packet;
			packet._size = 1024*64;
			if(packet._size)
			{
				packet._data.reset(new char[packet._size]);
				Future2<error_code, size_t> readRes;
				_sock.read(
					buffer(packet._data.get(), packet._size),
					readRes);
				ec = readRes.data1();

				if(ec)
				{
					if(asio::error::operation_aborted == ec.value())
					{
						return;
					}
					spawn(bind(&Channel::onReceive, shared_from_this(), ec, Packet()));
					return;
				}
			}

			spawn(bind(&Channel::onReceive, shared_from_this(), error_code(), packet));
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void Channel::onReceive(boost::system::error_code ec, Packet p)
	{
		_onReceive(ec, p);
	}

	//////////////////////////////////////////////////////////////////////////
	void Channel::send_f()
	{
		bool localSendInProcess = false;
		for(;;)
		{
			std::pair<Future<error_code>, Packet> op;
			{
				mutex::scoped_lock sl(_mtxSends);
				if(!localSendInProcess && _sendInProcess)
				{
					return;
				}
				if(_sends.empty())
				{
					_sendInProcess = false;
					return;
				}
				op = _sends[0];
				_sends.erase(_sends.begin());
				_sendInProcess = true;
				localSendInProcess = true;
			}

			size_t				transferedSize = 0;
			error_code			ec;

			//данные
			Packet				&packet = op.second;
			if(packet._size)
			{
				while(transferedSize < packet._size)
				{
					Future2<error_code, size_t> writeRes;
					_sock.write(
						buffer(packet._data.get() + transferedSize, packet._size - transferedSize),
						writeRes);
					ec = writeRes.data1();

					if(ec)
					{
						op.first(ec);
						mutex::scoped_lock sl(_mtxSends);
						_sendInProcess = false;
						return;
					}
					transferedSize += writeRes.data2();
				}
			}
			assert(transferedSize == packet._size);
			transferedSize = 0;

			op.first(error_code());
			//mutex::scoped_lock sl(_mtxSends);
			//_sendInProcess = false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	Channel::Channel(TSocketPtr socket)
		: _sock(socket)
		, _sendInProcess(false)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	Channel::Channel(TSocketSslPtr socket, TSslContextPtr sslContext)
		: _sock(socket, sslContext)
		, _sendInProcess(false)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	Channel::~Channel()
	{
		close();
	}

	//////////////////////////////////////////////////////////////////////////
	bool Channel::isOpen() const
	{
		return _sock.isOpen();
	}

	//////////////////////////////////////////////////////////////////////////
	boost::signals2::connection Channel::connectOnReceive(const TOnReceive &f)
	{
		return _onReceive.connect(f);
	}

	//////////////////////////////////////////////////////////////////////////
	void Channel::listen(size_t amount)
	{
		mutex::scoped_lock sl(_mtxReceive);
		_receives.push_back(amount);
		if(_receives.size() < 2)
		{
			spawn(bind(&Channel::receiveLoop_f, shared_from_this()));
		}
	}

	//////////////////////////////////////////////////////////////////////////
	async::Future2<boost::system::error_code, Packet> Channel::receive(size_t maxSize)
	{
		async::Future2<boost::system::error_code, Packet> res;
		spawn(bind(&Channel::receive_f, shared_from_this(), res, maxSize));
		return res;
	}

	//////////////////////////////////////////////////////////////////////////
	Future<error_code> Channel::send(const Packet &p)
	{
		Future<error_code> res;

		mutex::scoped_lock sl(_mtxSends);
		_sends.push_back(std::make_pair(res, p));
		if(_sends.size() < 2)
		{
			spawn(bind(&Channel::send_f, shared_from_this()));
		}

		return res;
	}

	//////////////////////////////////////////////////////////////////////////
	void Channel::close()
	{
		_sock.close();
	}
}}
