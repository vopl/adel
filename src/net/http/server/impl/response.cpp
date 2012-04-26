#include "pch.hpp"
#include "net/http/server/impl/response.hpp"
#include "net/http/impl/server.hpp"
#include "net/http/impl/contentFilterEncodeChunked.hpp"
#include "net/http/impl/contentFilterEncodeZlib.hpp"

#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/karma_string.hpp>
#include <boost/spirit/include/karma_char.hpp>

#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_container.hpp>

#include "net/http/headerName.hpp"

namespace
{
size_t h1 = net::http::hn::date::hash;
const char *csz = net::http::hn::date::csz();
const char *cszlc = net::http::hn::date::cszlc();

const std::string &str = net::http::hn::date::str();
const std::string &strlc = net::http::hn::date::strlc();
}

namespace net { namespace http { namespace server { namespace impl
{

	////////////////////////////////////////////////////////////////////////////////////////
	Response::Response(const net::http::impl::ServerPtr &server, const Channel &channel)
		: net::http::impl::ContentFilter(NULL)
		, _server(server)
		, _channel(channel)
		, _ewp(ewp_statusLine)
		, _mostContentFilter(this)
		, _outputGranula(_server->responseWriteGranula())
		, _bodySize((size_t)-1)
		, _bodyCompressLevel(1)
		, _bodyCompressBuffer(_server->responseWriteGranula())
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
	void Response::header(const char *dataz)
	{
		return header(dataz, strlen(dataz));
	}

	////////////////////////////////////////////////////////////////////////////////////////
	void Response::header(const std::string &data)
	{
		return header(data.data(), data.size());
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
			_ewp = ewp_body;
			_bodyPosition = _writePosition;
			if(size)
			{
				_writePosition = std::copy(data, data+size, _writePosition);
			}
			break;
		case ewp_headers:
			systemHeaders();
			_writePosition = std::copy(crlf, crlf+2, _writePosition);
			_ewp = ewp_body;
			_bodyPosition = _writePosition;
			if(size)
			{
				_writePosition = std::copy(data, data+size, _writePosition);
			}
			break;
		case ewp_body:
			if(size)
			{
				_writePosition = std::copy(data, data+size, _writePosition);
			}
			break;
		default:
			assert(0);
			return;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////
	bool Response::flush()
	{
		body(NULL, 0);

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
				pushLastChunk2Filter();
			}
		}
		_mostContentFilter->filterFlush();

		//_channel.close();
		//keep alive
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	void Response::setBodySize(size_t size)
	{
		_bodySize = size;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	void Response::setBodyCompress(int level, size_t buffer)
	{
		_bodyCompressLevel = level;
		_bodyCompressBuffer = buffer;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	Message::Iterator Response::beginWriteHeader(const char *name, size_t size)
	{
		static const char nvdelim[] = ": ";
		switch(_ewp)
		{
		case ewp_statusLine:
			statusLine();
			_writePosition = std::copy(name, name+size, _writePosition);
			_writePosition = std::copy(nvdelim, nvdelim+2, _writePosition);
			break;
		case ewp_headers:
			_writePosition = std::copy(name, name+size, _writePosition);
			_writePosition = std::copy(nvdelim, nvdelim+2, _writePosition);
			break;
		default:
			assert(0);
			return _writePosition;
		}
		return _writePosition;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	void Response::endWriteHeader(Message::Iterator iter)
	{
		static const char crlf[] = "\r\n";
		switch(_ewp)
		{
		case ewp_statusLine:
			assert(!"beginWriteHeader was called?");
			statusLine();
			_writePosition = std::copy(crlf, crlf+2, iter);
			break;
		case ewp_headers:
			_writePosition = std::copy(crlf, crlf+2, iter);
			break;
		default:
			assert(0);
			_writePosition = iter;
			return;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////
	void Response::pushLastChunk2Filter()
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
		pushLastChunk2Filter();

		size_t size = _outputGranula;
		Packet packet(boost::shared_array<char>(new char[size]), size);
		pushChunk(packet);
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	bool Response::filterPush(const Packet &packet, size_t offset)
	{
		/*
		if(offset)
		{
			Packet p(boost::shared_array<char>(new char[packet._size - offset]), packet._size - offset);
			memcpy(p._data.get(), packet._data.get()+offset, p._size);
			return _channel.send(p).data()?false:true;

		}
		return _channel.send(packet).data()?false:true;
		*/

		assert(packet._size > offset);
		size_t dataSize = packet._size - offset;
		while(dataSize)
		{
			if(!_output._data)
			{
				_output._data.reset(new char[_outputGranula]);
				_output._size = 0;
			}

			size_t spaceSize = _outputGranula - _output._size;
			if(dataSize >= spaceSize)
			{
				memcpy(_output._data.get()+_output._size, packet._data.get()+offset, spaceSize);
				_output._size += spaceSize;
				offset += spaceSize;
				dataSize -= spaceSize;
				if(!filterFlush())
				{
					return false;
				}
			}
			else
			{
				memcpy(_output._data.get()+_output._size, packet._data.get()+offset, dataSize);
				_output._size += dataSize;
				//offset += dataSize;
				//dataSize -= dataSize;
				return true;
			}

		}
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	bool Response::filterFlush()
	{
		//return true;

		if(_output._data)
		{
			assert(_output._size);
			if(_channel.send(_output).data())
			{
				return false;
			}
			_output._data.reset();
			_output._size = 0;
		}

		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	void Response::systemHeaders()
	{
		//header("Server: Apache/2.2.15 (CentOS)", 30);
		header("Date", HeaderValue<Date>(time(NULL)));

		//TODO: Date

		ContentFilter * ch;
/*
		ch = new net::http::impl::ContentFilterEncodeChunked(_mostContentFilter, _outputGranula);
		_mostContentFilter = ch;
		header("Transfer-Encoding: chunked", 26);
		_filterKeeper.push_back(net::http::impl::ContentFilterPtr(ch));
*/

/*
		ch = new net::http::impl::ContentFilterEncodeZlib(
			_mostContentFilter,
			net::http::ece_deflate,
			_bodyCompressLevel,
			_bodyCompressBuffer);

		_mostContentFilter = ch;
		header("Content-Encoding: deflate", 25);
		_filterKeeper.push_back(net::http::impl::ContentFilterPtr(ch));
//*/

		//keep alive
		//content-encoding
		//transfer-encoding
	}

}}}}
