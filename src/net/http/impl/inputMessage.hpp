#ifndef _NET_HTTP_IMPL_INPUTMESSAGE_HPP_
#define _NET_HTTP_IMPL_INPUTMESSAGE_HPP_

#include "net/packet.hpp"
#include "net/http/inputMessage.hpp"
#include "net/http/impl/inputMessageBuffer.hpp"
#include <boost/enable_shared_from_this.hpp>

namespace net { namespace http { namespace impl
{
	///////////////////////////////////////////////////////////////
	class InputMessage
		: public boost::enable_shared_from_this<InputMessage>
	{
	public:
		typedef net::http::InputMessage::Iterator Iterator;
		typedef net::http::InputMessage::Segment Segment;

	public:
		InputMessage();
		virtual ~InputMessage();

		bool isConnected() const;

		bool readFirstLine();
		bool readHeaders();
		bool readBody();
		bool ignoreBody();


		//requestLine, responseLine
		const Segment &firstLine() const;

		//headers
		const Segment &headers() const;
		const Segment *header(const HeaderName &name) const;
		const Segment *header(size_t key) const;
		const Segment *header(const std::string &name) const;
		const Segment *header(const char *name, size_t nameSize) const;
		const Segment *header(const char *namez) const;

		const Segment &body() const;

	public:
		void reinit();

	protected:
		InputMessageBufferPtr _firstBuffer;
		InputMessageBuffer 	*_lastBuffer;
	};

	typedef boost::shared_ptr<InputMessage> InputMessagePtr;

}}}

#endif
