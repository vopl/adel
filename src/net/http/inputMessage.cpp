#include "pch.hpp"
#include "net/http/inputMessage.hpp"
#include "net/http/impl/inputMessage.hpp"
#include "net/http/impl/inputMessageBuffer.hpp"

namespace net { namespace http
{
	//////////////////////////////////////////////////////////////////////////
	InputMessage::Iterator::Iterator(const Iterator &i)
		: _buffer(i._buffer)
		, _position(i._position)
	{
		assert(_buffer);
		assert(_position >= _buffer->begin());
		assert(_position < _buffer->end());
	}

	//////////////////////////////////////////////////////////////////////////
	InputMessage::Iterator::~Iterator()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	InputMessage::Iterator &InputMessage::Iterator::operator=(const Iterator &i)
	{
		_buffer = i._buffer;
		_position = i._position;

		assert(_buffer);
		assert(_position >= _buffer->begin());
		assert(_position < _buffer->end());

		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	InputMessage::Iterator::size_type InputMessage::Iterator::absolutePosition() const
	{
		assert(_buffer);
		assert(_position >= _buffer->begin());
		assert(_position < _buffer->end());

		return _buffer->offset() + (_position - _buffer->begin());
	}

	//////////////////////////////////////////////////////////////////////////
	InputMessage::Iterator::reference InputMessage::Iterator::dereference() const
	{
		assert(_buffer);
		assert(_position >= _buffer->begin());
		assert(_position < _buffer->end());

		return *_position;
	}

	//////////////////////////////////////////////////////////////////////////
	bool InputMessage::Iterator::equal(const Iterator &i) const
	{
		return _position == i._position;
	}

	//////////////////////////////////////////////////////////////////////////
	void InputMessage::Iterator::increment()
	{
		assert(_buffer);
		assert(_position >= _buffer->begin());
		assert(_position < _buffer->end());

		_position++;

		if(_buffer->end() == _position)
		{
			_buffer = _buffer->next();
			assert(_buffer);

			_position = _buffer->begin();
			assert(_position >= _buffer->begin());
			assert(_position < _buffer->end());
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void InputMessage::Iterator::decrement()
	{
		assert(_buffer);
		assert(_position >= _buffer->begin());
		assert(_position < _buffer->end());

		if(_buffer->begin() == _position)
		{
			_buffer = _buffer->prev();
			assert(_buffer);

			_position = _buffer->end()-1;
			assert(_position >= _buffer->begin());
			assert(_position < _buffer->end());
		}
		else
		{
			_position--;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void InputMessage::Iterator::advance(difference_type n)
	{
		assert(_buffer);
		assert(_position >= _buffer->begin());
		assert(_position < _buffer->end());

		while(n>0)
		{
			difference_type distInThisBuffer = _buffer->end() - _position;
			if(n < distInThisBuffer)
			{
				_position += n;
				return;
			}
			n -= distInThisBuffer;

			_buffer = _buffer->next();
			assert(_buffer);

			_position = _buffer->begin();
			assert(_position >= _buffer->begin());
			assert(_position < _buffer->end());
		}

		assert(0 >= n);
		while(n)
		{
			difference_type distInThisBuffer = _buffer->begin() - _position;
			if(n >= distInThisBuffer)
			{
				_position += n;
				return;
			}
			n -= distInThisBuffer;
			_buffer = _buffer->prev();
			assert(_buffer);

			_position = _buffer->end() - 1;
			assert(_position >= _buffer->begin());
			assert(_position < _buffer->end());
		}
	}

	//////////////////////////////////////////////////////////////////////////
	InputMessage::Iterator::difference_type InputMessage::Iterator::distance_to(const Iterator &i) const
	{
		return (difference_type)i.absolutePosition() - (difference_type)absolutePosition();
	}

	//////////////////////////////////////////////////////////////////////////
	InputMessage::Iterator::Iterator()
		: _buffer(NULL)
		, _position(NULL)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	InputMessage::Iterator::Iterator(impl::InputMessageBuffer *buffer, const char *position)
		: _buffer(buffer)
		, _position(position)
	{
		assert(_buffer);
		assert(_position >= _buffer->begin());
		assert(_position < _buffer->end());
	}










	//////////////////////////////////////////////////////////////////////////
	InputMessage::InputMessage()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	InputMessage::~InputMessage()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	bool InputMessage::isConnected() const
	{
		return _impl->isConnected();
	}

	//////////////////////////////////////////////////////////////////////////
	bool InputMessage::readRequestLine()
	{
		return _impl->readRequestLine();
	}

	//////////////////////////////////////////////////////////////////////////
	bool InputMessage::readHeaders()
	{
		return _impl->readHeaders();
	}

	//////////////////////////////////////////////////////////////////////////
	bool InputMessage::readBody()
	{
		return _impl->readBody();
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment &InputMessage::firstLine() const
	{
		return _impl->firstLine();
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment &InputMessage::headers() const
	{
		return _impl->headers();
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment *InputMessage::header(const HeaderName &name) const
	{
		return _impl->header(name);
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment *InputMessage::header(size_t key) const
	{
		return _impl->header(key);
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment *InputMessage::header(const std::string &name) const
	{
		return _impl->header(name);
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment *InputMessage::header(const char *name, size_t nameSize) const
	{
		return _impl->header(name, nameSize);
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment *InputMessage::header(const char *namez) const
	{
		return _impl->header(namez);
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment &InputMessage::body() const
	{
		return _impl->body();
	}


}}
