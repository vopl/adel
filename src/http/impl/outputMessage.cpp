#include "pch.hpp"
#include "http/impl/outputMessage.hpp"
#include "http/impl/contentEncoderWriter.hpp"
#include "http/error.hpp"

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
		, _contentEncoder(new ContentEncoderWriter(channel, granula))
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
		boost::system::error_code ec;
		if((ec = ensureMode(em_firstLine)))
		{
			return iterator();
		}
		return iterator();
	}

	//////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::firstLine(const char *data, size_t size)
	{
		boost::system::error_code ec;
		if((ec = ensureMode(em_firstLine)))
		{
			return ec;
		}
		return write(data, size);
	}

	//////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::firstLine(const char *dataz)
	{
		boost::system::error_code ec;
		if((ec = ensureMode(em_firstLine)))
		{
			return ec;
		}
		return write(dataz);
	}

	//////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::firstLine(const std::string &data)
	{
		boost::system::error_code ec;
		if((ec = ensureMode(em_firstLine)))
		{
			return ec;
		}
		return write(data);
	}

	//////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::firstLineFlush()
	{
		return ensureMode(em_headers);
	}

	//////////////////////////////////////////////////////////////
	OutputMessage::Iterator OutputMessage::headersIterator()
	{
		boost::system::error_code ec = ensureMode(em_headers);
		assert(!ec);
		(void)ec;
		return iterator();
	}

	//////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::header(const char *data, size_t size)
	{
		boost::system::error_code ec;
		if((ec = ensureMode(em_headers)))
		{
			return ec;
		}
		if((ec = write(data, size)))
		{
			return ec;
		}
		return write("\r\n", 2);
	}

	//////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::header(const char *dataz)
	{
		boost::system::error_code ec;
		if((ec = ensureMode(em_headers)))
		{
			return ec;
		}
		if((ec = write(dataz)))
		{
			return ec;
		}
		return write("\r\n", 2);
	}

	//////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::header(const std::string &data)
	{
		boost::system::error_code ec;
		if((ec = ensureMode(em_headers)))
		{
			return ec;
		}
		if((ec = write(data)))
		{
			return ec;
		}
		return write("\r\n", 2);
	}

	//////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::header(const HeaderName &name, const std::string &value)
	{
		return header(name, value.data(), value.size());
	}

	//////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::header(const HeaderName &name, const char *value, size_t valueSize)
	{
		boost::system::error_code ec;
		if((ec = ensureMode(em_headers)))
		{
			return ec;
		}
		if((ec = write(name.str)))
		{
			return ec;
		}
		if((ec = write(": ", 2)))
		{
			return ec;
		}
		if((ec = write(value, valueSize)))
		{
			return ec;
		}
		if((ec = write("\r\n", 2)))
		{
			return ec;
		}
		return error::make();
	}

	//////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::header(const HeaderName &name, const char *valuez)
	{
		return header(name, valuez, strlen(valuez));
	}

	//////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::headersFlush()
	{
		return ensureMode(em_body);
	}

	//////////////////////////////////////////////////////////////
	OutputMessage::Iterator OutputMessage::bodyIterator()
	{
		boost::system::error_code ec = ensureMode(em_body);
		assert(!ec);
		(void)ec;
		return iterator();
	}

	//////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::body(const char *data, size_t size)
	{
		boost::system::error_code ec;
		if((ec = ensureMode(em_body)))
		{
			return ec;
		}
		return write(data, size);
	}

	//////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::body(const char *dataz)
	{
		boost::system::error_code ec;
		if((ec = ensureMode(em_body)))
		{
			return ec;
		}
		return write(dataz);
	}

	//////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::body(const std::string &data)
	{
		boost::system::error_code ec;
		if((ec = ensureMode(em_body)))
		{
			return ec;
		}
		return write(data);
	}

	//////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::bodyFlush()
	{
		boost::system::error_code ec;
		if((ec = ensureMode(em_body)))
		{
			return ec;
		}

		if((ec = bufferFlush()))
		{
			return ec;
		}

		if((ec = _contentEncoder->filterFlush()))
		{
			return ec;
		}
		_mode = em_firstLine;
		return error::make();
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
	boost::system::error_code OutputMessage::bufferInc(size_t size)
	{
		assert(_writePosition);
		_writePosition += size;
		assert(_writePosition <= _writeEnd);
		if(_writePosition == _writeEnd)
		{
			return bufferNext();
		}
		return error::make();
	}

	//////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::write(const char *data, size_t size)
	{
		boost::system::error_code ec;
		while(size)
		{
			size_t writeSize = size;
			char *buf = bufferGet(writeSize);

			assert(writeSize && writeSize <= size);
			memcpy(buf, data, writeSize);
			if((ec = bufferInc(writeSize)))
			{
				return ec;
			}

			size -= writeSize;
			data += writeSize;
		}
		return error::make();
	}

	//////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::write(const char *dataz)
	{
		return write(dataz, strlen(dataz));
	}

	//////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::write(const std::string &data)
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
	boost::system::error_code OutputMessage::bufferFlush()
	{
		assert(_buffer._data);
		
		net::Packet buf = _buffer;
		buf._size = _writePosition - buf._data.get();
		size_t offset = _writeBegin - buf._data.get();
		assert(buf._size >= offset);

		boost::system::error_code ec;

		if(buf._size > offset)
		{
			if((ec = _contentEncoder->filterPush(buf, offset)))
			{
				return ec;
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

		return error::make();
	}

	//////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::bufferNext()
	{
		boost::system::error_code ec;

		if((ec = bufferFlush()))
		{
			return ec;
		}
		bufferEnsure();

		return error::make();
	}
	//////////////////////////////////////////////////////////////
	OutputMessage::Iterator OutputMessage::iterator()
	{
		bufferEnsure();

		return Iterator(this);
	}

	//////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::writeSystemHeaders()
	{
		return error::make();
	}

	//////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::setupBodyFilters()
	{
		return error::make();
	}

	//////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::ensureMode(EMode em)
	{
		assert(_mode <= em);
		if(_mode == em)
		{
			return error::make();
		}

		boost::system::error_code ec;
		switch(_mode)
		{
		case em_firstLine:
			{
				switch(em)
				{
				case em_headers:
					//terminate first line
					if((ec = write("\r\n", 2)))
					{
						return ec;
					}

					//now headers
					_mode = em_headers;

					return error::make();
				case em_body:
					//terminate first line
					if((ec = write("\r\n", 2)))
					{
						return ec;
					}

					//now headers
					_mode = em_headers;

					//write common headers(server, date, content-encoding, connection, ...)
					if((ec = this->writeSystemHeaders()))
					{
						return ec;
					}

					//terminate headers
					if((ec = write("\r\n", 2)))
					{
						return ec;
					}

					if((ec = bufferFlush()))
					{
						return ec;
					}
					//setup body filters
					if((ec = this->setupBodyFilters()))
					{
						return ec;
					}

					//now body
					_mode = em_body;

					return error::make();
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
					if((ec = this->writeSystemHeaders()))
					{
						return ec;
					}

					//terminate headers
					if((ec = write("\r\n", 2)))
					{
						return ec;
					}

					if((ec = bufferFlush()))
					{
						return ec;
					}
					//setup body filters
					if((ec = this->setupBodyFilters()))
					{
						return ec;
					}

					//now body
					_mode = em_body;

					return error::make();
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
		return error::make(error::unexpected);
	}

	//////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::iteratorIncrement()
	{
		assert(_writePosition < _writeEnd);
		_writePosition++;
		if(_writePosition == _writeEnd)
		{
			boost::system::error_code ec;
			if((ec = bufferNext()))
			{
				return ec;
			}
		}

		return error::make();
	}

	//////////////////////////////////////////////////////////////
	char &OutputMessage::iteratorDereference()
	{
		return *_writePosition;
	}

}}
