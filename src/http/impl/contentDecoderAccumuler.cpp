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
		if(_last)
		{
			InputMessageBufferPtr nullb;
			for(;;)
			{
				_last = _last->prev();
				if(_last)
				{
					_last->setNext(nullb);
				}
				else
				{
					break;
				}
			}
		}
		//_first.reset();
	}

	////////////////////////////////////////////////////////////////
	boost::system::error_code ContentDecoderAccumuler::push(const net::Packet &packet, size_t offset)
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
	boost::system::error_code ContentDecoderAccumuler::flush()
	{
		return http::error::make();
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
		if(buf)
		{
			return http::InputMessage::Iterator(buf, buf->begin());
		}

		return http::InputMessage::Iterator();
	}

	////////////////////////////////////////////////////////////////
	http::InputMessage::Iterator ContentDecoderAccumuler::end()
	{
		InputMessageBuffer	*buf = lastBuffer();
		return http::InputMessage::Iterator(buf, buf->end());
	}

	////////////////////////////////////////////////////////////////
	size_t ContentDecoderAccumuler::size()
	{
		return _size;
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

	////////////////////////////////////////////////////////////////
	void ContentDecoderAccumuler::dropTail(const http::InputMessage::Iterator &pos)
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

		boundBuffer->setNext(InputMessageBufferPtr());
		boundBuffer->setEnd(boundPosition);

		_size = boundBuffer->offset() + boundBuffer->size();

		if(!boundBuffer->size())
		{
			if(boundBuffer->prev())
			{
				boundBuffer = boundBuffer->prev();
				boundBuffer->setNext(InputMessageBufferPtr());
			}
			else
			{
				assert(_first.get() == boundBuffer);
				_first.reset();
				_last = NULL;
			}
		}
	}

}}
