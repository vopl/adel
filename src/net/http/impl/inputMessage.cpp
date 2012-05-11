#include "pch.hpp"
#include "net/http/impl/inputMessage.hpp"

namespace net { namespace http { namespace impl
{

	InputMessage::InputMessage()
	{
		assert(0);
	}

	//////////////////////////////////////////////////////////////////////////
	InputMessage::~InputMessage()
	{
		assert(0);
	}

	//////////////////////////////////////////////////////////////////////////
	bool InputMessage::isConnected() const
	{
		assert(0);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool InputMessage::readFirstLine()
	{
		assert(0);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool InputMessage::readHeaders()
	{
		assert(0);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool InputMessage::readBody()
	{
		assert(0);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool InputMessage::ignoreBody()
	{
		assert(0);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment &InputMessage::firstLine() const
	{
		assert(0);
		return *(const InputMessage::Segment *)NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment &InputMessage::headers() const
	{
		assert(0);
		return *(const InputMessage::Segment *)NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment *InputMessage::header(const HeaderName &name) const
	{
		assert(0);
		return (const InputMessage::Segment *)NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment *InputMessage::header(size_t key) const
	{
		assert(0);
		return (const InputMessage::Segment *)NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment *InputMessage::header(const std::string &name) const
	{
		assert(0);
		return (const InputMessage::Segment *)NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment *InputMessage::header(const char *name, size_t nameSize) const
	{
		assert(0);
		return (const InputMessage::Segment *)NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment *InputMessage::header(const char *namez) const
	{
		assert(0);
		return (const InputMessage::Segment *)NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment &InputMessage::body() const
	{
		assert(0);
		return *(const InputMessage::Segment *)NULL;
	}
	
	//////////////////////////////////////////////////////////////////////////
	void InputMessage::reinit()
	{
		assert(0);
	}

}}}
