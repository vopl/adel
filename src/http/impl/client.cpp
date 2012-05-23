#include "pch.hpp"
#include "http/impl/client.hpp"
#include "utils/ntoa.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_string.hpp>
#include <boost/spirit/include/qi_char.hpp>

#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_core.hpp>

namespace http { namespace impl
{
	namespace po = boost::program_options;
	using namespace http::client;

	//////////////////////////////////////////////////////////////////////
	utils::OptionsPtr Client::prepareOptions(const char *prefix)
	{
		utils::OptionsPtr options(new utils::Options(prefix));
		options->addOption(
			"request.writeGranula",
			po::value<size_t>()->default_value(32768),
			"buffer size during write request data");

		options->addOption(
			"response.readGranula",
			po::value<size_t>()->default_value(1024),
			"buffer size during read response data");

		options->addOption(
			"timeout",
			po::value<size_t>()->default_value(10000),
			"read/write timeout in milliseconds");

		return options;

	}

	//////////////////////////////////////////////////////////////////////
	Client::Client()
		: _responseReadGranula(1024)
		, _requestWriteGranula(32768)
		, _timeout(10000)
	{
	}

	//////////////////////////////////////////////////////////////////////
	Client::~Client()
	{
	}

	//////////////////////////////////////////////////////////////////////
	void Client::init(utils::OptionsPtr options)
	{
		utils::Options &o = *options;

		_responseReadGranula = o["response.readGranula"].as<size_t>();
		_requestWriteGranula = o["request.writeGranula"].as<size_t>();
		_timeout = o["timeout"].as<size_t>();

	}

	//////////////////////////////////////////////////////////////////////
	boost::system::error_code Client::connect(
		client::impl::RequestPtr &request,
		const char *host, const char *service, bool useSsl)
	{
		async::Future2<boost::system::error_code, net::Channel> cres =
			_connector.connect(host, service, useSsl);
		cres.wait();

		if(cres.data1NoWait())
		{
			return cres.data1NoWait();
		}

		std::string headerHost = host;

		unsigned short port = cres.data2NoWait().endpointRemote().port();

		if((port!=80 && !useSsl) || (port!=443 && useSsl))
		{
			char tmp[32];
			tmp[0] = ':';
			utils::_ntoa(port, tmp+1);
			headerHost += tmp;
		}

		request.reset(new client::impl::Request(shared_from_this(), cres.data2NoWait(), headerHost));
		return cres.data1NoWait();
	}

	namespace
	{
		struct UrlParts
		{
			std::string host;
			std::string service;

			bool		useSsl;

			std::string path;
		};

		boost::system::error_code parseUrl(const char *url, UrlParts *res)
		{
			// [shceme://]host[:port][/path]#anchor

			namespace qi = boost::spirit::qi;
			using namespace qi;
			namespace px = boost::phoenix;

			const char *begin = url;
			const char *end = url + strlen(url);

			bool schemaHttp = false;
			bool schemaHttps = false;

			boost::iterator_range<const char *> host, service, path;

			bool b = qi::parse(begin, end,
				//scheme
				-(lit("http") >> (lit('s')[px::ref(schemaHttps)=true] || eps[px::ref(schemaHttp)=true]) >> "://") >>

				//host
				(
					raw[+(char_ - char_(":/?#"))][px::ref(host) = qi::_1]
				) >>

				//port
				(
					-(lit(':') >> raw[+char_("0-9")][px::ref(service) = qi::_1])
				) >>

				//path
				(
					-(raw[+(char_ - char_("#"))][px::ref(path) = qi::_1])
				) >>

				//anchor
				-(lit('#') >> +char_)
			);

			if(!b)
			{
				return http::error::make(http::error::bad_url);
			}

			res->host.assign(host.begin(), host.end());
			res->service.assign(service.begin(), service.end());
			res->path.assign(path.begin(), path.end());

			if(res->service.empty())
			{
				if(schemaHttps)
				{
					res->service = "443";
				}
				else
				{
					res->service = "80";
				}
			}

			if(res->path.empty())
			{
				res->path = '/';
			}
			res->useSsl = schemaHttps || (!schemaHttp && res->service=="443");

			return http::error::make();
		}
	}

	//////////////////////////////////////////////////////////////////////
	boost::system::error_code Client::connectGet(
		client::impl::RequestPtr &request,
		const char *url,
		const Version &version)
	{
		boost::system::error_code ec;

		UrlParts urlParts;
		if((ec = parseUrl(url, &urlParts)))
		{
			return ec;
		}

		if((ec = connect(request, urlParts.host.c_str(), urlParts.service.c_str(), urlParts.useSsl)))
		{
			return ec;
		}

		if((ec = request->firstLine(em_GET, urlParts.path.c_str(), urlParts.path.size(), version)))
		{
			return ec;
		}

		return http::error::make();
	}

	//////////////////////////////////////////////////////////////////////
	boost::system::error_code Client::get(
		client::impl::ResponsePtr &response,
		const char *url,
		const Version &version)
	{
		client::impl::RequestPtr request;
		boost::system::error_code ec;;

		if((ec = connectGet(request, url, version)))
		{
			return ec;
		}

		if((ec = request->bodyFlush()))
		{
			return ec;
		}

		response = request->response();

		return http::error::make();
	}

	//////////////////////////////////////////////////////////////////////
	size_t Client::requestWriteGranula() const
	{
		return _requestWriteGranula;
	}

	//////////////////////////////////////////////////////////////////////
	size_t Client::responseReadGranula() const
	{
		return _responseReadGranula;
	}

}}
