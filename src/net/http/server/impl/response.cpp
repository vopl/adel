#include "pch.hpp"
#include "net/http/server/impl/response.hpp"
#include "net/http/server/impl/request.hpp"
#include "net/http/impl/server.hpp"
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
		: net::http::impl::OutputMessage(channel, server->responseWriteGranula())
		, _server(server)
		, _request(request)
		, _version(request->version_())
		, _contentLength(_unknownContentLength)
		, _chunked(false)
		, _keepAlive(false)
		, _contentEncoding(ece_unknown)
		, _contentEncodingCompressLevel(0)
	{
	}

	////////////////////////////////////////////////////////////////////////////////////////
	Response::~Response()
	{
	}

	////////////////////////////////////////////////////////////////////////////////////////
	bool Response::bodyFlush()
	{
		if(!net::http::impl::OutputMessage::bodyFlush())
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
	bool Response::responseLine(const Version &version, const EStatusCode &statusCode)
	{
		_version = version;
		return responseLine(statusCode);
	}

	////////////////////////////////////////////////////////////////////////////////////////
	bool Response::responseLine(const EStatusCode &statusCode)
	{
		Iterator iter = firstLineIterator();
		using namespace boost::spirit::karma;
		namespace karma = boost::spirit::karma;
		namespace px = boost::phoenix;

		return generate(firstLineIterator(),
			"HTTP/"<<uint_[karma::_1 = _version._hi]<<'.'<<uint_[karma::_1 = _version._lo]<<' '<<
			uint_[karma::_1 = statusCode]<<' '<<reasonPhrase(statusCode));
	}

	////////////////////////////////////////////////////////////////////////////////////////
	void Response::setContentLength(size_t size)
	{
		_contentLength = size;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	void Response::setContentCompress(int level)
	{
		_contentEncodingCompressLevel = level;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	bool Response::writeSystemHeaders()
	{
		//keep alive
		HeaderValue<Connection> hvConnection(_request->header(hn::connection));
		if(!hvConnection.isCorrect())
		{
			_keepAlive = _version >= Version(1,1);
		}
		else
		{
			_keepAlive = ec_keepAlive == hvConnection.value();
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

		//писать заголовки
		if(_unknownContentLength != _contentLength)
		{
			if(!header(hn::contentLength, HeaderValue<Unsigned>(_contentLength)))
			{
					return false;
			}
		}

		if(_chunked)
		{
			if(!header(hn::transferEncoding, HeaderValue<TransferEncoding>(ete_chunked)))
			{
					return false;
			}
		}

		if(_contentEncoding != ece_identity)
		{
			if(!header(hn::contentEncoding, HeaderValue<ContentEncoding>(_contentEncoding)))
			{
					return false;
			}
		}

		if(_keepAlive)
		{
			if(!header(hn::connection, HeaderValue<Connection>(ec_keepAlive)))
			{
					return false;
			}
		}
		else
		{
			if(hvConnection.isCorrect() || _version>=Version(1,1))
			{
				if(!header(hn::connection, HeaderValue<Connection>(ec_close)))
				{
						return false;
				}
			}
		}

		if(!header(hn::date, HeaderValue<Date>(time(NULL))))
		{
				return false;
		}

		if(!header(hn::server, "haws", 4))
		{
			return false;
		}

		return net::http::impl::OutputMessage::writeSystemHeaders();
	}

	////////////////////////////////////////////////////////////////////////////////////////
	bool Response::setupBodyFilters()
	{
		if(_chunked)
		{
			net::http::impl::ContentFilterPtr cf(new net::http::impl::ContentFilterEncodeChunked(_contentFilter, _server->responseWriteGranula()));
			_contentFilter = cf;
		}

		if(_contentEncoding != ece_identity)
		{
			assert(ece_gzip == _contentEncoding || ece_deflate == _contentEncoding);
			net::http::impl::ContentFilterPtr cf(new net::http::impl::ContentFilterEncodeZlib(_contentFilter, _contentEncoding, _contentEncodingCompressLevel, _server->responseWriteGranula()));
			_contentFilter = cf;
		}

		return net::http::impl::OutputMessage::setupBodyFilters();
	}


}}}}
