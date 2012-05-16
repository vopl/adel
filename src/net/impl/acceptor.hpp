#ifndef _NET_IMPL_ACCEPTOR_HPP_
#define _NET_IMPL_ACCEPTOR_HPP_

#include "net/acceptor.hpp"
#include "async/service.hpp"
#include "net/impl/channel.hpp"

namespace net { namespace impl
{
	using namespace async;

	//////////////////////////////////////////////////////////////////////////
	class Acceptor;
	typedef boost::shared_ptr<Acceptor> AcceptorPtr;

	class Acceptor
		: public boost::enable_shared_from_this<Acceptor>
	{
		boost::mutex	_mtx;
		bool			_inProcess;

		typedef boost::asio::ip::tcp::acceptor TAcceptor;
		typedef boost::shared_ptr<boost::asio::ip::tcp::acceptor> TAcceptorPtr;

		TAcceptorPtr	_acceptor;
		TSslContextPtr	_sslContext;

		typedef net::Acceptor::TOnAccept TOnAccept;
		boost::signals2::signal<void(boost::system::error_code, net::Channel)> _onAccept;

	private:
		std::string onSslPassword();

	private:
		void listen_f(
				Future<boost::system::error_code> res,
				const std::string &host,
				const std::string &service,
				bool useSsl);

		void accept_f(bool useSsl);

		void onAccept(boost::system::error_code ec, net::Channel channel);

	public:
		Acceptor();
		~Acceptor();

		boost::signals2::connection onAccept(const TOnAccept &f);
		Future<boost::system::error_code> listen(const char *host, const char *service, bool useSsl);
		void unlisten();

	};
}}

#endif
