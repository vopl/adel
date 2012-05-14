#include "pch.hpp"
#include "http/impl/outputMessage.hpp"
#include "http/impl/contentFilterChannelWriter.hpp"

namespace http { namespace impl
{
	//////////////////////////////////////////////////////////////
	OutputMessage::OutputMessage(const net::Channel &channel, size_t granula)
		: _channel(channel)
		, _granula(granula)
		, _mode(em_firstLine)
		, _writeBegin(NULL)
		, _writePosition(NULL)
		, _writeEnd(NULL)
		, _contentFilter(new ContentFilterChannelWriter(channel, granula))
	{
		//bufferEnsure();
	}

	//////////////////////////////////////////////////////////////
	OutputMessage::~OutputMessage()
	{
	}

	//////////////////////////////////////////////////////////////
	bool OutputMessage::isConnected() const
	{
		return _channel.isOpen();
	}

	//////////////////////////////////////////////////////////////
	OutputMessage::Iterator	OutputMessage::firstLineIterator()
	{
		if(!ensureMode(em_firstLine))
		{
			return false;
		}
		return iterator();
	}

	//////////////////////////////////////////////////////////////
	bool OutputMessage::firstLine(const char *data, size_t size)
	{
		if(!ensureMode(em_firstLine))
		{
			return false;
		}
		return write(data, size);
	}

	//////////////////////////////////////////////////////////////
	bool OutputMessage::firstLine(const char *dataz)
	{
		if(!ensureMode(em_firstLine))
		{
			return false;
		}
		return write(dataz);
	}

	//////////////////////////////////////////////////////////////
	bool OutputMessage::firstLine(const std::string &data)
	{
		if(!ensureMode(em_firstLine))
		{
			return false;
		}
		return write(data);
	}

	//////////////////////////////////////////////////////////////
	bool OutputMessage::firstLineFlush()
	{
		return ensureMode(em_headers);
	}

	//////////////////////////////////////////////////////////////
	OutputMessage::Iterator OutputMessage::headersIterator()
	{
		bool b = ensureMode(em_headers);
		assert(b);
		(void)b;
		return iterator();
	}

	//////////////////////////////////////////////////////////////
	bool OutputMessage::header(const char *data, size_t size)
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
	bool OutputMessage::header(const char *dataz)
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
	bool OutputMessage::header(const std::string &data)
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
	bool OutputMessage::header(const HeaderName &name, const std::string &value)
	{
		return header(name, value.data(), value.size());
	}

	//////////////////////////////////////////////////////////////
	bool OutputMessage::header(const HeaderName &name, const char *value, size_t valueSize)
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
	bool OutputMessage::header(const HeaderName &name, const char *valuez)
	{
		return header(name, valuez, strlen(valuez));
	}

	//////////////////////////////////////////////////////////////
	bool OutputMessage::headersFlush()
	{
		return ensureMode(em_body);
	}

	//////////////////////////////////////////////////////////////
	OutputMessage::Iterator OutputMessage::bodyIterator()
	{
		bool b = ensureMode(em_body);
		assert(b);
		(void)b;
		return iterator();
	}

	//////////////////////////////////////////////////////////////
	bool OutputMessage::body(const char *data, size_t size)
	{
		if(!ensureMode(em_body))
		{
			return false;
		}
		return write(data, size);
	}

	//////////////////////////////////////////////////////////////
	bool OutputMessage::body(const char *dataz)
	{
		if(!ensureMode(em_body))
		{
			return false;
		}
		return write(dataz);
	}

	//////////////////////////////////////////////////////////////
	bool OutputMessage::body(const std::string &data)
	{
		if(!ensureMode(em_body))
		{
			return false;
		}
		return write(data);
	}

	//////////////////////////////////////////////////////////////
	bool OutputMessage::bodyFlush()
	{
		if(!ensureMode(em_body))
		{
			return false;
		}

		if(!bufferFlush())
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
	char *OutputMessage::bufferGet(size_t &size)
	{
		bufferEnsure();

		assert(_writeBegin && _writeEnd && _writePosition);

		size_t availSize = _writeEnd - _writePosition;
		if(availSize < size)
		{
			size = availSize;
		}
		return _writePosition;
	}

	//////////////////////////////////////////////////////////////
	bool OutputMessage::bufferInc(size_t size)
	{
		assert(_writePosition);
		_writePosition += size;
		assert(_writePosition <= _writeEnd);
		if(_writePosition == _writeEnd)
		{
			return bufferNext();
		}
		return true;
	}

	//////////////////////////////////////////////////////////////
	bool OutputMessage::write(const char *data, size_t size)
	{
		while(size)
		{
			size_t writeSize = size;
			char *buf = bufferGet(writeSize);

			assert(writeSize && writeSize <= size);
			memcpy(buf, data, writeSize);
			if(!bufferInc(writeSize))
			{
				return false;
			}

			size -= writeSize;
			data += writeSize;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////
	bool OutputMessage::write(const char *dataz)
	{
		return write(dataz, strlen(dataz));
	}

	//////////////////////////////////////////////////////////////
	bool OutputMessage::write(const std::string &data)
	{
		return write(data.data(), data.size());
	}

	//////////////////////////////////////////////////////////////
	void OutputMessage::bufferEnsure()
	{
		if(!_writePosition)
		{
			assert(!_buffer._data);
			assert(!_buffer._size);
			assert(!_writeEnd);
			assert(!_writeBegin);

			_buffer._size = _granula;
			_buffer._data.reset(new char [_buffer._size]);

			_writeBegin = _buffer._data.get();
			_writePosition = _writeBegin;
			_writeEnd = _writeBegin + _buffer._size;
		}
	}

	//////////////////////////////////////////////////////////////
	bool OutputMessage::bufferFlush()
	{
		assert(_buffer._data);
		
		net::Packet buf = _buffer;
		buf._size = _writePosition - buf._data.get();
		size_t offset = _writeBegin - buf._data.get();
		assert(buf._size >= offset);

		if(buf._size > offset)
		{
			if(!_contentFilter->filterPush(buf, offset))
			{
				return false;
			}
		}

		if(_writePosition == _writeEnd)
		{
			_buffer._data.reset();
			_buffer._size = 0;
			_writeBegin = NULL;
			_writePosition = NULL;
			_writeEnd = NULL;
		}
		else
		{
			_writeBegin = _writePosition;
		}

		return true;
	}

	//////////////////////////////////////////////////////////////
	bool OutputMessage::bufferNext()
	{
		if(!bufferFlush())
		{
			return false;
		}
		bufferEnsure();

		return true;
	}
	//////////////////////////////////////////////////////////////
	OutputMessage::Iterator OutputMessage::iterator()
	{
		bufferEnsure();

		return Iterator(this);
	}

	//////////////////////////////////////////////////////////////
	bool OutputMessage::writeSystemHeaders()
	{
		return true;
	}

	//////////////////////////////////////////////////////////////
	bool OutputMessage::setupBodyFilters()
	{
		return true;
	}

	//////////////////////////////////////////////////////////////
	bool OutputMessage::ensureMode(EMode em)
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

					if(!bufferFlush())
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
				switch(em)
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

					if(!bufferFlush())
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
	bool OutputMessage::iteratorIncrement()
	{
		assert(_writePosition < _writeEnd);
		_writePosition++;
		if(_writePosition == _writeEnd)
		{
			if(!bufferNext())
			{
				return false;
			}
		}

		return true;
	}

	//////////////////////////////////////////////////////////////
	char &OutputMessage::iteratorDereference()
	{
		return *_writePosition;
	}

}}
