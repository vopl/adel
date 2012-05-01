#include "pch.hpp"
#include "net/http/impl/message.hpp"
#include "net/http/impl/messageBuffer.hpp"

namespace net { namespace http { namespace impl
{
	///////////////////////////////////////////////////////
	Message::Message()
		: _size(0)
		, _lastBuffer(NULL)
	{

	}

	///////////////////////////////////////////////////////
	Message::~Message()
	{

	}

	///////////////////////////////////////////////////////
	MessageIterator Message::begin()
	{
		if(!_firstBuffer)
		{
			obtainMoreBuffers(true);
		}
		assert(_firstBuffer);
		assert(_lastBuffer);
		return MessageIterator(_firstBuffer, _firstBuffer->begin());
	}

	///////////////////////////////////////////////////////
	MessageIterator Message::end()
	{
		if(!_firstBuffer)
		{
			obtainMoreBuffers(true);
		}
		assert(_firstBuffer);
		assert(_lastBuffer);
		return MessageIterator(_lastBuffer->shared_from_this(), _lastBuffer->end());
	}

	///////////////////////////////////////////////////////
	MessageIterator Message::endInfinity()
	{
		if(!_firstBuffer)
		{
			obtainMoreBuffers(true);
		}
		return MessageIterator(_firstBuffer, NULL);
	}

	///////////////////////////////////////////////////////
	const MessageIterator::size_type &Message::size() const
	{
		return _size;
	}

	///////////////////////////////////////////////////////
	bool Message::obtainMoreBuffers(bool force)
	{
		return false;
	}

	///////////////////////////////////////////////////////
	void Message::dropTail(MessageIterator::size_type size)
	{
		assert(size > 0);
		assert(_lastBuffer);

		while(_lastBuffer->offset() >= size)
		{
			assert(_lastBuffer->_prev);
			_lastBuffer = _lastBuffer->_prev;
			_lastBuffer->_next.reset();
		}
		assert(_lastBuffer->offset() < size);
		assert(_lastBuffer->offset()+_lastBuffer->size() >= size);
		_lastBuffer->_end = _lastBuffer->_begin + (size - _lastBuffer->offset());
		return;
	}

	///////////////////////////////////////////////////////
	void Message::dropFront()
	{
		if(_firstBuffer->_next)
		{
			_firstBuffer->_next->_prev = NULL;
			_firstBuffer.swap(_firstBuffer->_next);
		}
		else
		{
			assert(_lastBuffer == _firstBuffer.get());
			_lastBuffer = NULL;
			_firstBuffer.reset();
		}
	}

	///////////////////////////////////////////////////////
	void Message::pushBuffer(const Packet &packet)
	{
		MessageBufferPtr buffer(new MessageBuffer(
			this,
			_size,
			packet._data,
			0,
			packet._size));

		if(!_firstBuffer)
		{
			_firstBuffer = buffer;
			_lastBuffer = _firstBuffer.get();
		}
		else
		{
			assert(_lastBuffer);
			_lastBuffer->_next = buffer;
			buffer->_prev = _lastBuffer;
			_lastBuffer = buffer.get();
		}
		_size += packet._size;
	}



}}}
