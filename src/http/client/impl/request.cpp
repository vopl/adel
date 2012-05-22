#include "pch.hpp"
#include "http/client/impl/request.hpp"
#include "http/impl/client.hpp"
#include "http/impl/contentEncoderChunked.hpp"
#include "http/impl/contentEncoderZlib.hpp"
#include "http/headerName.hpp"

#include "http/client/log.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_string.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_lit.hpp>

#include <boost/spirit/include/phoenix_core.hpp>

namespace http { namespace client { namespace impl
{

	//////////////////////////////////////////////////////////////////////////
	Request::Request(const http::impl::ClientPtr &client, const net::Channel &channel, const std::string &host)
		: http::impl::OutputMessage(channel, client->requestWriteGranula())
		, _client(client)
		, _host(host)
	{
		_version = Version(1,1);
	}

	//////////////////////////////////////////////////////////////////////////
	Request::~Request()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code Request::firstLine(EMethod method, const char *path, size_t pathSize, const Version &version)
	{
		Iterator iter = firstLineIterator();
		boost::system::error_code ec;

		switch(method)
		{
		default:
		case em_UNKNOWN:
			return http::error::make(http::error::wrong_value);
		case em_OPTIONS:
			ec = write("OPTIONS ", 8);
			break;
		case em_GET:
			ec = write("GET ", 4);
			break;
		case em_POST:
			ec = write("POST ", 5);
			break;
		case em_HEAD:
			ec = write("HEAD ", 5);
			break;
		case em_TRACE:
			ec = write("TRACE ", 6);
			break;
		case em_PUT:
			ec = write("PUT ", 4);
			break;
		case em_DELETE:
			ec = write("DELETE ", 7);
			break;
		case em_CONNECT:
			ec = write("CONNECT ", 8);
			break;
		}

		if(ec)
		{
			return ec;
		}

		if((ec = write(path, pathSize)))
		{
			return ec;
		}

		if((ec = write(" HTTP/", 6)))
		{
			return ec;
		}

		using namespace boost::spirit::karma;
		namespace karma = boost::spirit::karma;
		namespace px = boost::phoenix;

		bool b = generate(iter,
			uint_[karma::_1 = version._hi]<<'.'<<uint_[karma::_1 = version._lo]);

		if(!b)
		{
			return error::make(error::unexpected);
		}

		return error::make();
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code Request::firstLine(EMethod method, const char *pathz, const Version &version)
	{
		return firstLine(method, pathz, strlen(pathz), version);
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code Request::firstLine(EMethod method, const std::string &path, const Version &version)
	{
		return firstLine(method, path.data(), path.size(), version);
	}

	//////////////////////////////////////////////////////////////////////////
	ResponsePtr Request::response()
	{
		if(!_response)
		{
			_response.reset(new Response(_client, _channel, this));
		}

		return _response;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	boost::system::error_code Request::writeSystemHeaders()
	{

		boost::system::error_code ec;

		if((ec = header(hn::host, _host)))
		{
			return ec;
		}

		if(_version < Version(1,1))
		{
			if((ec = header(http::hn::te, "chunked", 7)))
			{
				return ec;
			}
		}

		if((ec = header(http::hn::acceptEncoding, "deflate, gzip", 13)))
		{
			return ec;
		}

		if((ec = header(hn::userAgent, "hawc", 4)))
		{
			return ec;
		}

		return http::impl::OutputMessage::writeSystemHeaders();
	}

	////////////////////////////////////////////////////////////////////////////////////////
	boost::system::error_code Request::setupBodyFilters()
	{
		return http::impl::OutputMessage::setupBodyFilters();
	}


}}}
