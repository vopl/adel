#include "pch.hpp"
#include "net/http/impl/contentFilterEncodeChunked.hpp"

#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/karma_string.hpp>
#include <boost/spirit/include/karma_uint.hpp>



namespace net { namespace http { namespace impl
{
	//////////////////////////////////////////////////////////////////////////////
	ContentFilterEncodeChunked::ContentFilterEncodeChunked(ContentFilter* upstream, size_t granula)
		: ContentFilter(upstream)
		, _granula(granula)
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
		return push(packet._data.get() + offset, packet._size - offset);
	}
	
	//////////////////////////////////////////////////////////////////////////////
	bool ContentFilterEncodeChunked::filterFlush()
	{
		if(!flush(true))
		{
			return false;
		}
		return _upstream->filterFlush();
	}

	//////////////////////////////////////////////////////////////////////////////
	bool ContentFilterEncodeChunked::push(const char *data, size_t size)
	{
		while(size)
		{
			if(!_output._data)
			{
				_output._data.reset(new char[32+2 + _granula + 5]);
				_output._size = 32+2 + _granula;
				_outputOffset = 32+2;
			}

			size_t spaceAmount = _output._size - _outputOffset;
			if(spaceAmount > size)
			{
				memcpy(_output._data.get() + _outputOffset, data, size);
				_outputOffset += size;
				//size = 0;
				return true;
			}
			else
			{
				memcpy(_output._data.get() + _outputOffset, data, spaceAmount);
				_outputOffset += spaceAmount;
				size -= spaceAmount;
				if(!flush())
				{
					return false;
				}
			}
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////////
	bool ContentFilterEncodeChunked::flush(bool finish)
	{
		if(finish && !_output._data)
		{
			_output._data.reset(new char[3]);
			_output._size = 3;
			char *iter = _output._data.get();
			*iter++ = '0';
			*iter++ = '\r';
			*iter++ = '\n';
			if(!_upstream->filterPush(_output))
			{
				return false;
			}

			_output._data.reset();
			_output._size = 0;
			return true;
		}
		char *iter = _output._data.get() + _outputOffset;
		*iter++ = '\r';
		*iter++ = '\n';
		if(finish)
		{
			*iter++ = '0';
			*iter++ = '\r';
			*iter++ = '\n';
		}
		_output._size = iter - _output._data.get();

		size_t chunkSize = _outputOffset - (32+2);

		size_t headerSize = 2;
		if(chunkSize)
		{
			for(; chunkSize; chunkSize >>= 4)
			{
				headerSize++;
			}
		}
		else
		{
			headerSize++;
		}
		chunkSize = _outputOffset - (32+2);

		iter = _output._data.get() + 32+2 - headerSize;

		bool genResult = boost::spirit::karma::generate(iter, boost::spirit::karma::uint_generator<size_t, 16>()[boost::spirit::karma::_1 = chunkSize]<<"\r\n");
		assert(genResult);
		(void)genResult;

		assert(32+2 == iter - _output._data.get());

		if(!_upstream->filterPush(_output, 32+2 - headerSize))
		{
			return false;
		}

		_output._data.reset();
		_output._size = 0;
		_outputOffset = 0;
		return true;
	}
}}}
