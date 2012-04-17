#include "pch.hpp"
#include "net/message.hpp"

namespace net
{
	static const Message::Iterator::difference_type badOffset = -1;

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
		assert(badOffset != _chunkIndex);
		assert(badOffset != _offsetInChunk);
		return _message->_chunks[_chunkIndex]._packet._data[_offsetInChunk];
	}

	///////////////////////////////////////////////////////////////
	bool Message::Iterator::equal(const Iterator &i) const
	{
		assert(_message == i._message);
		if(badOffset == _chunkIndex)
		{
			if(badOffset == i._chunkIndex)
			{
				return true;
			}
			else
			{
				if(_message->_chunks.size() >= i._chunkIndex)
				{
					return false;
				}

				return true;
			}
		}
		else
		{
			if(badOffset == i._chunkIndex)
			{
				if(_message->_chunks.size() >= _chunkIndex)
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
		assert(badOffset != _chunkIndex);
		assert(_message->_chunks.size() > _chunkIndex);
		assert(_message->_chunks[_chunkIndex]._packet._size > _offsetInChunk);

		_offsetInChunk++;

		Message::SChunk &chunk = _message->_chunks[_chunkIndex];
		if(chunk._packet._size <= _offsetInChunk)
		{
			_chunkIndex++;
			_offsetInChunk = 0;

			if(_message->_chunks.size() <= _chunkIndex)
			{
				_message->obtainMoreChunks();
			}
		}
	}

	///////////////////////////////////////////////////////////////
	void Message::Iterator::decrement()
	{
		assert(badOffset != _chunkIndex);
		assert(_message->_chunks.size() > _chunkIndex);
		assert(_message->_chunks[_chunkIndex]._packet._size > _offsetInChunk);

		_offsetInChunk--;

		if(0 > _offsetInChunk)
		{
			_chunkIndex--;
			assert(0 <= _chunkIndex);
			Message::SChunk &chunk = _message->_chunks[_chunkIndex];

			assert(0 < chunk._packet._size);
			_offsetInChunk = chunk._packet._size-1;
		}
	}

	///////////////////////////////////////////////////////////////
	void Message::Iterator::advance(difference_type n)
	{
		assert(badOffset != _chunkIndex);
		
		if(0 < n)
		{
			//вперед
			while(n)
			{
				assert(0 <= _chunkIndex);
				assert(_message->_chunks.size() > _chunkIndex);
				Message::SChunk &chunk = _message->_chunks[_chunkIndex];

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
					
					assert(_message->_chunks.size() >= _chunkIndex);
					if(_message->_chunks.size() == _chunkIndex)
					{
						if(!_message->obtainMoreChunks())
						{
							assert(!n);
						}
					}
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
				Message::SChunk &chunk = _message->_chunks[_chunkIndex];

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
					Message::SChunk &chunk = _message->_chunks[_chunkIndex];
					_offsetInChunk = chunk._packet._size-1;
				}
			}
		}
	}

	///////////////////////////////////////////////////////////////
	Message::Iterator::difference_type Message::Iterator::distance_to(const Iterator &i) const
	{
		return i.absolutePosition() - absolutePosition();
	}

	///////////////////////////////////////////////////////////////
	Message::Iterator::difference_type Message::Iterator::absolutePosition() const
	{
		if(badOffset == _chunkIndex)
		{
			return _message->_size;
		}
		
		assert(_message->_chunks.size() >= _chunkIndex);
		if(_message->_chunks.size() == _chunkIndex)
		{
			assert(0 == _offsetInChunk);
			return _message->_size;
		}
		
		Message::SChunk &chunk = _message->_chunks[_chunkIndex];
		return chunk._offset + _offsetInChunk;
	}

	///////////////////////////////////////////////////////////////
	Message::Iterator::Iterator(Message *message, difference_type chunkIndex, difference_type offsetInChunk)
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
		return Iterator(this, 0, 0);
	}

	///////////////////////////////////////////////////////////////
	Message::Iterator Message::end()
	{
		return Iterator(this, _chunks.size(), 0);
	}

	///////////////////////////////////////////////////////////////
	Message::Iterator Message::endInfinity()
	{
		return Iterator(this, badOffset, badOffset);
	}

	///////////////////////////////////////////////////////////////
	bool Message::obtainMoreChunks()
	{
		assert(0);
		return false;
	}


}
