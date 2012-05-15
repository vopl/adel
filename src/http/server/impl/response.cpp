#include "pch.hpp"
#include "http/server/impl/response.hpp"
#include "http/server/impl/request.hpp"
#include "http/impl/server.hpp"
#include "http/impl/contentFilterEncodeChunked.hpp"
#include "http/impl/contentFilterEncodeZlib.hpp"
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
	Response::Response(const http::impl::ServerPtr &server, const net::Channel &channel, Request *request)
		: http::impl::OutputMessage(channel, server->responseWriteGranula())
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
	boost::system::error_code Response::bodyFlush()
	{
		boost::system::error_code ec;
		if((ec = http::impl::OutputMessage::bodyFlush()))
		{
			return ec;
		}

		if(_keepAlive)
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
	boost::system::error_code Response::writeSystemHeaders()
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
			if((ec = header(hn::connection, HeaderValue<Connection>(ec_keepAlive))))
			{
				return ec;
			}
		}
		else
		{
			if(hvConnection.isCorrect() || _version>=Version(1,1))
			{
				if((ec = header(hn::connection, HeaderValue<Connection>(ec_close))))
				{
					return ec;
				}
			}
		}

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
		if(_chunked)
		{
			http::impl::ContentFilterPtr cf(new http::impl::ContentFilterEncodeChunked(_contentFilter, _server->responseWriteGranula()));
			_contentFilter = cf;
		}

		if(_contentEncoding != ece_identity)
		{
			assert(ece_gzip == _contentEncoding || ece_deflate == _contentEncoding);
			http::impl::ContentFilterPtr cf(new http::impl::ContentFilterEncodeZlib(_contentFilter, _contentEncoding, _contentEncodingCompressLevel, _server->responseWriteGranula()));
			_contentFilter = cf;
		}

		return http::impl::OutputMessage::setupBodyFilters();
	}


}}}
