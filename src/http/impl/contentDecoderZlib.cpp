#include "pch.hpp"
#include "http/impl/contentDecoderZlib.hpp"
#include "http/log.hpp"
#include "http/error.hpp"


namespace http { namespace impl
{
	//////////////////////////////////////////////////////////////////////////////
	ContentDecoderZlib::ContentDecoderZlib(const ContentDecoderPtr &upstream, EContentEncoding ece, size_t granula)
		: _upstream(upstream)
		, _ece(ece)
		, _granula(granula)
		, _outputOffset(0)
		, _streamEnd(false)
	{
		switch(_ece)
		{
		case ece_unknown:
			break;
		case ece_identity:
			break;
		case ece_deflate:
			{
				memset(&_z_stream, 0, sizeof(_z_stream));
				int i = inflateInit(&_z_stream);
				if(Z_OK != i)
				{
					ELOG("inflateInit failed: "<<i<<" ("<<(_z_stream.msg?_z_stream.msg:"no message")<<")");
					_ece = ece_unknown;
					break;
				}
			}
			break;
		case ece_gzip:
			{
				memset(&_z_stream, 0, sizeof(_z_stream));

				int i = inflateInit2(
					&_z_stream,
					15+16);
				if(Z_OK != i)
				{
					ELOG("inflateInit2 failed: "<<i<<" ("<<(_z_stream.msg?_z_stream.msg:"no message")<<")");
					_ece = ece_unknown;
					break;
				}
			}
			break;
		case ece_compress:
			assert(0);
			break;
		default:
			_ece = ece_unknown;
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////////
	ContentDecoderZlib::~ContentDecoderZlib()
	{
		switch(_ece)
		{
		case ece_unknown:
			break;
		case ece_identity:
			break;
		case ece_deflate:
		case ece_gzip:
			inflateEnd(&_z_stream);
			break;
		case ece_compress:
			assert(0);
			break;
		default:
			assert(!"wtf?");
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////////
	boost::system::error_code ContentDecoderZlib::push(const net::Packet &packet, size_t offset)
	{
		switch(_ece)
		{
		case ece_unknown:
			return _upstream->push(packet, offset);
		case ece_identity:
			return _upstream->push(packet, offset);
		case ece_deflate:
		case ece_gzip:
			{
				_z_stream.next_in = (Bytef*)(packet._data.get() + offset);
				_z_stream.avail_in = (uInt)(packet._size - offset);

				while(_z_stream.avail_in)
				{
					if(!_output._data)
					{
						assert(!_outputOffset);
						_output._data.reset(new char[_granula]);
						_output._size = _granula;
					}
					_z_stream.next_out = (Bytef*)(_output._data.get() + _outputOffset);
					_z_stream.avail_out = (uInt)(_output._size - _outputOffset);

					int i = inflate(&_z_stream, Z_NO_FLUSH);

					_outputOffset = _output._size - _z_stream.avail_out;

					_streamEnd = Z_STREAM_END == i;
					switch(i)
					{
					case Z_OK:
					case Z_STREAM_END:
					case Z_BUF_ERROR:
						_output._size = _outputOffset;
						assert(_output._size);
						{
							boost::system::error_code ec;
							if((ec = _upstream->push(_output)))
							{
								return ec;
							}
						}
						_outputOffset = 0;
						_output._size = 0;
						_output._data.reset();
						break;
					default:
						ELOG("inflate failed: "<<i<<" ("<<(_z_stream.msg?_z_stream.msg:"no message")<<")");
						return http::error::make(http::error::unexpected);
					}
				}
				_z_stream.next_in = NULL;
				_z_stream.avail_in = 0;
			}
			return http::error::make();
		case ece_compress:
			assert(0);
			return http::error::make(http::error::not_implemented);
		default:
			assert(!"wtf?");
			return http::error::make(http::error::unexpected);
		}

		return _upstream->push(packet, offset);
	}

	//////////////////////////////////////////////////////////////////////////////
	boost::system::error_code ContentDecoderZlib::flush()
	{
		switch(_ece)
		{
		case ece_unknown:
			return _upstream->flush();
		case ece_identity:
			return _upstream->flush();
		case ece_deflate:
		case ece_gzip:
			{
				_z_stream.next_in = NULL;
				_z_stream.avail_in = 0;

				while(!_streamEnd)
				{
					if(!_output._data)
					{
						assert(!_outputOffset);
						_output._data.reset(new char[_granula]);
						_output._size = _granula;
					}
					_z_stream.next_out = (Bytef*)(_output._data.get() + _outputOffset);
					_z_stream.avail_out = (uInt)(_output._size - _outputOffset);

					int i = inflate(&_z_stream, Z_FINISH);

					_outputOffset = _output._size - _z_stream.avail_out;
					_streamEnd = Z_STREAM_END == i;

					switch(i)
					{
					case Z_OK:
					case Z_BUF_ERROR:
						_output._size = _outputOffset;
						{
							boost::system::error_code ec;
							if((ec = _upstream->push(_output)))
							{
								return ec;
							}
						}
						_outputOffset = 0;
						_output._size = 0;
						_output._data.reset();
						break;
					case Z_STREAM_END:
						_output._size = _outputOffset;
						if(_output._size)
						{
							boost::system::error_code ec;
							if((ec = _upstream->push(_output)))
							{
								return ec;
							}
						}
						_outputOffset = 0;
						_output._size = 0;
						_output._data.reset();
						return _upstream->flush();
					default:
						ELOG("inflate failed: "<<i<<" ("<<(_z_stream.msg?_z_stream.msg:"no message")<<")");
						return http::error::make(http::error::not_implemented);
					}
				}
			}
			break;
		case ece_compress:
			assert(0);
			return http::error::make(http::error::not_implemented);
		default:
			assert(!"wtf?");
			return http::error::make(http::error::unexpected);
		}

		return _upstream->flush();
	}

}}
