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
	boost::system::error_code OutputMessage::Iterator::bufferInc(size_t size)
	{
		assert(_message);
		return _message->bufferInc(size);
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::Iterator::write(const char *data, size_t size)
	{
		assert(_message);
		return _message->write(data, size);
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::Iterator::write(const char *dataz)
	{
		assert(_message);
		return _message->write(dataz);
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::Iterator::write(const std::string &data)
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
	boost::system::error_code OutputMessage::Iterator::increment()
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
	OutputMessage::OutputMessage()
		: _impl()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	OutputMessage::OutputMessage(const ImplPtr &impl)
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
	net::Channel OutputMessage::channel()
	{
		return _impl->channel();
	}

	//////////////////////////////////////////////////////////////////////////
	OutputMessage::Iterator OutputMessage::firstLineIterator()
	{
		return _impl->firstLineIterator();
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::firstLine(const char *data, size_t size)
	{
		return _impl->firstLine(data, size);
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::firstLine(const char *dataz)
	{
		return _impl->firstLine(dataz);
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::firstLine(const std::string &data)
	{
		return _impl->firstLine(data);
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::firstLineFlush()
	{
		return _impl->firstLineFlush();
	}

	//////////////////////////////////////////////////////////////////////////
	OutputMessage::Iterator OutputMessage::headersIterator()
	{
		return _impl->headersIterator();
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::header(const char *data, size_t size)
	{
		return _impl->header(data, size);
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::header(const char *dataz)
	{
		return _impl->header(dataz);
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::header(const std::string &data)
	{
		return _impl->header(data);
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::header(const HeaderName &name, const std::string &value)
	{
		return _impl->header(name, value);
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::header(const HeaderName &name, const char *value, size_t valueSize)
	{
		return _impl->header(name, value, valueSize);
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::header(const HeaderName &name, const char *valuez)
	{
		return _impl->header(name, valuez);
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::headersFlush()
	{
		return _impl->headersFlush();
	}

	//////////////////////////////////////////////////////////////////////////
	OutputMessage::Iterator OutputMessage::bodyIterator()
	{
		return _impl->bodyIterator();
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::body(const char *data, size_t size)
	{
		return _impl->body(data, size);
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::body(const char *dataz)
	{
		return _impl->body(dataz);
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::body(const std::string &data)
	{
		return _impl->body(data);
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code OutputMessage::bodyFlush()
	{
		return _impl->bodyFlush();
	}

}
