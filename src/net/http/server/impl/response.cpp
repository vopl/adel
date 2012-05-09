#include "pch.hpp"
#include "net/http/server/impl/response.hpp"
#include "net/http/server/impl/request.hpp"
#include "net/http/impl/server.hpp"
#include "net/http/impl/messageBuffer.hpp"
#include "net/http/impl/contentFilterEncodeChunked.hpp"
#include "net/http/impl/contentFilterEncodeZlib.hpp"
#include "net/http/headerName.hpp"

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
	Response::Response(const net::http::impl::ServerPtr &server, const Channel &channel, Request *request)
		: _server(server)
		, _channel(channel)
		, _request(request)
		, _ewp(ewp_statusLine)
		//, _mostContentFilter(this)
		, _outputGranula(_server->responseWriteGranula())
		, _bodySize(_unknownBodySize)
		, _bodyCompressLevel(1)
		, _bodyCompressGranula(_server->responseWriteGranula())
		, _keepAlive(false)
	{
		//_mostContentFilter = this;

		if(obtainMoreBuffers(true))
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

	/////////////////////////////////////////////////////////////////////
	void Response::header(const HeaderName &name, const char *value, size_t valueSize)
	{
		MessageIterator outIter = beginWriteHeader(name.str.data(), name.str.size());
		outIter = std::copy(value, value+valueSize, outIter);
		endWriteHeader(outIter);
	}

	/////////////////////////////////////////////////////////////////////
	void Response::header(const HeaderName &name, const char *valuez)
	{
		return header(name, valuez, strlen(valuez));
	}

	/////////////////////////////////////////////////////////////////////
	void Response::header(const HeaderName &name, const std::string &value)
	{
		return header(name, value.data(), value.size());
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
	MessageIterator Response::getWriteIterator()
	{
		return _writePosition;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	void Response::setWriteIterator(MessageIterator iter)
	{
		_writePosition = iter;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	bool Response::flush()
	{
		body(NULL, 0);

		net::http::impl::Message::dropTail(_writePosition.absolutePosition());
		if(!pushFullBuffers2Filter())
		{
			return false;
		}
		if(!_mostContentFilter)
		{
			_mostContentFilter = shared_from_this();
		}
		if(!_mostContentFilter->filterFlush())
		{
			return false;
		}

		if(_keepAlive)
		{
			_request->reinit();
			_server->onRequest(_request->shared_from_this());
			//_channel.close();
		}
		else
		{
			_channel.close();
		}
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	void Response::setBodySize(size_t size)
	{
		_bodySize = size;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	void Response::setBodyCompress(int level, size_t granula)
	{
		_bodyCompressLevel = level;
		_bodyCompressGranula = granula;
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
	void Response::endWriteHeader(MessageIterator iter)
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
	bool Response::pushFullBuffers2Filter()
	{
		size_t bodyOffset = _bodyPosition.isEndInfinity()?std::numeric_limits<size_t>::max():_bodyPosition.absolutePosition();
		size_t writeOffset = _writePosition.isEndInfinity()?0:_writePosition.absolutePosition();

		while(_firstBuffer)
		{
			if(writeOffset < _firstBuffer->offset() + _firstBuffer->size())
			{
				return true;
			}

			net::http::impl::MessageBufferPtr firstBuffer = _firstBuffer;
			net::http::impl::Message::dropFront();

			size_t offsetInPacket;
			Packet packet = firstBuffer->asPacket(offsetInPacket);

			if(!_mostContentFilter)
			{
				_mostContentFilter = shared_from_this();
			}
			if(_mostContentFilter == boost::shared_static_cast<net::http::impl::ContentFilter>(shared_from_this()))
			{
				//нет фильтров, и заголовки и тело - все идет подряд

				if(!/*this->*/filterPush(packet, offsetInPacket))
				{
					return false;
				}

			}
			else
			{
				//есть фильтры тела, заголовки без фильтров, тело с фильтрами
				if(bodyOffset < firstBuffer->offset())
				{
					//тело целиком
					if(!_mostContentFilter)
					{
						_mostContentFilter = shared_from_this();
					}
					if(!_mostContentFilter->filterPush(packet, offsetInPacket))
					{
						return false;
					}
				}
				else if(bodyOffset >= firstBuffer->offset() + firstBuffer->size())
				{
					//заголовок целиком
					if(!/*this->*/filterPush(packet, offsetInPacket))
					{
						return false;
					}
				}
				else
				{
					//заголовок и тело
					Packet headersPacket = packet;
					headersPacket._size = offsetInPacket + bodyOffset;
					/*this->*/filterPush(headersPacket, offsetInPacket);

					if(!_mostContentFilter)
					{
						_mostContentFilter = shared_from_this();
					}
					if(!_mostContentFilter->filterPush(packet, bodyOffset + offsetInPacket))
					{
						return false;
					}
				}
			}
		}

		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	bool Response::obtainMoreBuffers(bool force)
	{
		if(!pushFullBuffers2Filter())
		{
			return false;
		}

		size_t size = _outputGranula;
		Packet packet(boost::shared_array<char>(new char[size]), size);
		pushBuffer(packet);
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
		//keep alive
		HeaderValue<Connection> hvConnection(_request->header(hn::connection));
		if(!hvConnection.isCorrect() && _version >= Version(1,1))
		{
			hvConnection = ec_keepAlive;
		}

		//chunked
		HeaderValue<TransferEncoding> hvTransferEncoding(_request->header(hn::te));
		if(!hvTransferEncoding.isCorrect())
		{
			if(_version >= Version(1,1))
			{
				hvTransferEncoding = ete_chunked;
			}
		}
		else
		{
			if(hvTransferEncoding.value() & ete_chunked)
			{
				hvTransferEncoding = ete_chunked;
			}
			else
			{
				hvTransferEncoding.setIsCorrect(false);
			}
		}

		//content encoding
		HeaderValue<ContentEncoding> hvContentEncoding(_request->header(hn::acceptEncoding));
		if(hvContentEncoding.isCorrect())
		{
			if(hvContentEncoding.value() & ece_deflate)
			{
				hvContentEncoding = ece_deflate;
			}
			else if(hvContentEncoding.value() & ece_gzip)
			{
				hvContentEncoding = ece_gzip;
			}
			else
			{
				hvContentEncoding.setIsCorrect(false);
			}
		}

		//content length
		HeaderValue<Unsigned> hvContentLength;
		if(_unknownBodySize != _bodySize)
		{
			//будет тело
			hvContentLength = _bodySize;

			if(_bodySize)
			{
				bool compress = hvContentEncoding.isCorrect() && (hvContentEncoding.value()&(ece_deflate|ece_gzip));
				bool chunked = hvTransferEncoding.isCorrect() && (hvTransferEncoding.value()&ete_chunked);
				bool contentLength = hvContentLength.isCorrect();
				bool keepAlive = hvConnection.isCorrect() && (hvConnection.value()==ec_keepAlive);

				//логика по сжатию
				if(_bodyCompressLevel)
				{
					//нужен compress && (chunked || !keepAlive)
					if(compress)
					{
						//у пресованного потока пока длина не известна
						contentLength = false;
						hvContentLength.setIsCorrect(false);

						if(!chunked && keepAlive)
						{
							//невозможно определить длину, бросить keepAlive
							keepAlive = false;
							hvConnection.setIsCorrect(false);
						}
					}
					else
					{
						_bodyCompressLevel = 0;
					}
				}

				if(!_bodyCompressLevel)
				{
					//без компрессии
					if(contentLength && chunked)
					{
						//chunked или contentLength лишний
						chunked = false;
						hvTransferEncoding.setIsCorrect(false);
					}
					hvContentEncoding.setIsCorrect(false);
				}
			}
			else//if(_bodySize)
			{
				hvContentEncoding.setIsCorrect(false);
				hvTransferEncoding.setIsCorrect(false);
			}

		}
		else
		{
			//неизвестно, будет тело или нет
			hvContentLength.setIsCorrect(false);
			bool chunked = hvTransferEncoding.isCorrect() && (hvTransferEncoding.value()&ete_chunked);
			bool keepAlive = hvConnection.isCorrect() && (hvConnection.value()==ec_keepAlive);

			if(!chunked && keepAlive)
			{
				//невозможно определить длину, бросить keepAlive
				keepAlive = false;
				hvConnection.setIsCorrect(false);
			}
		}


		//писать заголовки
		if(hvContentLength.isCorrect())
		{
			header(hn::contentLength, hvContentLength);
		}
		if(hvTransferEncoding.isCorrect())
		{
			header(hn::transferEncoding, hvTransferEncoding);

			if(!_mostContentFilter)
			{
				_mostContentFilter = shared_from_this();
			}
			net::http::impl::ContentFilterPtr ch(new net::http::impl::ContentFilterEncodeChunked(_mostContentFilter, _outputGranula));
			_mostContentFilter = ch;
			_filterKeeper.push_back(ch);
		}

		if(hvContentEncoding.isCorrect())
		{
			if(hvContentEncoding.value() & ece_deflate)
			{
				hvContentEncoding.value() = ece_deflate;

				if(!_mostContentFilter)
				{
					_mostContentFilter = shared_from_this();
				}
				net::http::impl::ContentFilterPtr ch(new net::http::impl::ContentFilterEncodeZlib(
					_mostContentFilter,
					net::http::ece_deflate,
					_bodyCompressLevel,
					_bodyCompressGranula));

				_mostContentFilter = ch;
				_filterKeeper.push_back(ch);

			}
			else if(hvContentEncoding.value() & ece_gzip)
			{
				hvContentEncoding.value() = ece_gzip;

				if(!_mostContentFilter)
				{
					_mostContentFilter = shared_from_this();
				}
				net::http::impl::ContentFilterPtr ch(new net::http::impl::ContentFilterEncodeZlib(
					_mostContentFilter,
					net::http::ece_gzip,
					_bodyCompressLevel,
					_bodyCompressGranula));

				_mostContentFilter = ch;
				_filterKeeper.push_back(ch);

			}
			else
			{
				hvContentEncoding.value() = ece_identity;
			}

			header(hn::contentEncoding, hvContentEncoding);
		}
		if(hvConnection.isCorrect())
		{
			header(hn::connection, hvConnection);

			_keepAlive = hvConnection.value() == ec_keepAlive;
		}
		else
		{
			_keepAlive = false;
		}
		header(hn::date, HeaderValue<Date>(time(NULL)));
		header(hn::server, "haws", 4);

	}

}}}}
