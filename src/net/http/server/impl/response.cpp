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
		: net::http::impl::ContentFilter(NULL)
		, _server(server)
		, _channel(channel)
		, _ewp(ewp_statusLine)
		, _mostContentFilter(this)
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
		_bodyPosition = endInfinity();
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
			_bodyPosition = _writePosition;
			_writePosition = std::copy(data, data+size, _writePosition);
			break;
		case ewp_headers:
			systemHeaders();
			_writePosition = std::copy(crlf, crlf+2, _writePosition);
			_bodyPosition = _writePosition;
			_writePosition = std::copy(data, data+size, _writePosition);
			break;
		case ewp_body:
			_writePosition = std::copy(data, data+size, _writePosition);
			break;
		default:
			assert(0);
			return;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////
	bool Response::flush()
	{
		if(_chunks.empty())
		{
			return true;
		}

		SChunk &lastChunk = _chunks.back();

		if(lastChunk._packet._data)
		{
			assert(_writePosition.absolutePosition() >= lastChunk._offset);
			assert(_writePosition.absolutePosition() < lastChunk._offset+lastChunk._packet._size);

			lastChunk._packet._size = _writePosition.absolutePosition() - lastChunk._offset;
			if(lastChunk._packet._size)
			{
				pushLastChunk();
			}
		}
		_mostContentFilter->filterFlush();

		//_channel.close();
		//keep alive
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	void Response::pushLastChunk()
	{
		if(_chunks.empty())
		{
			return;
		}

		SChunk &lastChunk = _chunks.back();
		if(	_mostContentFilter != this &&
			_bodyPosition.chunkIndex() != Iterator::_badOffset)
		{
			Iterator::size_type curChunkIndex = (Iterator::size_type)(_chunks.size()-1);

			if(curChunkIndex < _bodyPosition.chunkIndex())
			{
				/*this->*/filterPush(lastChunk._packet, 0);
			}
			else if(curChunkIndex == _bodyPosition.chunkIndex())
			{
				Packet headersPart = lastChunk._packet;
				headersPart._size = _bodyPosition.offsetInChunk();
				/*this->*/filterPush(headersPart, 0);
				_mostContentFilter->filterPush(lastChunk._packet, _bodyPosition.offsetInChunk());
			}
			else// curChunkIndex > _bodyPosition.chunkIndex()
			{
				_mostContentFilter->filterPush(lastChunk._packet, 0);
			}
		}
		else
		{
			/*this->*/filterPush(lastChunk._packet, 0);
		}

		lastChunk._packet._data.reset();
	}

	////////////////////////////////////////////////////////////////////////////////////////
	bool Response::obtainMoreChunks()
	{
		pushLastChunk();

		size_t size = _server->responseWriteGranula();
		Packet packet(boost::shared_array<char>(new char[size]), size);
		pushChunk(packet);
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	size_t Response::filterPush(const Packet &packet, size_t offset)
	{
		assert(packet._size > offset);

		if(offset)
		{
			Packet p(
				boost::shared_array<char>(new char[packet._size - offset]),
				packet._size - offset);
			memcpy(p._data.get(), packet._data.get()+offset, p._size);

			_channel.send(p);
			return p._size;
		}

		_channel.send(packet);
		return packet._size;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	size_t Response::filterFlush()
	{
		return 0;
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
