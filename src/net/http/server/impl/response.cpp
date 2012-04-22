#include "pch.hpp"
#include "net/http/server/impl/response.hpp"
#include "net/http/impl/server.hpp"

#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/karma_string.hpp>
#include <boost/spirit/include/karma_char.hpp>

#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_container.hpp>


namespace net { namespace http { namespace server { namespace impl
{

	////////////////////////////////////////////////////////////////////////////////////////
	Response::Response(const net::http::impl::ServerPtr &server, const Channel &channel)
		: _server(server)
		, _channel(channel)
		, _ewp(ewp_statusLine)
		, _sendChunk(0)
	{
		if(obtainMoreChunks())
		{
			_writePosition = begin();
		}
		else
		{
			assert(0);
			_writePosition = endInfinity();
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////
	Response::~Response()
	{
	}

	////////////////////////////////////////////////////////////////////////////////////////
	void Response::version(const Version &version)
	{
		_version = version;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	void Response::statusCode(const EStatusCode &statusCode)
	{
		_statusCode = statusCode;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	void Response::statusLine()
	{
		switch(_ewp)
		{
		case ewp_statusLine:
			{
				using namespace boost::spirit::karma;
				namespace karma = boost::spirit::karma;
				namespace px = boost::phoenix;

				bool genResult = generate(_writePosition,
					"HTTP/"<<uint_[karma::_1 = _version._hi]<<'.'<<uint_[karma::_1 = _version._lo]<<' '<<
					uint_[karma::_1 = _statusCode]<<' '<<reasonPhrase(_statusCode)<<"\r\n"
					);

				assert(genResult);
				(void)genResult;
			}
			_ewp = ewp_headers;
			break;
		default:
			assert(0);
			return;
		}
		flush();
	}

	////////////////////////////////////////////////////////////////////////////////////////
	void Response::header(const char *data, size_t size)
	{
		static const char crlf[] = "\r\n";
		switch(_ewp)
		{
		case ewp_statusLine:
			statusLine();
			_writePosition = std::copy(data, data+size, _writePosition);
			_writePosition = std::copy(crlf, crlf+2, _writePosition);
			break;
		case ewp_headers:
			_writePosition = std::copy(data, data+size, _writePosition);
			_writePosition = std::copy(crlf, crlf+2, _writePosition);
			break;
		default:
			assert(0);
			return;
		}
		flush();
	}

	////////////////////////////////////////////////////////////////////////////////////////
	void Response::body(const char *data, size_t size)
	{
		static const char crlf[] = "\r\n";
		switch(_ewp)
		{
		case ewp_statusLine:
			statusLine();
			systemHeaders();
			_writePosition = std::copy(crlf, crlf+2, _writePosition);
			_writePosition = std::copy(data, data+size, _writePosition);
			break;
		case ewp_headers:
			systemHeaders();
			_writePosition = std::copy(crlf, crlf+2, _writePosition);
			_writePosition = std::copy(data, data+size, _writePosition);
			break;
		case ewp_body:
			_writePosition = std::copy(data, data+size, _writePosition);
			break;
		default:
			assert(0);
			return;
		}
		flush();
	}

	////////////////////////////////////////////////////////////////////////////////////////
	bool Response::flush(bool withTail)
	{
		for(; _sendChunk+1 < _chunks.size(); _sendChunk++)
		{
			assert(_chunks[_sendChunk]._packet._data);

			async::Future<boost::system::error_code> sendRes = _channel.send(_chunks[_sendChunk]._packet);
			if(sendRes.data())
			{
				return false;
			}

			_chunks[_sendChunk]._packet._data.reset();
		}

		if(withTail && !_chunks.empty())
		{
			size_t totalLength = _writePosition - begin();
			size_t tailLength = totalLength - _chunks.back()._offset;

			if(tailLength)
			{
				Packet tail = _chunks.back()._packet;
				assert(tailLength <= tail._size);
				tail._size = tailLength;

				async::Future<boost::system::error_code> sendRes = _channel.send(tail);
				if(sendRes.data())
				{
					return false;
				}
			}
		}

		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	bool Response::obtainMoreChunks()
	{
		boost::uint32_t size = _server->responseWriteGranula();
		Packet packet(boost::shared_array<char>(new char[size]), size);
		pushChunk(packet);
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	void Response::systemHeaders()
	{
		header("Server: Apache/2.2.15 (CentOS)", 30);
		//keep alive
		//content-encoding
		//transfer-encoding
	}

}}}}
