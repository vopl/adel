#include "pch.hpp"
#include "http/client/impl/request.hpp"
#include "http/impl/client.hpp"
#include "http/impl/contentFilterEncodeChunked.hpp"
#include "http/impl/contentFilterEncodeZlib.hpp"
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
	Request::Request(const http::impl::ClientPtr &client, const net::Channel &channel)
		: http::impl::OutputMessage(channel, client->requestWriteGranula())
		, _client(client)
		, _version(1,1)
		, _contentLength(_unknownContentLength)
		, _chunked(false)
		, _keepAlive(false)
		, _contentEncoding(ece_identity)
		, _contentEncodingCompressLevel(0)
	{
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
	void Request::setContentLength(size_t size)
	{
		_contentLength = size;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	void Request::setContentCompress(int level)
	{
		_contentEncodingCompressLevel = level;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	void Request::setKeepAlive(bool keepAlive)
	{
		_keepAlive = keepAlive;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	void Request::setChunked(bool chunked)
	{
		_chunked = chunked;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	void Request::setContentEncoding(EContentEncoding contentEncoding)
	{
		_contentEncoding = contentEncoding;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	boost::system::error_code Request::writeSystemHeaders()
	{
		//content length
		if(_unknownContentLength != _contentLength)
		{
			//будет тело
			if(_contentLength)
			{
				//логика по сжатию
				if(_contentEncodingCompressLevel)
				{
					//нужен chunked || !keepAlive
					if(_contentEncoding != ece_identity)
					{
						//у пресованного потока пока длина не известна
						_contentLength = _unknownContentLength;

						if(!_chunked && _keepAlive)
						{
							//невозможно определить длину, бросить keepAlive
							_keepAlive = false;
						}
					}
					else
					{
						//без компрессии, клиент не поддерживает
						if(_contentLength && _chunked)
						{
							//chunked или contentLength лишний
							_chunked = false;
						}
					}
				}
				else
				{
					//без сжатия
					_contentEncoding = ece_identity;
				}
			}
			else//if(_contentLength)
			{
				//нулевое тело, бросить кодирование
				_contentEncoding = ece_identity;
				_chunked = false;
			}

		}
		else//if(_unknownContentLength != _contentLength)
		{
			//неизвестно, будет тело или нет
			if(!_chunked && _keepAlive)
			{
				//невозможно определить длину, бросить keepAlive
				_keepAlive = false;
			}
		}

		if(_chunked && _contentLength!=_unknownContentLength)
		{
			_chunked = false;
		}

		boost::system::error_code ec;
		//писать заголовки
		if(_unknownContentLength != _contentLength)
		{
			if((ec = header(hn::contentLength, HeaderValue<Unsigned>(_contentLength))))
			{
				return ec;
			}
		}

		if(_chunked)
		{
			if((ec = header(hn::transferEncoding, HeaderValue<TransferEncoding>(ete_chunked))))
			{
				return ec;
			}
		}

		if(_contentEncoding != ece_identity)
		{
			if((ec = header(hn::contentEncoding, HeaderValue<ContentEncoding>(_contentEncoding))))
			{
				return ec;
			}
		}

		if(_keepAlive)
		{
			if(_version<Version(1,1))
			{
				if((ec = header(hn::connection, HeaderValue<Connection>(ec_keepAlive))))
				{
					return ec;
				}
			}
		}
		else
		{
			if(_version>=Version(1,1))
			{
				if((ec = header(hn::connection, HeaderValue<Connection>(ec_close))))
				{
					return ec;
				}
			}
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
		if(_chunked)
		{
			http::impl::ContentFilterPtr cf(new http::impl::ContentFilterEncodeChunked(_contentFilter, _client->requestWriteGranula()));
			_contentFilter = cf;
		}

		if(_contentEncoding != ece_identity)
		{
			assert(ece_gzip == _contentEncoding || ece_deflate == _contentEncoding);
			http::impl::ContentFilterPtr cf(new http::impl::ContentFilterEncodeZlib(_contentFilter, _contentEncoding, _contentEncodingCompressLevel, _client->requestWriteGranula()));
			_contentFilter = cf;
		}

		return http::impl::OutputMessage::setupBodyFilters();
	}


}}}
