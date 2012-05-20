#include "pch.hpp"
#include "http/impl/inputMessageBuffer.hpp"
#include "http/impl/inputMessage.hpp"

namespace http { namespace impl
{
	////////////////////////////////////////////////////////////////
	InputMessageBuffer::InputMessageBuffer(
		size_t offset,
		boost::shared_array<char> data,
		size_t dataBegin,
		size_t dataEnd)

		: _offset(offset)
		, _next()
		, _prev(NULL)
		, _data(data)
		, _begin(data.get()+dataBegin)
		, _end(data.get()+dataEnd)
	{
	}

	////////////////////////////////////////////////////////////////
	InputMessageBuffer::~InputMessageBuffer()
	{
	}

	////////////////////////////////////////////////////////////////
	const char *InputMessageBuffer::begin()
	{
		return _begin;
	}

	////////////////////////////////////////////////////////////////
	const char *InputMessageBuffer::end()
	{
		return _end;
	}

	////////////////////////////////////////////////////////////////
	InputMessageBuffer *InputMessageBuffer::next()
	{
		//assert(_next);
		return _next.get();
	}

	////////////////////////////////////////////////////////////////
	InputMessageBuffer *InputMessageBuffer::prev()
	{
		assert(_prev);
		return _prev;
	}

	////////////////////////////////////////////////////////////////
	void InputMessageBuffer::setNext(const InputMessageBufferPtr &b)
	{
		_next = b;
	}

	////////////////////////////////////////////////////////////////
	void InputMessageBuffer::setPrev(InputMessageBuffer *b)
	{
		_prev = b;
	}

	////////////////////////////////////////////////////////////////
	void InputMessageBuffer::setBegin(const char *pos)
	{
		assert(_data);
		assert(_data.get() <= pos);
		_begin = pos;
	}

	////////////////////////////////////////////////////////////////
	void InputMessageBuffer::setEnd(const char *pos)
	{
		assert(_data);
		assert(_data.get() <= pos);
		_end = pos;
	}

	////////////////////////////////////////////////////////////////
	void InputMessageBuffer::setOffset(size_t offset)
	{
		_offset = offset;
	}

	////////////////////////////////////////////////////////////////
	net::Packet InputMessageBuffer::asPacket(size_t &offset)
	{
		net::Packet res(_data, _end - _data.get());
		offset = _begin - _data.get();

		return res;
	}

	////////////////////////////////////////////////////////////////
	size_t InputMessageBuffer::size() const
	{
		return _end - _begin;
	}

	////////////////////////////////////////////////////////////////
	size_t InputMessageBuffer::offset() const
	{
		return _offset;
	}
}}
