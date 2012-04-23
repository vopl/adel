#include "pch.hpp"
#include "net/http/impl/contentFilterEncodeChunked.hpp"


namespace net { namespace http { namespace impl
{
	//////////////////////////////////////////////////////////////////////////////
	ContentFilterEncodeChunked::ContentFilterEncodeChunked(ContentFilter* upstream)
		: ContentFilter(upstream)
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	ContentFilterEncodeChunked::~ContentFilterEncodeChunked()
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	bool ContentFilterEncodeChunked::filterPush(const Packet &packet, size_t offset)
	{
		assert(offset < packet._size);
		size_t pushSize = packet._size - offset;

		if(pushSize)
		{
			Packet header;
			header._data.reset(new char[32]);//16+2=18 max
			header._size = sprintf(header._data.get(), "%zx\r\n", pushSize);
			assert(header._size < 32);

			return
				_upstream->filterPush(header, 0) &&
				_upstream->filterPush(packet, offset) &&
				_upstream->filterPush(_chunkFooter, 0);
		}

		return true;
	}
	
	//////////////////////////////////////////////////////////////////////////////
	bool ContentFilterEncodeChunked::filterFlush()
	{
		return
			_upstream->filterPush(_lastChunk, 0) &&
			_upstream->filterFlush();
	}

	//////////////////////////////////////////////////////////////////////////////
	namespace
	{
		Packet initPacket(const char *data)
		{
			Packet p;
			p._size = strlen(data);
			p._data.reset(new char[p._size]);
			memcpy(p._data.get(), data, p._size);
			return p;
		}
	}
	Packet ContentFilterEncodeChunked::_chunkFooter = initPacket("\r\n");
	Packet ContentFilterEncodeChunked::_lastChunk = initPacket("0\r\n");

}}}
