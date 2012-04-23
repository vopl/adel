#include "pch.hpp"
#include "net/http/impl/contentFilterEncodeZlib.hpp"
#include "net/log.hpp"

namespace net { namespace http { namespace impl
{
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
		case ece_compress:
			assert(0);
			break;
		case ece_gzip:
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
			deflateEnd(&_z_stream);
			break;
		case ece_compress:
			assert(0);
			break;
		case ece_gzip:
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
			{
				_z_stream.next_in = (Bytef*)(packet._data.get() + offset);
				_z_stream.avail_in = packet._size - offset;

				while(_z_stream.avail_in)
				{
					if(!_output._data)
					{
						assert(!_outputOffset);
						_output._data.reset(new char[_granula]);
						_output._size = _granula;
					}
					_z_stream.next_out = (Bytef*)(_output._data.get() + _outputOffset);
					_z_stream.avail_out = _output._size - _outputOffset;

					int i = deflate(&_z_stream, Z_NO_FLUSH);

					_outputOffset = _output._size - _z_stream.avail_out;
					switch(i)
					{
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
					case Z_OK:
						_z_stream.next_in = NULL;
						break;
					default:
						ELOG("deflate failed: "<<i<<" ("<<(_z_stream.msg?_z_stream.msg:"no message")<<")");
						return false;
					}
				}
			}
			return true;
		case ece_compress:
			assert(0);
			break;
		case ece_gzip:
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
					_z_stream.avail_out = _output._size - _outputOffset;

					int i = deflate(&_z_stream, Z_FINISH);

					_outputOffset = _output._size - _z_stream.avail_out;
					switch(i)
					{
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
		case ece_gzip:
			assert(0);
			break;
		default:
			assert(!"wtf?");
			break;
		}

		return _upstream->filterFlush();
	}

}}}
