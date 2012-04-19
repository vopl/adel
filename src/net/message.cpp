#include "pch.hpp"
#include "net/message.hpp"
#include "net/impl/message.hpp"

namespace net
{
	///////////////////////////////////////////////////////////////
	Message::Iterator::Iterator()
		: _message()
		, _chunkIndex(_badOffset)
		, _offsetInChunk(_badOffset)
	{
	}

	///////////////////////////////////////////////////////////////
	Message::Iterator::Iterator(const Iterator &i)
		: _message(i._message)
		, _chunkIndex(i._chunkIndex)
		, _offsetInChunk(i._offsetInChunk)
	{
	}

	///////////////////////////////////////////////////////////////
	Message::Iterator::~Iterator()
	{
	}

	///////////////////////////////////////////////////////////////
	Message::Iterator::reference Message::Iterator::dereference() const
	{
		assert(_badOffset != _chunkIndex);
		assert(_badOffset != _offsetInChunk);

		while(_message->chunks().size() <= _chunkIndex)
		{
			_message->obtainMoreChunks();
		}
		assert(_message->chunks()[_chunkIndex]._packet._size > _offsetInChunk);
		return _message->chunks()[_chunkIndex]._packet._data[_offsetInChunk];
	}

	///////////////////////////////////////////////////////////////
	bool Message::Iterator::equal(const Iterator &i) const
	{
		assert(_message == i._message || !_message || !i._message);
		if(_badOffset == _chunkIndex)
		{
			if(_badOffset == i._chunkIndex)
			{
				return true;
			}
			else
			{
				if(_message->chunks().size() > i._chunkIndex)
				{
					return false;
				}

				return true;
			}
		}
		else
		{
			if(_badOffset == i._chunkIndex)
			{
				if(_message->chunks().size() > _chunkIndex)
				{
					return false;
				}

				return true;
			}
			else
			{
				return _chunkIndex == i._chunkIndex && _offsetInChunk == i._offsetInChunk;
			}
		}

		assert(!"never here");
		return false;
	}

	///////////////////////////////////////////////////////////////
	void Message::Iterator::increment()
	{
		assert(_badOffset != _chunkIndex);
		assert(_message->chunks().size() > _chunkIndex);
		assert(_message->chunks()[_chunkIndex]._packet._size > _offsetInChunk);

		_offsetInChunk++;

		const impl::Message::SChunk &chunk = _message->chunks()[_chunkIndex];
		if(chunk._packet._size <= _offsetInChunk)
		{
			_chunkIndex++;
			_offsetInChunk = 0;
		}
	}

	///////////////////////////////////////////////////////////////
	void Message::Iterator::decrement()
	{
		assert(_badOffset != _chunkIndex);
		assert(_message->chunks().size() > _chunkIndex);
		assert(_message->chunks()[_chunkIndex]._packet._size > _offsetInChunk);

		_offsetInChunk--;

		if(0 > _offsetInChunk)
		{
			_chunkIndex--;
			assert(0 <= _chunkIndex);
			impl::Message::SChunk &chunk = _message->chunks()[_chunkIndex];

			assert(0 < chunk._packet._size);
			_offsetInChunk = chunk._packet._size-1;
		}
	}

	///////////////////////////////////////////////////////////////
	void Message::Iterator::advance(difference_type n)
	{
		assert(_badOffset != _chunkIndex);
		
		if(0 < n)
		{
			//вперед
			while(n)
			{
				assert(0 <= _chunkIndex);
				assert(_message->chunks().size() > _chunkIndex);
				impl::Message::SChunk &chunk = _message->chunks()[_chunkIndex];

				assert(0 < chunk._packet._size);
				difference_type delta = chunk._packet._size - _offsetInChunk;
				if(delta > n)
				{
					_offsetInChunk += n;
					n = 0;
				}
				else
				{
					n -= delta;
					_chunkIndex++;
					_offsetInChunk = 0;
					
					assert(_message->chunks().size() >= _chunkIndex);
/*
					if(_message->chunks().size() == _chunkIndex)
					{
						if(!_message->obtainMoreChunks())
						{
							assert(!n);
						}
					}
*/
				}
			}
		}
		else
		{
			//назад
			n = -n;
			while(n)
			{
				assert(0 <= _chunkIndex);
				impl::Message::SChunk &chunk = _message->chunks()[_chunkIndex];

				assert(0 < chunk._packet._size);
				difference_type delta = _offsetInChunk+1;
				if(delta > n)
				{
					_offsetInChunk -= n;
					n = 0;
				}
				else
				{
					n -= delta;
					_chunkIndex--;
					assert(0 <= _chunkIndex);
					impl::Message::SChunk &chunk = _message->chunks()[_chunkIndex];
					_offsetInChunk = chunk._packet._size-1;
				}
			}
		}
	}

	///////////////////////////////////////////////////////////////
	Message::Iterator::difference_type Message::Iterator::distance_to(const Iterator &i) const
	{
		assert(_message == i._message || !_message || !i._message);
		return i.absolutePosition() - absolutePosition();
	}

	///////////////////////////////////////////////////////////////
	Message::Iterator::difference_type Message::Iterator::absolutePosition() const
	{
		if(_badOffset == _chunkIndex)
		{
			return _message->size();
		}
		
		assert(_message->chunks().size() >= _chunkIndex);
		if(_message->chunks().size() == _chunkIndex)
		{
			assert(0 == _offsetInChunk);
			return _message->size();
		}
		
		impl::Message::SChunk &chunk = _message->chunks()[_chunkIndex];
		return chunk._offset + _offsetInChunk;
	}

	///////////////////////////////////////////////////////////////
	Message::Iterator::Iterator(impl::MessagePtr message, difference_type chunkIndex, difference_type offsetInChunk)
		: _message(message)
		, _chunkIndex(chunkIndex)
		, _offsetInChunk(offsetInChunk)
	{
	}


	///////////////////////////////////////////////////////////////
	Message::Message()
	{
	}

	///////////////////////////////////////////////////////////////
	Message::~Message()
	{
	}

	///////////////////////////////////////////////////////////////
	Message::Iterator Message::begin()
	{
		return _impl->begin();
	}

	///////////////////////////////////////////////////////////////
	Message::Iterator Message::end()
	{
		return _impl->end();
	}

	///////////////////////////////////////////////////////////////
	Message::Iterator Message::endInfinity()
	{
		return _impl->endInfinity();
	}
}
