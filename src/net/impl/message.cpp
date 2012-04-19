#include "pch.hpp"
#include "net/impl/message.hpp"

namespace net { namespace impl
{
	///////////////////////////////////////////////////////
	Message::Message()
		: _size(0)
	{

	}

	///////////////////////////////////////////////////////
	Message::~Message()
	{

	}

	///////////////////////////////////////////////////////
	Message::Iterator Message::begin()
	{
		return Iterator(shared_from_this(), 0, 0);
	}

	///////////////////////////////////////////////////////
	Message::Iterator Message::end()
	{
		return Iterator(shared_from_this(), chunks().size(), 0);
	}

	///////////////////////////////////////////////////////
	Message::Iterator Message::endInfinity()
	{
		return Iterator(shared_from_this(), Iterator::_badOffset, Iterator::_badOffset);
	}

	///////////////////////////////////////////////////////
	Message::TVChunks &Message::chunks()
	{
		return _chunks;
	}

	///////////////////////////////////////////////////////
	const Message::Iterator::difference_type &Message::size() const
	{
		return _size;
	}

	///////////////////////////////////////////////////////
	bool Message::obtainMoreChunks()
	{
		return false;
	}


}}
