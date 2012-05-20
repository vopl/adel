#include "pch.hpp"
#include "http/server/impl/response.hpp"
#include "http/server/impl/request.hpp"
#include "http/impl/server.hpp"
#include "http/headerName.hpp"

#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/karma_string.hpp>
#include <boost/spirit/include/karma_char.hpp>

#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_container.hpp>


namespace http { namespace server { namespace impl
{

	////////////////////////////////////////////////////////////////////////////////////////
	ResponsePtr Response::shared_from_this()
	{
		return boost::static_pointer_cast<Response>(http::impl::OutputMessage::shared_from_this());
	}

	////////////////////////////////////////////////////////////////////////////////////////
	Response::Response(const http::impl::ServerPtr &server, const net::Channel &channel, Request *request)
		: http::impl::OutputMessage(channel, server->responseWriteGranula())
		, _server(server)
		, _request(request)
	{
		_version = request->version_();
	}

	////////////////////////////////////////////////////////////////////////////////////////
	Response::~Response()
	{
	}

	////////////////////////////////////////////////////////////////////////////////////////
	boost::system::error_code Response::bodyFlush()
	{
		boost::system::error_code ec;
		if((ec = http::impl::OutputMessage::bodyFlush()))
		{
			return ec;
		}

		if(ec_keepAlive == _keepAlive)
		{
			RequestPtr r = _request->shared_from_this();
			_request->reinit();
			_server->onRequest(r);
			//_channel.close();
		}
		else
		{
			_channel.close();
		}
		return error::make();
	}


	////////////////////////////////////////////////////////////////////////////////////////
	boost::system::error_code Response::firstLine(const Version &version, const EStatusCode &statusCode)
	{
		_version = version;
		return firstLine(statusCode);
	}

	////////////////////////////////////////////////////////////////////////////////////////
	boost::system::error_code Response::firstLine(const EStatusCode &statusCode)
	{
		Iterator iter = firstLineIterator();
		using namespace boost::spirit::karma;
		namespace karma = boost::spirit::karma;
		namespace px = boost::phoenix;

		bool b = generate(iter,
			"HTTP/"<<uint_[karma::_1 = _version._hi]<<'.'<<uint_[karma::_1 = _version._lo]<<' '<<
			uint_[karma::_1 = statusCode]<<' '<<reasonPhrase(statusCode));

		if(!b)
		{
			return error::make(error::unexpected);
		}

		return error::make();
	}

	////////////////////////////////////////////////////////////////////////////////////////
	boost::system::error_code Response::writeSystemHeaders()
	{
		//keep alive
		HeaderValue<Connection> hvConnection(_request->header(hn::connection));
		if(!hvConnection.isCorrect())
		{
			if(_version >= Version(1,1))
			{
				_keepAlive = ec_keepAlive;
			}
			else
			{
				//undef -> close
			}
		}
		else
		{
			_keepAlive = hvConnection.value();
		}

		//chunked
		HeaderValue<TransferEncoding> hvTransferEncoding(_request->header(hn::te));
		if(!hvTransferEncoding.isCorrect())
		{
			if(_version >= Version(1,1))
			{
				_chunked = true;
			}
		}
		else
		{
			if(hvTransferEncoding.value() & ete_chunked)
			{
				_chunked = true;
			}
			else
			{
				_chunked = false;
			}
		}

		//content encoding
		HeaderValue<ContentEncoding> hvContentEncoding(_request->header(hn::acceptEncoding));
		if(hvContentEncoding.isCorrect())
		{
			if(hvContentEncoding.value() & ece_deflate)
			{
				_contentEncoding = ece_deflate;
			}
			else if(hvContentEncoding.value() & ece_gzip)
			{
				_contentEncoding = ece_gzip;
			}
			else
			{
				_contentEncoding = ece_identity;
			}
		}
		else
		{
			_contentEncoding = ece_identity;
		}

		boost::system::error_code ec;
		if((ec = header(hn::date, HeaderValue<Date>(time(NULL)))))
		{
			return ec;
		}

		if((ec = header(hn::server, "haws", 4)))
		{
			return ec;
		}

		return http::impl::OutputMessage::writeSystemHeaders();
	}

	////////////////////////////////////////////////////////////////////////////////////////
	boost::system::error_code Response::setupBodyFilters()
	{
		return http::impl::OutputMessage::setupBodyFilters();
	}


}}}
