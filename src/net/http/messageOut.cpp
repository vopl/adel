#include "pch.hpp"
#include "net/http/messageOut.hpp"
#include "net/http/impl/messageOut.hpp"

namespace net { namespace http
{
	//////////////////////////////////////////////////////////////////////////
	MessageOut::Iterator::Iterator()
		: _message(NULL)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	MessageOut::Iterator::Iterator(const Iterator &i)
		: _message(i._message)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	MessageOut::Iterator::~Iterator()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	char *MessageOut::Iterator::getBuffer(size_t &size)
	{
		assert(_message);
		return _message->getBuffer(size);
	}

	//////////////////////////////////////////////////////////////////////////
	void MessageOut::Iterator::nextBuffer()
	{
		assert(_message);
		return _message->nextBuffer();
	}

	//////////////////////////////////////////////////////////////////////////
	bool MessageOut::Iterator::write(const char *data, size_t size)
	{
		return _message->write(data, size);
	}

	//////////////////////////////////////////////////////////////////////////
	bool MessageOut::Iterator::write(const char *dataz)
	{
		return _message->write(dataz);
	}

	//////////////////////////////////////////////////////////////////////////
	bool MessageOut::Iterator::write(const std::string &data)
	{
		return _message->write(data);
	}

	//////////////////////////////////////////////////////////////////////////
	MessageOut::Iterator::reference MessageOut::Iterator::dereference() const
	{
		return _message->iteratorDereference();
	}

	//////////////////////////////////////////////////////////////////////////
	bool MessageOut::Iterator::equal(const Iterator &i) const
	{
		assert(_message && _message == i._message);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void MessageOut::Iterator::increment()
	{
		_message->iteratorIncrement();
	}

	//////////////////////////////////////////////////////////////////////////
	MessageOut::Iterator::Iterator(impl::MessageOut *message)
		: _message(message)
	{
	}








	//////////////////////////////////////////////////////////////////////////
	MessageOut::MessageOut()
	{
		assert(0);
	}

	//////////////////////////////////////////////////////////////////////////
	MessageOut::~MessageOut()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	bool MessageOut::isConnected() const
	{
		return _impl->isConnected();
	}

	//////////////////////////////////////////////////////////////////////////
	MessageOut::Iterator	MessageOut::firstLineIterator()
	{
		return _impl->firstLineIterator();
	}

	//////////////////////////////////////////////////////////////////////////
	bool		MessageOut::firstLine(const char *data, size_t size)
	{
		return _impl->firstLine(data, size);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		MessageOut::firstLine(const char *dataz)
	{
		return _impl->firstLine(dataz);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		MessageOut::firstLine(const std::string &data)
	{
		return _impl->firstLine(data);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		MessageOut::firstLineFlush()
	{
		return _impl->firstLineFlush();
	}

	//////////////////////////////////////////////////////////////////////////
	MessageOut::Iterator	MessageOut::headersIterator()
	{
		return _impl->headersIterator();
	}

	//////////////////////////////////////////////////////////////////////////
	bool		MessageOut::header(const char *data, size_t size)
	{
		return _impl->header(data, size);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		MessageOut::header(const char *dataz)
	{
		return _impl->header(dataz);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		MessageOut::header(const std::string &data)
	{
		return _impl->header(data);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		MessageOut::header(const HeaderName &name, const std::string &value)
	{
		return _impl->header(name, value);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		MessageOut::header(const HeaderName &name, const char *value, size_t valueSize)
	{
		return _impl->header(name, value, valueSize);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		MessageOut::header(const HeaderName &name, const char *valuez)
	{
		return _impl->header(name, valuez);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		MessageOut::headersFlush()
	{
		return _impl->headersFlush();
	}

	//////////////////////////////////////////////////////////////////////////
	MessageOut::Iterator	MessageOut::bodyIterator()
	{
		return _impl->bodyIterator();
	}

	//////////////////////////////////////////////////////////////////////////
	bool		MessageOut::body(const char *data, size_t size)
	{
		return _impl->body(data, size);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		MessageOut::body(const char *dataz)
	{
		return _impl->body(dataz);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		MessageOut::body(const std::string &data)
	{
		return _impl->body(data);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		MessageOut::bodyFlush()
	{
		return _impl->bodyFlush();
	}

}}
