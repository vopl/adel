#ifndef _NET_HTTP_IMPL_OUTPUTMESSAGE_HPP_
#define _NET_HTTP_IMPL_OUTPUTMESSAGE_HPP_

#include "net/http/outputMessage.hpp"
#include "net/http/impl/contentFilter.hpp"
#include "net/channel.hpp"
#include <boost/enable_shared_from_this.hpp>

namespace net { namespace http { namespace impl
{
	////////////////////////////////////////////////////////////////////////
	class OutputMessage
		: public boost::enable_shared_from_this<OutputMessage>
	{
	public:
		typedef net::http::OutputMessage::Iterator Iterator;

	public:
		OutputMessage(const Channel &channel, size_t granula);
		virtual ~OutputMessage();

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
		bool		header(const char *namez, const HeaderValue<HeaderValueTag> &value);
		template <class HeaderValueTag>
		bool		header(const char *name, size_t nameSize, const HeaderValue<HeaderValueTag> &value);
		template <class HeaderValueTag>
		bool		header(const std::string &name, const HeaderValue<HeaderValueTag> &value);
		template <class HeaderValueTag>
		bool		header(const HeaderName &name, const HeaderValue<HeaderValueTag> &value);

		bool		headersFlush();

		///////////////////////////////////////////////
		Iterator	bodyIterator();
		bool		body(const char *data, size_t size);
		bool		body(const char *dataz);
		bool		body(const std::string &data);
		bool		bodyFlush();

	public:
		///////////////////////////////////////////////
		char *bufferGet(size_t &size);
		bool bufferInc(size_t size);

		bool write(const char *data, size_t size);
		bool write(const char *dataz);
		bool write(const std::string &data);

	protected:
		void bufferEnsure();
		bool bufferFlush();
		bool bufferNext();
		Iterator iterator();

	protected:
		virtual bool writeSystemHeaders();
		virtual bool setupBodyFilters();

	protected:
		Channel _channel;

	private:
		enum EMode
		{
			em_firstLine,
			em_headers,
			em_body,
		};

	private:
		Packet	_buffer;
		size_t	_granula;
		EMode	_mode;
		char 	*_writeBegin;
		char 	*_writePosition;
		char 	*_writeEnd;

	private:
		bool ensureMode(EMode em);

	private:
		friend class net::http::OutputMessage::Iterator;
		bool iteratorIncrement();
		char &iteratorDereference();

	protected:
		ContentFilterPtr	_contentFilter;
	};

	typedef boost::shared_ptr<OutputMessage> OutputMessagePtr;




	////////////////////////////////////////////////////////////////////////
	template <class HeaderValueTag>
	bool OutputMessage::header(const char *name, size_t nameSize, const HeaderValue<HeaderValueTag> &value)
	{
		Iterator iter = headersIterator();
		if(!write(name, nameSize))
		{
			return false;
		}
		if(!write(": ", 2))
		{
			return false;
		}
		if(!value.generate(iter))
		{
			return false;
		}
		if(!write("\r\n", 2))
		{
			return false;
		}
		return true;
	}

	////////////////////////////////////////////////////////////////////////
	template <class HeaderValueTag>
	bool OutputMessage::header(const char *namez, const HeaderValue<HeaderValueTag> &value)
	{
		return header(namez, strlen(namez), value);
	}

	////////////////////////////////////////////////////////////////////////
	template <class HeaderValueTag>
	bool OutputMessage::header(const std::string &name, const HeaderValue<HeaderValueTag> &value)
	{
		return header(name.data(), name.size(), value);
	}

	////////////////////////////////////////////////////////////////////////
	template <class HeaderValueTag>
	bool OutputMessage::header(const HeaderName &name, const HeaderValue<HeaderValueTag> &value)
	{
		return header(name.csz, name.size, value);
	}


}}}

#endif
