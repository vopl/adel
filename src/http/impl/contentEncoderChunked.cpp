#include "pch.hpp"
#include "http/impl/contentFilterEncodeChunked.hpp"
#include "http/error.hpp"

#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/karma_string.hpp>
#include <boost/spirit/include/karma_uint.hpp>

#include <boost/foreach.hpp>


namespace http { namespace impl
{
	//////////////////////////////////////////////////////////////////////////////
	ContentFilterEncodeChunked::ContentFilterEncodeChunked(ContentFilterPtr upstream, size_t granula)
		: _upstream(upstream)
		, _granula(granula)
		, _size(0)
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	ContentFilterEncodeChunked::~ContentFilterEncodeChunked()
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	boost::system::error_code ContentFilterEncodeChunked::filterPush(const net::Packet &packet, size_t offset)
	{
		assert(offset < packet._size);

		size_t packetSize = packet._size - offset;

		SChunk chunk = {packet, offset};
		_chunks.push_back(chunk);
		_size += packet._size - offset;

		if(_size >= _granula)
		{
			boost::system::error_code ec;
			if((ec = push2Upstream(false)))
			{
				return ec;
			}
		}
		return http::error::make();
	}
	
	//////////////////////////////////////////////////////////////////////////////
	boost::system::error_code ContentFilterEncodeChunked::filterFlush()
	{
		boost::system::error_code ec;
		if((ec = push2Upstream(true)))
		{
			return ec;
		}
		return _upstream->filterFlush();
	}


	namespace
	{
		net::Packet initPacket(const char * data)
		{
			net::Packet res;

			res._size = strlen(data);
			res._data.reset(new char[res._size]);
			memcpy(res._data.get(), data, res._size);

			return res;
		}
		static net::Packet lastChunkPacket = initPacket("0\r\n\r\n");
		static net::Packet crlfPacket = initPacket("\r\n");
	}
	//////////////////////////////////////////////////////////////////////////////
	boost::system::error_code ContentFilterEncodeChunked::push2Upstream(bool finish)
	{
		boost::system::error_code ec;
		if(_size)
		{
			net::Packet header(boost::shared_array<char>(new char[34]), 34);
			char *iter = header._data.get();

			bool genResult = boost::spirit::karma::generate(iter, boost::spirit::karma::uint_generator<size_t, 16>()[boost::spirit::karma::_1 = _size]<<"\r\n");
			assert(genResult);
			(void)genResult;

			header._size = iter - header._data.get();

			if((ec = _upstream->filterPush(header, 0)))
			{
				return ec;
			}

			BOOST_FOREACH(SChunk &chunk, _chunks)
			{
				if((ec = _upstream->filterPush(chunk._packet, chunk._offset)))
				{
					return ec;
				}
			}
			_chunks.clear();
			_size = 0;

			if((ec = _upstream->filterPush(crlfPacket, 0)))
			{
				return ec;
			}

		}

		if(finish)
		{
			if((ec = _upstream->filterPush(lastChunkPacket, 0)))
			{
				return ec;
			}
		}

		return http::error::make();
	}
}}
