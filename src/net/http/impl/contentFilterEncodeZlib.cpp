#include "pch.hpp"
#include "net/http/impl/contentFilterEncodeZlib.hpp"
#include "net/log.hpp"

namespace net { namespace http { namespace impl
{
	namespace
	{
		voidpf zalloc (voidpf opaque, uInt items, uInt size)
		{
			return malloc(items*size);
		}
		void zfree(voidpf opaque, voidpf address)
		{
			return free(address);
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	ContentFilterEncodeZlib::ContentFilterEncodeZlib(ContentFilter* upstream, EContentEncoding ece, int level, size_t granula)
		: ContentFilter(upstream)
		, _ece(ece)
		, _level(level)
		, _outputOffset(0)
		, _granula(granula)
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
				int i = deflateInit(&_z_stream, _level);
				if(Z_OK != i)
				{
					ELOG("deflateInit failed: "<<i<<" ("<<(_z_stream.msg?_z_stream.msg:"no message")<<")");
					_ece = ece_unknown;
					break;
				}
			}
			break;
		case ece_gzip:
			{
				memset(&_z_stream, 0, sizeof(_z_stream));

				int i = deflateInit2(
					&_z_stream,
					_level,
					Z_DEFLATED,
					15+16,
					8,
					Z_DEFAULT_STRATEGY);
				if(Z_OK != i)
				{
					ELOG("deflateInit2 failed: "<<i<<" ("<<(_z_stream.msg?_z_stream.msg:"no message")<<")");
					_ece = ece_unknown;
					break;
				}

				/*memset(&_gz_header, 0, sizeof(_gz_header));
				_gz_header.os = 255;
				_gz_header.hcrc = true;
				i = deflateSetHeader(&_z_stream, &_gz_header);
				if(Z_OK != i)
				{
					ELOG("deflateSetHeader failed: "<<i<<" ("<<(_z_stream.msg?_z_stream.msg:"no message")<<")");
					_ece = ece_unknown;
					break;
				}*/

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
	ContentFilterEncodeZlib::~ContentFilterEncodeZlib()
	{
		switch(_ece)
		{
		case ece_unknown:
			break;
		case ece_identity:
			break;
		case ece_deflate:
		case ece_gzip:
			deflateEnd(&_z_stream);
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
	bool ContentFilterEncodeZlib::filterPush(const Packet &packet, size_t offset)
	{
		switch(_ece)
		{
		case ece_unknown:
			return _upstream->filterPush(packet, offset);
		case ece_identity:
			return _upstream->filterPush(packet, offset);
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

					int i = deflate(&_z_stream, Z_NO_FLUSH);

					_outputOffset = _output._size - _z_stream.avail_out;
					switch(i)
					{
					case Z_OK:
						if(_z_stream.avail_out)
						{
							break;
						}
					case Z_BUF_ERROR:
						_output._size = _outputOffset;
						assert(_output._size);
						if(!_upstream->filterPush(_output))
						{
							return false;
						}
						_outputOffset = 0;
						_output._size = 0;
						_output._data.reset();
						break;
					default:
						ELOG("deflate failed: "<<i<<" ("<<(_z_stream.msg?_z_stream.msg:"no message")<<")");
						return false;
					}
				}
				_z_stream.next_in = NULL;
				_z_stream.avail_in = NULL;
			}
			return true;
		case ece_compress:
			assert(0);
			break;
		default:
			assert(!"wtf?");
			break;
		}

		return _upstream->filterPush(packet, offset);
	}
	
	//////////////////////////////////////////////////////////////////////////////
	bool ContentFilterEncodeZlib::filterFlush()
	{
		switch(_ece)
		{
		case ece_unknown:
			return _upstream->filterFlush();
		case ece_identity:
			return _upstream->filterFlush();
		case ece_deflate:
		case ece_gzip:
			{
				_z_stream.next_in = NULL;
				_z_stream.avail_in = 0;

				for(;;)
				{
					if(!_output._data)
					{
						assert(!_outputOffset);
						_output._data.reset(new char[_granula]);
						_output._size = _granula;
					}
					_z_stream.next_out = (Bytef*)(_output._data.get() + _outputOffset);
					_z_stream.avail_out = (uInt)(_output._size - _outputOffset);

					int i = deflate(&_z_stream, Z_FINISH);

					_outputOffset = _output._size - _z_stream.avail_out;
					switch(i)
					{
					case Z_OK:
					case Z_BUF_ERROR:
						_output._size = _outputOffset;
						if(!_upstream->filterPush(_output))
						{
							return false;
						}
						_outputOffset = 0;
						_output._size = 0;
						_output._data.reset();
						break;
					case Z_STREAM_END:
						_output._size = _outputOffset;
						if(!_upstream->filterPush(_output))
						{
							return false;
						}
						_outputOffset = 0;
						_output._size = 0;
						_output._data.reset();
						return _upstream->filterFlush();
					default:
						ELOG("deflate failed: "<<i<<" ("<<(_z_stream.msg?_z_stream.msg:"no message")<<")");
						return false;
					}
				}
			}
			break;
		case ece_compress:
			assert(0);
			break;
		default:
			assert(!"wtf?");
			break;
		}

		return _upstream->filterFlush();
	}

}}}
