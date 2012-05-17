#include "pch.hpp"
#include "http/impl/contentDecoderAccumuler.hpp"
#include "http/error.hpp"

namespace http { namespace impl
{
	////////////////////////////////////////////////////////////////
	ContentDecoderAccumuler::ContentDecoderAccumuler()
		: _first()
		, _last(NULL)
		, _size(0)
	{

	}

	////////////////////////////////////////////////////////////////
	ContentDecoderAccumuler::~ContentDecoderAccumuler()
	{
	}

	////////////////////////////////////////////////////////////////
	boost::system::error_code ContentDecoderAccumuler::decoderPush(const net::Packet &packet, size_t offset)
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

		return http::error::make();
	}

	////////////////////////////////////////////////////////////////
	boost::system::error_code ContentDecoderAccumuler::decoderFlush()
	{
		return error::make();
	}

	////////////////////////////////////////////////////////////////
	InputMessageBuffer	*ContentDecoderAccumuler::firstBuffer()
	{
		return _first.get();
	}

	////////////////////////////////////////////////////////////////
	InputMessageBuffer	*ContentDecoderAccumuler::lastBuffer()
	{
		return _last;
	}

	////////////////////////////////////////////////////////////////
	http::InputMessage::Iterator ContentDecoderAccumuler::begin()
	{
		InputMessageBuffer	*buf = firstBuffer();
		return http::InputMessage::Iterator(buf, buf->begin());
	}

	////////////////////////////////////////////////////////////////
	http::InputMessage::Iterator ContentDecoderAccumuler::end()
	{
		InputMessageBuffer	*buf = lastBuffer();
		return http::InputMessage::Iterator(buf, buf->end());
	}

	////////////////////////////////////////////////////////////////
	void ContentDecoderAccumuler::dropFront(const http::InputMessage::Iterator &pos)
	{
		if(!_first)
		{
			assert(0);
			return;
		}

		InputMessageBuffer *boundBuffer = pos.buffer();
		const char *boundPosition = pos.position();
		assert(boundBuffer);
		assert(boundPosition);

		while(boundBuffer != _first.get())
		{
			assert(_first);
			_size -= _first->size();
			if(_first->next())
			{
				_first->next()->setPrev(NULL);
				_first = _first->next()->shared_from_this();
			}
			else
			{
				_first.reset();
				_last = NULL;
			}
		}
		assert(_first.get() == boundBuffer);
		assert(_first->begin() <= boundPosition);
		assert(_first->end() >= boundPosition);

		if(_first->end() == boundPosition)
		{
			_size -= _first->size();
			if(_first->next())
			{
				_first->next()->setPrev(NULL);
				_first = _first->next()->shared_from_this();
			}
			else
			{
				_first.reset();
				_last = NULL;
			}
		}
		else
		{
			_size -= boundPosition - _first->begin();
			_first->setBegin(boundPosition);
		}

		InputMessageBuffer *buffer = _first.get();
		size_t offset=0;
		while(buffer)
		{
			buffer->setOffset(offset);
			offset += buffer->size();
			buffer = buffer->next();
		}

		assert(offset == _size);
	}

}}
