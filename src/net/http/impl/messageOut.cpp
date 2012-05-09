#include "pch.hpp"
#include "net/http/impl/messageOut.hpp"
#include "net/http/impl/contentFilterChannelWriter.hpp"

namespace net { namespace http { namespace impl
{
	//////////////////////////////////////////////////////////////
	MessageOut::MessageOut(const Channel &channel, size_t bufferGranula)
		: _channel(channel)
		, _bufferGranula(bufferGranula)
		, _writePosition(NULL)
		, _writeEnd(NULL)
		, _contentFilter(new ContentFilterChannelWriter(channel, bufferGranula))
	{
		bool b = nextBuffer();
		assert(b);
		(void)b;
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
	bool MessageOut::firstLine(const char *data, size_t size)
	{
		if(!ensureMode(em_firstLine))
		{
			return false;
		}
		return write(data, size);
	}

	//////////////////////////////////////////////////////////////
	bool MessageOut::firstLine(const char *dataz)
	{
		if(!ensureMode(em_firstLine))
		{
			return false;
		}
		return write(dataz);
	}

	//////////////////////////////////////////////////////////////
	bool MessageOut::firstLine(const std::string &data)
	{
		if(!ensureMode(em_firstLine))
		{
			return false;
		}
		return write(data);
	}

	//////////////////////////////////////////////////////////////
	bool MessageOut::firstLineFlush()
	{
		return ensureMode(em_headers);
	}

	//////////////////////////////////////////////////////////////
	MessageOut::Iterator MessageOut::headersIterator()
	{
		bool b = ensureMode(em_headers);
		assert(b);
		(void)b;
		return iterator();
	}

	//////////////////////////////////////////////////////////////
	bool MessageOut::header(const char *data, size_t size)
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
	bool MessageOut::header(const char *dataz)
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
	bool MessageOut::header(const std::string &data)
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
	bool MessageOut::header(const HeaderName &name, const std::string &value)
	{
		return header(name, value.data(), value.size());
	}

	//////////////////////////////////////////////////////////////
	bool MessageOut::header(const HeaderName &name, const char *value, size_t valueSize)
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
	bool MessageOut::header(const HeaderName &name, const char *valuez)
	{
		return header(name, valuez, strlen(valuez));
	}

	//////////////////////////////////////////////////////////////
	bool MessageOut::headersFlush()
	{
		return ensureMode(em_body);
	}

	//////////////////////////////////////////////////////////////
	MessageOut::Iterator MessageOut::bodyIterator()
	{
		bool b = ensureMode(em_body);
		assert(b);
		(void)b;
		return iterator();
	}

	//////////////////////////////////////////////////////////////
	bool MessageOut::body(const char *data, size_t size)
	{
		if(!ensureMode(em_body))
		{
			return false;
		}
		return write(data, size);
	}

	//////////////////////////////////////////////////////////////
	bool MessageOut::body(const char *dataz)
	{
		if(!ensureMode(em_body))
		{
			return false;
		}
		return write(dataz);
	}

	//////////////////////////////////////////////////////////////
	bool MessageOut::body(const std::string &data)
	{
		if(!ensureMode(em_body))
		{
			return false;
		}
		return write(data);
	}

	//////////////////////////////////////////////////////////////
	bool MessageOut::bodyFlush()
	{
		if(!ensureMode(em_body))
		{
			return false;
		}
		if(!_contentFilter->filterFlush())
		{
			return false;
		}
		_mode = em_firstLine;
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
	bool MessageOut::nextBuffer()
	{
		if(_buffer._size && _buffer._data)
		{
			if(!_contentFilter->filterPush(_buffer, 0))
			{
				return false;
			}

			_buffer._data.reset();
			_buffer._size = 0;
			_writePosition = NULL;
			_writeEnd = NULL;
		}

		_buffer._data.reset(new char [_bufferGranula]);
		_buffer._size = _bufferGranula;

		_writePosition = _buffer._data.get();
		_writeEnd = _writePosition + _buffer._size;
		return true;
	}

	//////////////////////////////////////////////////////////////
	bool MessageOut::write(const char *data, size_t size)
	{
		while(size)
		{
			size_t writeSize = size;
			char *buf = getBuffer(writeSize);

			assert(writeSize && writeSize <= size);
			memcpy(buf, data, writeSize);

			if(writeSize == size)
			{
				if(!nextBuffer())
				{
					return false;
				}
			}
			size -= writeSize;
		}
		return true;
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
	MessageOut::Iterator MessageOut::iterator()
	{
		return Iterator(this);
	}

	//////////////////////////////////////////////////////////////
	bool MessageOut::writeSystemHeaders()
	{
		return true;
	}

	//////////////////////////////////////////////////////////////
	bool MessageOut::setupBodyFilters()
	{
		return true;
	}

	//////////////////////////////////////////////////////////////
	bool MessageOut::ensureMode(EMode em)
	{
		assert(_mode <= em);
		if(_mode == em)
		{
			return true;
		}

		switch(_mode)
		{
		case em_firstLine:
			{
				switch(em)
				{
				case em_headers:
					//terminate first line
					if(!write("\r\n", 2))
					{
						return false;
					}

					//now headers
					_mode = em_headers;

					return true;
				case em_body:
					//terminate first line
					if(!write("\r\n", 2))
					{
						return false;
					}

					//now headers
					_mode = em_headers;

					//write common headers(server, date, content-encoding, connection, ...)
					if(!this->writeSystemHeaders())
					{
						return false;
					}

					//terminate headers
					if(!write("\r\n", 2))
					{
						return false;
					}

					//setup body filters
					if(!this->setupBodyFilters())
					{
						return false;
					}

					//now body
					_mode = em_body;
					return true;
				default:
					assert(!"unknown mode");
					break;
				}
			}
			break;
		case em_headers:
			{
				switch(_mode)
				{
				case em_body:
					//write common headers
					if(!this->writeSystemHeaders())
					{
						return false;
					}

					//terminate headers
					if(!write("\r\n", 2))
					{
						return false;
					}

					//setup body filters
					if(!this->setupBodyFilters())
					{
						return false;
					}

					//now body
					_mode = em_body;

					return true;
				default:
					assert(!"unknown mode");
					break;
				}
			}
			break;
		default:
			assert(!"unknown mode");
			break;
		}

		assert(!"never here");
		return true;
	}

	//////////////////////////////////////////////////////////////
	bool MessageOut::iteratorIncrement()
	{
		assert(_writePosition < _writeEnd);
		_writePosition++;
		if(_writePosition == _writeEnd)
		{
			if(!nextBuffer())
			{
				return false;
			}
		}

		return true;
	}

	//////////////////////////////////////////////////////////////
	char &MessageOut::iteratorDereference()
	{
		return *_writePosition;
	}

}}}
