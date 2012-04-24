#include "pch.hpp"
#include "net/http/impl/contentFilterEncodeChunked.hpp"

#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/karma_string.hpp>
#include <boost/spirit/include/karma_uint.hpp>

#include <boost/foreach.hpp>


namespace net { namespace http { namespace impl
{
	//////////////////////////////////////////////////////////////////////////////
	ContentFilterEncodeChunked::ContentFilterEncodeChunked(ContentFilter* upstream, size_t granula)
		: ContentFilter(upstream)
		, _granula(granula)
		, _size(0)
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

		size_t packetSize = packet._size - offset;

		SChunk chunk = {packet, offset};
		_chunks.push_back(chunk);
		_size += packet._size - offset;

		if(_size >= _granula)
		{
			if(!push2Upstream(false))
			{
				return false;
			}
		}
		return true;
	}
	
	//////////////////////////////////////////////////////////////////////////////
	bool ContentFilterEncodeChunked::filterFlush()
	{
		if(!push2Upstream(true))
		{
			return false;
		}
		return _upstream->filterFlush();
	}


	namespace
	{
		Packet initPacket(const char * data)
		{
			Packet res;

			res._size = strlen(data);
			res._data.reset(new char[res._size]);
			memcpy(res._data.get(), data, res._size);

			return res;
		}
		static Packet lastChunkPacket = initPacket("0\r\n");
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ContentFilterEncodeChunked::push2Upstream(bool finish)
	{
		if(_size)
		{
			Packet header(boost::shared_array<char>(new char[34]), 34);
			char *iter = header._data.get();

			bool genResult = boost::spirit::karma::generate(iter, boost::spirit::karma::uint_generator<size_t, 16>()[boost::spirit::karma::_1 = _size]<<"\r\n");
			assert(genResult);
			(void)genResult;

			header._size = iter - header._data.get();

			if(!_upstream->filterPush(header, 0))
			{
				return false;
			}

			BOOST_FOREACH(SChunk &chunk, _chunks)
			{
				if(!_upstream->filterPush(chunk._packet, chunk._offset))
				{
					return false;
				}
			}
			_chunks.clear();
			_size = 0;
		}

		if(finish)
		{
			if(!_upstream->filterPush(lastChunkPacket, 0))
			{
				return false;
			}
		}

		return true;
	}
}}}
