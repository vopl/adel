#include "pch.hpp"
#include "http/outputMessage.hpp"
#include "http/impl/outputMessage.hpp"

namespace http
{
	//////////////////////////////////////////////////////////////////////////
	OutputMessage::Iterator::Iterator()
		: _message(NULL)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	OutputMessage::Iterator::Iterator(const Iterator &i)
		: _message(i._message)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	OutputMessage::Iterator::~Iterator()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	char *OutputMessage::Iterator::bufferGet(size_t &size)
	{
		assert(_message);
		return _message->bufferGet(size);
	}

	//////////////////////////////////////////////////////////////////////////
	bool OutputMessage::Iterator::bufferInc(size_t size)
	{
		assert(_message);
		return _message->bufferInc(size);
	}

	//////////////////////////////////////////////////////////////////////////
	bool OutputMessage::Iterator::write(const char *data, size_t size)
	{
		assert(_message);
		return _message->write(data, size);
	}

	//////////////////////////////////////////////////////////////////////////
	bool OutputMessage::Iterator::write(const char *dataz)
	{
		assert(_message);
		return _message->write(dataz);
	}

	//////////////////////////////////////////////////////////////////////////
	bool OutputMessage::Iterator::write(const std::string &data)
	{
		assert(_message);
		return _message->write(data);
	}

	//////////////////////////////////////////////////////////////////////////
	OutputMessage::Iterator::reference OutputMessage::Iterator::dereference() const
	{
		assert(_message);
		return _message->iteratorDereference();
	}

	//////////////////////////////////////////////////////////////////////////
	bool OutputMessage::Iterator::increment()
	{
		assert(_message);
		return _message->iteratorIncrement();
	}

	//////////////////////////////////////////////////////////////////////////
	OutputMessage::Iterator::Iterator(impl::OutputMessage *message)
		: _message(message)
	{
		assert(_message);
	}








	//////////////////////////////////////////////////////////////////////////
	OutputMessage::OutputMessage(ImplPtr impl)
		: _impl(impl)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	OutputMessage::~OutputMessage()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	bool OutputMessage::isConnected() const
	{
		return _impl->isConnected();
	}

	//////////////////////////////////////////////////////////////////////////
	OutputMessage::Iterator OutputMessage::firstLineIterator()
	{
		return _impl->firstLineIterator();
	}

	//////////////////////////////////////////////////////////////////////////
	bool OutputMessage::firstLine(const char *data, size_t size)
	{
		return _impl->firstLine(data, size);
	}

	//////////////////////////////////////////////////////////////////////////
	bool OutputMessage::firstLine(const char *dataz)
	{
		return _impl->firstLine(dataz);
	}

	//////////////////////////////////////////////////////////////////////////
	bool OutputMessage::firstLine(const std::string &data)
	{
		return _impl->firstLine(data);
	}

	//////////////////////////////////////////////////////////////////////////
	bool OutputMessage::firstLineFlush()
	{
		return _impl->firstLineFlush();
	}

	//////////////////////////////////////////////////////////////////////////
	OutputMessage::Iterator OutputMessage::headersIterator()
	{
		return _impl->headersIterator();
	}

	//////////////////////////////////////////////////////////////////////////
	bool OutputMessage::header(const char *data, size_t size)
	{
		return _impl->header(data, size);
	}

	//////////////////////////////////////////////////////////////////////////
	bool OutputMessage::header(const char *dataz)
	{
		return _impl->header(dataz);
	}

	//////////////////////////////////////////////////////////////////////////
	bool OutputMessage::header(const std::string &data)
	{
		return _impl->header(data);
	}

	//////////////////////////////////////////////////////////////////////////
	bool OutputMessage::header(const HeaderName &name, const std::string &value)
	{
		return _impl->header(name, value);
	}

	//////////////////////////////////////////////////////////////////////////
	bool OutputMessage::header(const HeaderName &name, const char *value, size_t valueSize)
	{
		return _impl->header(name, value, valueSize);
	}

	//////////////////////////////////////////////////////////////////////////
	bool OutputMessage::header(const HeaderName &name, const char *valuez)
	{
		return _impl->header(name, valuez);
	}

	//////////////////////////////////////////////////////////////////////////
	bool OutputMessage::headersFlush()
	{
		return _impl->headersFlush();
	}

	//////////////////////////////////////////////////////////////////////////
	OutputMessage::Iterator OutputMessage::bodyIterator()
	{
		return _impl->bodyIterator();
	}

	//////////////////////////////////////////////////////////////////////////
	bool OutputMessage::body(const char *data, size_t size)
	{
		return _impl->body(data, size);
	}

	//////////////////////////////////////////////////////////////////////////
	bool OutputMessage::body(const char *dataz)
	{
		return _impl->body(dataz);
	}

	//////////////////////////////////////////////////////////////////////////
	bool OutputMessage::body(const std::string &data)
	{
		return _impl->body(data);
	}

	//////////////////////////////////////////////////////////////////////////
	bool OutputMessage::bodyFlush()
	{
		return _impl->bodyFlush();
	}

}