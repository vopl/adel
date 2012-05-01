#include "pch.hpp"
#include "net/http/impl/messageBuffer.hpp"
#include "net/http/impl/message.hpp"

namespace net { namespace http { namespace impl
{
	////////////////////////////////////////////////////////////////
	MessageBuffer::MessageBuffer(
		Message *message,
		size_t offset,
		boost::shared_array<char> data,
		size_t dataBegin,
		size_t dataEnd)

		: _message(message)
		, _offset(offset)
		, _next()
		, _prev(NULL)
		, _data(data)
		, _begin(data.get()+dataBegin)
		, _end(data.get()+dataEnd)
		, _iteratorUseCount(0)
	{
	}

	////////////////////////////////////////////////////////////////
	MessageBuffer::~MessageBuffer()
	{
	}

	////////////////////////////////////////////////////////////////
	Message *MessageBuffer::message() const
	{
		return _message;
	}

	////////////////////////////////////////////////////////////////
	char *MessageBuffer::begin()
	{
		return _begin;
	}

	////////////////////////////////////////////////////////////////
	char *MessageBuffer::end()
	{
		return _end;
	}

	////////////////////////////////////////////////////////////////
	bool MessageBuffer::hasNext()
	{
		if(!_next)
		{
			bool b = _message->obtainMoreBuffers(false);
			(void)b;
		}
		return _next?true:false;
	}

	////////////////////////////////////////////////////////////////
	MessageBufferPtr MessageBuffer::next()
	{
		if(!_next)
		{
			bool b = _message->obtainMoreBuffers(true);
			(void)b;
		}
		return _next;
	}

	////////////////////////////////////////////////////////////////
	bool MessageBuffer::hasPrev()
	{
		return _prev?true:false;
	}

	////////////////////////////////////////////////////////////////
	MessageBufferPtr MessageBuffer::prev()
	{
		assert(_prev);
		return _prev->shared_from_this();
	}

	////////////////////////////////////////////////////////////////
	size_t MessageBuffer::size() const
	{
		return _end - _begin;
	}

	////////////////////////////////////////////////////////////////
	size_t MessageBuffer::offset() const
	{
		return _offset;
	}

	////////////////////////////////////////////////////////////////
	Packet MessageBuffer::asPacket(size_t &offsetInPacket)
	{
		Packet result(_data, _end - _data.get());
		offsetInPacket = _begin - _data.get();
		return result;
	}

	////////////////////////////////////////////////////////////////
	size_t MessageBuffer::iteratorUseCount()
	{
		return _iteratorUseCount;
	}

	////////////////////////////////////////////////////////////////
	void MessageBuffer::incIteratorUseCount()
	{
		_iteratorUseCount++;
	}

	////////////////////////////////////////////////////////////////
	void MessageBuffer::decIteratorUseCount()
	{
		_iteratorUseCount--;
	}


}}}
