#ifndef _HTTP_IMPL_OUTPUTMESSAGE_HPP_
#define _HTTP_IMPL_OUTPUTMESSAGE_HPP_

#include "http/outputMessage.hpp"
#include "http/impl/contentFilter.hpp"
#include "http/error.hpp"
#include "net/channel.hpp"
#include <boost/enable_shared_from_this.hpp>

namespace http { namespace impl
{
	////////////////////////////////////////////////////////////////////////
	class OutputMessage
		: public boost::enable_shared_from_this<OutputMessage>
	{
	public:
		typedef http::OutputMessage::Iterator Iterator;

	public:
		OutputMessage(const net::Channel &channel, size_t granula);
		virtual ~OutputMessage();

		///////////////////////////////////////////////
		bool isConnected() const;

		///////////////////////////////////////////////
		Iterator	firstLineIterator();
		boost::system::error_code		firstLine(const char *data, size_t size);
		boost::system::error_code		firstLine(const char *dataz);
		boost::system::error_code		firstLine(const std::string &data);
		boost::system::error_code		firstLineFlush();

		///////////////////////////////////////////////
		Iterator	headersIterator();

		boost::system::error_code		header(const char *data, size_t size);
		boost::system::error_code		header(const char *dataz);
		boost::system::error_code		header(const std::string &data);

		boost::system::error_code		header(const HeaderName &name, const std::string &value);
		boost::system::error_code		header(const HeaderName &name, const char *value, size_t valueSize);
		boost::system::error_code		header(const HeaderName &name, const char *valuez);

		template <class HeaderValueTag>
		boost::system::error_code		header(const char *namez, const HeaderValue<HeaderValueTag> &value);
		template <class HeaderValueTag>
		boost::system::error_code		header(const char *name, size_t nameSize, const HeaderValue<HeaderValueTag> &value);
		template <class HeaderValueTag>
		boost::system::error_code		header(const std::string &name, const HeaderValue<HeaderValueTag> &value);
		template <class HeaderValueTag>
		boost::system::error_code		header(const HeaderName &name, const HeaderValue<HeaderValueTag> &value);

		boost::system::error_code		headersFlush();

		///////////////////////////////////////////////
		Iterator	bodyIterator();
		boost::system::error_code		body(const char *data, size_t size);
		boost::system::error_code		body(const char *dataz);
		boost::system::error_code		body(const std::string &data);
		boost::system::error_code		bodyFlush();

	public:
		///////////////////////////////////////////////
		char *bufferGet(size_t &size);
		boost::system::error_code bufferInc(size_t size);

		boost::system::error_code write(const char *data, size_t size);
		boost::system::error_code write(const char *dataz);
		boost::system::error_code write(const std::string &data);

	protected:
		void bufferEnsure();
		boost::system::error_code bufferFlush();
		boost::system::error_code bufferNext();
		Iterator iterator();

	protected:
		virtual boost::system::error_code writeSystemHeaders();
		virtual boost::system::error_code setupBodyFilters();

	protected:
		net::Channel _channel;

	private:
		enum EMode
		{
			em_firstLine,
			em_headers,
			em_body,
		};

	private:
		net::Packet	_buffer;
		size_t	_granula;
		EMode	_mode;
		char 	*_writeBegin;
		char 	*_writePosition;
		char 	*_writeEnd;

	private:
		boost::system::error_code ensureMode(EMode em);

	private:
		friend class http::OutputMessage::Iterator;
		boost::system::error_code iteratorIncrement();
		char &iteratorDereference();

	protected:
		ContentFilterPtr	_contentFilter;
	};

	typedef boost::shared_ptr<OutputMessage> OutputMessagePtr;




	////////////////////////////////////////////////////////////////////////
	template <class HeaderValueTag>
	boost::system::error_code OutputMessage::header(const char *name, size_t nameSize, const HeaderValue<HeaderValueTag> &value)
	{
		Iterator iter = headersIterator();
		boost::system::error_code ec;
		if((ec = write(name, nameSize)))
		{
			return ec;
		}
		if((ec = write(": ", 2)))
		{
			return ec;
		}
		if(!value.generate(iter))
		{
			return http::error::make(http::error::wrong_value);
		}
		if((ec = write("\r\n", 2)))
		{
			return ec;
		}
		return ec;
	}

	////////////////////////////////////////////////////////////////////////
	template <class HeaderValueTag>
	boost::system::error_code OutputMessage::header(const char *namez, const HeaderValue<HeaderValueTag> &value)
	{
		return header(namez, strlen(namez), value);
	}

	////////////////////////////////////////////////////////////////////////
	template <class HeaderValueTag>
	boost::system::error_code OutputMessage::header(const std::string &name, const HeaderValue<HeaderValueTag> &value)
	{
		return header(name.data(), name.size(), value);
	}

	////////////////////////////////////////////////////////////////////////
	template <class HeaderValueTag>
	boost::system::error_code OutputMessage::header(const HeaderName &name, const HeaderValue<HeaderValueTag> &value)
	{
		return header(name.csz, name.size, value);
	}


}}

#endif
