#include "pch.hpp"
#include "net/pch.hpp"
#include "net/impl/connector.hpp"
#include "net/log.hpp"
#include "utils/implAccess.hpp"


namespace net { namespace impl
{
	using namespace boost;
	using namespace boost::asio;
	using namespace boost::system;
	using namespace async;

	//////////////////////////////////////////////////////////////////////////
	std::string Connector::onSslPassword()
	{
		return "test";
	}

	//////////////////////////////////////////////////////////////////////////
	void Connector::connect_f(Future2<error_code, net::Channel> res, const std::string &host, const std::string &service, bool useSsl)
	{
		//ILOG("connect to "<<host<<":"<<service<<", ssl="<<useSsl);
		TSslContextPtr sslContext;

		if(useSsl)
		{
			mutex::scoped_lock sl(_mtx);

			if(!_sslContext)
			{
				ILOG("create ssl context");
				sslContext.reset(new TSslContext(async::io(), ssl::context::sslv23));

				error_code ec;
				sslContext->set_options(
					  ssl::context::default_workarounds
					| ssl::context::no_sslv2
					| ssl::context::single_dh_use, ec);
				assert(!ec);
				if(ec)
				{
					WLOG("set_options failed: "<<ec);
					res(ec, net::Channel());
					return;
				}

				sslContext->set_password_callback(bind(&Connector::onSslPassword, shared_from_this()));
				sslContext->use_certificate_chain_file("server.pem", ec);
				assert(!ec);
				if(ec)
				{
					WLOG("use_certificate_chain_file failed: "<<ec);
					res(ec, net::Channel());
					return;
				}

				sslContext->use_private_key_file("server.pem", ssl::context::pem, ec);
				assert(!ec);
				if(ec)
				{
					WLOG("use_private_key_file failed: "<<ec);
					res(ec, net::Channel());
					return;
				}

				sslContext->use_tmp_dh_file("dh512.pem", ec);
				assert(!ec);
				if(ec)
				{
					WLOG("use_tmp_dh_file failed: "<<ec);
					res(ec, net::Channel());
					return;
				}
				_sslContext = sslContext;
			}
			else
			{
				sslContext = _sslContext;
			}
		}

		error_code ec;
		//резолвить адрес
		ip::tcp::resolver resolver(async::io());

		Future2<error_code, ip::tcp::resolver::iterator> resolveRes;
		resolver.async_resolve(
			ip::tcp::resolver::query(host, service),
			async::bridge(resolveRes));
		ec = resolveRes.data1();

		if(ec)
		{
			//неудача, вернуть ее
			WLOG("async_resolve failed: "<<ec);
			res(ec, net::Channel());
			return;
		}
		ip::tcp::resolver::iterator riter = resolveRes.data2();
		ip::tcp::resolver::iterator rend = ip::tcp::resolver::iterator();

		//создать сокет
		TSocketSslPtr sockSsl;
		TSocketPtr sock;

		if(sslContext)
		{
			sockSsl.reset(new TSocketSsl(async::io(), *sslContext));
		}
		else
		{
			sock.reset(new TSocket(async::io()));
		}

		//подключать
		ec = error_code();
		for(;;)
		{
			for(; riter!=rend; ++riter)
			{
				Future<error_code> cres;

				if(sockSsl)
				{
					sockSsl->lowest_layer().async_connect(*riter, async::bridge(cres));
				}
				else
				{
					sock->async_connect(*riter, async::bridge(cres));
				}

				ec = cres;
				if(!ec)
				{
					break;
				}
				WLOG("async_connect failed: "<<ec);
			}

			if(ec || riter==rend)
			{
				//неудача, вернуть ее
				res(ec, net::Channel());
				return;
			}

			if(sockSsl)
			{
				Future<error_code> hres;
				sockSsl->async_handshake(ssl::stream_base::client, async::bridge(hres));
				ec = hres;

				if(ec)
				{
					WLOG("handshake failed: "<<ec);
					continue;
				}
				//успех
				//ILOG("success");
				res(error_code(), utils::ImplAccess<net::Channel>(ChannelPtr(new Channel(sockSsl, sslContext))));
				return;
			}
			else
			{
				//успех
				//ILOG("success");
				res(error_code(), utils::ImplAccess<net::Channel>(ChannelPtr(new Channel(sock))));
				return;
			}
		}

	}

	//////////////////////////////////////////////////////////////////////////
	Connector::Connector()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	Connector::~Connector()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	Future2<error_code, net::Channel> Connector::connect(const char *host, const char *service, bool useSsl)
	{
		async::Future2<error_code, net::Channel> res;
		spawn(bind(&Connector::connect_f, shared_from_this(), res, std::string(host), std::string(service), useSsl));
		return res;
	}

}}
