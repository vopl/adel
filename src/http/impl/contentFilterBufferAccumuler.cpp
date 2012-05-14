#include "pch.hpp"
#include "http/impl/contentFilterBufferAccumuler.hpp"

namespace http { namespace impl
{
	////////////////////////////////////////////////////////////////
	ContentFilterBufferAccumuler::ContentFilterBufferAccumuler()
		: _first()
		, _last(NULL)
		, _size(0)
	{

	}

	////////////////////////////////////////////////////////////////
	ContentFilterBufferAccumuler::~ContentFilterBufferAccumuler()
	{
	}

	////////////////////////////////////////////////////////////////
	bool ContentFilterBufferAccumuler::filterPush(const net::Packet &packet, size_t offset)
	{
		assert(packet._size);
		assert(packet._size>offset);

		InputMessageBufferPtr buf(new InputMessageBuffer(_size, packet._data, offset, packet._size));

		if(_last)
		{
			assert(_first);

			buf->setPrev(_last);
			_last->setNext(buf);
			_last = buf.get();
		}
		else
		{
			assert(!_first);

			_first = buf;
			_last = _first.get();
		}

		_size += packet._size - offset;

		return true;
	}

	////////////////////////////////////////////////////////////////
	bool ContentFilterBufferAccumuler::filterFlush()
	{
		return true;
	}

	////////////////////////////////////////////////////////////////
	InputMessageBuffer	*ContentFilterBufferAccumuler::firstBuffer()
	{
		return _first.get();
	}

	////////////////////////////////////////////////////////////////
	InputMessageBuffer	*ContentFilterBufferAccumuler::lastBuffer()
	{
		return _last;
	}

	////////////////////////////////////////////////////////////////
	http::InputMessage::Iterator ContentFilterBufferAccumuler::begin()
	{
		InputMessageBuffer	*buf = firstBuffer();
		return http::InputMessage::Iterator(buf, buf->begin());
	}

	////////////////////////////////////////////////////////////////
	http::InputMessage::Iterator ContentFilterBufferAccumuler::end()
	{
		InputMessageBuffer	*buf = lastBuffer();
		return http::InputMessage::Iterator(buf, buf->end());
	}

}}
