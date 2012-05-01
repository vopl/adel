#include "pch.hpp"
#include "net/http/messageIterator.hpp"
#include "net/http/impl/messageBuffer.hpp"
#include "net/http/impl/message.hpp"

namespace net { namespace http
{

	///////////////////////////////////////////////////////////////
	MessageIterator::MessageIterator()
		: _buffer()
		, _position(NULL)
	{
	}

	///////////////////////////////////////////////////////////////
	MessageIterator::MessageIterator(const MessageIterator &i)
		: _buffer(i._buffer)
		, _position(i._position)
	{
		if(_buffer)
		{
			assert(_buffer->begin() <= _position || !_position);
			assert(_buffer->end() >= _position || !_position);
			_buffer->incIteratorUseCount();
		}
	}

	///////////////////////////////////////////////////////////////
	MessageIterator::~MessageIterator()
	{
		if(_buffer)
		{
			_buffer->decIteratorUseCount();
		}
	}

	///////////////////////////////////////////////////////////////
	MessageIterator::reference MessageIterator::dereference() const
	{
		assert(_buffer);
		assert(_position);

		assert(_buffer->begin() <= _position);
		assert(_buffer->end() > _position);

		return *_position;
	}

	///////////////////////////////////////////////////////////////
	bool MessageIterator::equal(const MessageIterator &i) const
	{
		if(_position == i._position)
		{
			assert(_buffer == i._buffer || !_buffer || !i._buffer);
			return true;
		}

		if(!_position || !_buffer)
		{
			if(!i._position || !i._buffer)
			{
				return true;
			}
			else
			{
				if(i._buffer->end() <= i._position)
				{
					assert(i._buffer->end() == i._position);
					return true;
				}
				return false;
			}
		}
		else
		{
			if(!i._position ||  !i._buffer)
			{
				if(_buffer->end() <= _position)
				{
					assert(_buffer->end() == _position);
					return true;
				}
				return false;
			}
			else
			{
				assert(_buffer->message() == i._buffer->message());
				assert(_position != i._position);
				return false;
			}
		}

		assert(!"never here");
		return false;
	}

	///////////////////////////////////////////////////////////////
	void MessageIterator::increment()
	{
		assert(_buffer);
		assert(_position);

		assert(_buffer->begin() <= _position);
		assert(_buffer->end() > _position);

		_position++;

		if(_buffer->end() <= _position)
		{
			size_t extra = _position - _buffer->end();
			if(_buffer->end() < _position || _buffer->hasNext())
			{
				impl::MessageBufferPtr buffer = _buffer->next();
				if(buffer)
				{
					_buffer->decIteratorUseCount();
					_buffer = buffer;
					_buffer->incIteratorUseCount();

					assert(_buffer->size());
					_position = _buffer->begin() + extra;
				}
				else
				{
					//ok, its end
				}
			}
			else
			{
				//ok, its end
			}
		}
	}

	///////////////////////////////////////////////////////////////
	void MessageIterator::decrement()
	{
		assert(_buffer);
		assert(_position);

		assert(_buffer->begin() <= _position);
		assert(_buffer->end() >= _position);
		if(_buffer->begin() == _position)
		{
			_buffer->decIteratorUseCount();
			_buffer = _buffer->prev();
			assert(_buffer);
			assert(_buffer->size());
			_buffer->incIteratorUseCount();

			_position = _buffer->end() - 1;
		}
		else
		{
			_position--;
		}
	}

	///////////////////////////////////////////////////////////////
	void MessageIterator::advance(difference_type n)
	{
		assert(_buffer);
		assert(_position);

		while(n>0)
		{
			difference_type distInThisBuffer = _buffer->end() - _position;
			if(n < distInThisBuffer)
			{
				_position += n;
				return;
			}
			n -= distInThisBuffer;

			if(_buffer->hasNext() || n)
			{
				_buffer->decIteratorUseCount();
				_buffer = _buffer->next();
				_buffer->incIteratorUseCount();
				assert(_buffer);
				assert(_buffer->size());
				_position = _buffer->begin();
			}
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
			_buffer->decIteratorUseCount();
			_buffer = _buffer->prev();
			_buffer->incIteratorUseCount();
			assert(_buffer);
			assert(_buffer->size());
			_position = _buffer->end() - 1;
		}
	}

	///////////////////////////////////////////////////////////////
	MessageIterator::difference_type MessageIterator::distance_to(const MessageIterator &i) const
	{
		return (difference_type)i.absolutePosition() - (difference_type)absolutePosition();
	}

	///////////////////////////////////////////////////////////////
	impl::MessageBufferPtr	MessageIterator::buffer()
	{
		return _buffer;
	}

	///////////////////////////////////////////////////////////////
	char *MessageIterator::rawBufferFwd(size_type &size)
	{
		assert(_buffer);
		assert(_position);

		assert(_buffer->end() >= _position);
		size_t bufSize = _buffer->end() - _position;

		if(bufSize < size)
		{
			size = bufSize;
		}

		return _position;
	}

	///////////////////////////////////////////////////////////////
	char *MessageIterator::rawBufferBwd(size_type &size)
	{
		assert(0);
		return NULL;
	}


	///////////////////////////////////////////////////////////////
	MessageIterator::size_type MessageIterator::absolutePosition() const
	{
		if(!_position)
		{
			assert(_buffer);
			return _buffer->message()->size();
		}
		assert(_buffer);

		return _buffer->offset() + (_position - _buffer->begin());
	}

	///////////////////////////////////////////////////////////////
	bool MessageIterator::isEndInfinity() const
	{
		return _position?false:true;
	}

	///////////////////////////////////////////////////////////////
	MessageIterator::MessageIterator(impl::MessageBufferPtr buffer, char *position)
		: _buffer(buffer)
		, _position(position)
	{
		assert(_buffer);
		assert(_buffer->size());

		_buffer->incIteratorUseCount();

		assert(!_position || _position >= _buffer->begin());
		assert(!_position || _position <= _buffer->end());
	}

}}
