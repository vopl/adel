#include "pch.hpp"
#include "net/http/impl/messageOut.hpp"

namespace net { namespace http { namespace impl
{
	//////////////////////////////////////////////////////////////
	MessageOut::MessageOut(const Channel &channel, size_t bufferGranula)
		: _channel(channel)
		, _bufferGranula(bufferGranula)
		, _writePosition(NULL)
		, _writeEnd(NULL)
	{
	}

	//////////////////////////////////////////////////////////////
	MessageOut::~MessageOut()
	{
	}

	//////////////////////////////////////////////////////////////
	bool MessageOut::isConnected() const
	{
		return _channel.isOpen();
	}

	//////////////////////////////////////////////////////////////
	MessageOut::Iterator	MessageOut::firstLineIterator()
	{
		if(!ensureMode(em_firstLine))
		{
			return false;
		}
		return iterator();
	}

	//////////////////////////////////////////////////////////////
	bool		MessageOut::firstLine(const char *data, size_t size)
	{
		if(!ensureMode(em_firstLine))
		{
			return false;
		}
		return write(data, size);
	}

	//////////////////////////////////////////////////////////////
	bool		MessageOut::firstLine(const char *dataz)
	{
		if(!ensureMode(em_firstLine))
		{
			return false;
		}
		return write(dataz);
	}

	//////////////////////////////////////////////////////////////
	bool		MessageOut::firstLine(const std::string &data)
	{
		if(!ensureMode(em_firstLine))
		{
			return false;
		}
		return write(data);
	}

	//////////////////////////////////////////////////////////////
	bool		MessageOut::firstLineFlush()
	{
		return ensureMode(em_headers);
	}

	//////////////////////////////////////////////////////////////
	MessageOut::Iterator	MessageOut::headersIterator()
	{
		bool b = ensureMode(em_headers);
		assert(b);
		(void)b;
		return iterator();
	}

	//////////////////////////////////////////////////////////////
	bool		MessageOut::header(const char *data, size_t size)
	{
		if(!ensureMode(em_headers))
		{
			return false;
		}
		if(!write(data, size))
		{
			return false;
		}
		return write("\r\n", 2);
	}

	//////////////////////////////////////////////////////////////
	bool		MessageOut::header(const char *dataz)
	{
		if(!ensureMode(em_headers))
		{
			return false;
		}
		if(!write(dataz))
		{
			return false;
		}
		return write("\r\n", 2);
	}

	//////////////////////////////////////////////////////////////
	bool		MessageOut::header(const std::string &data)
	{
		if(!ensureMode(em_headers))
		{
			return false;
		}
		if(!write(data))
		{
			return false;
		}
		return write("\r\n", 2);
	}

	//////////////////////////////////////////////////////////////
	bool		MessageOut::header(const HeaderName &name, const std::string &value)
	{
		return header(name, value.data(), value.size());
	}

	//////////////////////////////////////////////////////////////
	bool		MessageOut::header(const HeaderName &name, const char *value, size_t valueSize)
	{
		if(!ensureMode(em_headers))
		{
			return false;
		}
		if(!write(name.str))
		{
			return false;
		}
		if(!write(": ", 2))
		{
			return false;
		}
		if(!write(value, valueSize))
		{
			return false;
		}
		if(!write("\r\n", 2))
		{
			return false;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////
	bool		MessageOut::header(const HeaderName &name, const char *valuez)
	{
		return header(name, valuez, strlen(valuez));
	}

	//////////////////////////////////////////////////////////////
	bool		MessageOut::headersFlush()
	{
		return ensureMode(em_body);
	}

	//////////////////////////////////////////////////////////////
	MessageOut::Iterator	MessageOut::bodyIterator()
	{
		bool b = ensureMode(em_body);
		assert(b);
		(void)b;
		return iterator();
	}

	//////////////////////////////////////////////////////////////
	bool		MessageOut::body(const char *data, size_t size)
	{
		if(!ensureMode(em_body))
		{
			return false;
		}
		return write(data, size);
	}

	//////////////////////////////////////////////////////////////
	bool		MessageOut::body(const char *dataz)
	{
		if(!ensureMode(em_body))
		{
			return false;
		}
		return write(dataz);
	}

	//////////////////////////////////////////////////////////////
	bool		MessageOut::body(const std::string &data)
	{
		if(!ensureMode(em_body))
		{
			return false;
		}
		return write(data);
	}

	//////////////////////////////////////////////////////////////
	bool		MessageOut::bodyFlush()
	{
		if(!ensureMode(em_body))
		{
			return false;
		}
		assert(!"flush");
		//_mode = em_firstLine;
		return true;
	}

	//////////////////////////////////////////////////////////////
	char *MessageOut::getBuffer(size_t &size)
	{
		assert(_writeEnd && _writePosition);

		size_t availSize = _writeEnd - _writePosition;
		if(availSize < size)
		{
			size = availSize;
		}
		return _writePosition;
	}
	//////////////////////////////////////////////////////////////
	void MessageOut::nextBuffer()
	{
		if(_buffer._size)
		{
			assert(0);
			//flush packet
			_buffer._data.reset();
			_buffer._size = 0;
			_writePosition = NULL;
			_writeEnd = NULL;
		}

		_buffer._data.reset(new char [_bufferGranula]);
		_buffer._size = _bufferGranula;

		_writePosition = _buffer._data.get();
		_writeEnd = _writePosition + _buffer._size;
	}

	//////////////////////////////////////////////////////////////
	void MessageOut::iteratorIncrement()
	{
		assert(_writePosition < _writeEnd);
		_writePosition++;
		if(_writePosition == _writeEnd)
		{
			nextBuffer();
		}
	}

	//////////////////////////////////////////////////////////////
	char &MessageOut::iteratorDereference()
	{
		return *_writePosition;
	}

	//////////////////////////////////////////////////////////////
	MessageOut::Iterator	MessageOut::iterator()
	{
		return Iterator(this);
	}

	//////////////////////////////////////////////////////////////
	bool MessageOut::write(const char *data, size_t size)
	{
		while(size)
		{
			size_t writeSize = size;
			char *buf = getBuffer(writeSize);
			assert(writeSize);
			memcpy(buf, data, writeSize);

			if(writeSize == size)
			{
				nextBuffer();
			}
			size -= writeSize;
		}
		return true;//isConnected()
	}

	//////////////////////////////////////////////////////////////
	bool MessageOut::write(const char *dataz)
	{
		return write(dataz, strlen(dataz));
	}

	//////////////////////////////////////////////////////////////
	bool MessageOut::write(const std::string &data)
	{
		return write(data.data(), data.size());
	}

	//////////////////////////////////////////////////////////////
	bool MessageOut::ensureMode(EMode em)
	{
		assert(_mode <= em);
		if(_mode == em)
		{
			return true;
		}

		switch(em)
		{
		case em_headers:
			{
				switch(_mode)
				{
				case em_firstLine:
					{
						if(!write("\r\n", 2))
						{
							return false;
						}
					}
					_mode = em;
					return true;
				default:
					assert(0);
					break;
				}
			}
			break;
		case em_body:
			{
				switch(_mode)
				{
				case em_firstLine:
					{
						if(!write("\r\n\r\n", 4))
						{
							return false;
						}
						_mode = em;
					}
					return true;
				case em_headers:
					{
						if(!write("\r\n", 2))
						{
							return false;
						}
						_mode = em;
					}
					return true;
				default:
					assert(0);
					break;
				}
			}
			break;
		default:
			assert(0);
			break;
		}

		assert(!"never here");
		return true;
	}

}}}
