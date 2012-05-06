#ifndef _NET_HTTP_IMPL_MESSAGEOUT_HPP_
#define _NET_HTTP_IMPL_MESSAGEOUT_HPP_

#include "net/http/messageOut.hpp"
#include "net/channel.hpp"
#include <boost/enable_shared_from_this.hpp>

namespace net { namespace http { namespace impl
{
	class MessageOut
		: boost::enable_shared_from_this<MessageOut>
	{
	public:
		typedef net::http::MessageOut::Iterator Iterator;

	public:
		MessageOut(const Channel &channel, size_t bufferGranula);
		~MessageOut();

		///////////////////////////////////////////////
		bool isConnected() const;

		///////////////////////////////////////////////
		Iterator	firstLineIterator();
		bool		firstLine(const char *data, size_t size);
		bool		firstLine(const char *dataz);
		bool		firstLine(const std::string &data);
		bool		firstLineFlush();

		///////////////////////////////////////////////
		Iterator	headersIterator();

		bool		header(const char *data, size_t size);
		bool		header(const char *dataz);
		bool		header(const std::string &data);

		bool		header(const HeaderName &name, const std::string &value);
		bool		header(const HeaderName &name, const char *value, size_t valueSize);
		bool		header(const HeaderName &name, const char *valuez);

		template <class HeaderValueTag>
		bool		header(const HeaderName &name, const HeaderValue<HeaderValueTag> &value);
		template <class HeaderValueTag>
		bool		header(const char *namez, const HeaderValue<HeaderValueTag> &value);
		template <class HeaderValueTag>
		bool		header(const std::string &name, const HeaderValue<HeaderValueTag> &value);

		bool		headersFlush();

		///////////////////////////////////////////////
		Iterator	bodyIterator();
		bool		body(const char *data, size_t size);
		bool		body(const char *dataz);
		bool		body(const std::string &data);
		bool		bodyFlush();

	public:
		///////////////////////////////////////////////
		char *getBuffer(size_t &size);
		void nextBuffer();
		void iteratorIncrement();
		char &iteratorDereference();
		Iterator iterator();
		bool write(const char *data, size_t size);
		bool write(const char *dataz);
		bool write(const std::string &data);

	protected:
		enum	EMode
		{
			em_firstLine,
			em_headers,
			em_body,

		};

	protected:
		Channel _channel;
		Packet	_buffer;
		size_t	_bufferGranula;
		EMode	_mode;
		char 	*_writePosition;
		char 	*_writeEnd;

	protected:
		bool ensureMode(EMode em);

	};

	typedef boost::shared_ptr<MessageOut> MessageOutPtr;

}}}

#endif
