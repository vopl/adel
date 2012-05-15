#ifndef _HTTP_OUTPUTMESSAGE_HPP_
#define _HTTP_OUTPUTMESSAGE_HPP_

#include "http/headerName.hpp"
#include "http/headerValue.hpp"

namespace http
{
	namespace impl
	{
		class OutputMessage;
		typedef boost::shared_ptr<OutputMessage> OutputMessagePtr;
	}

	///////////////////////////////////////////////////////////////
	class OutputMessage
	{
	protected:
		typedef impl::OutputMessagePtr ImplPtr;
		ImplPtr	_impl;

	public:
		class Iterator
			: public boost::iterator_facade<Iterator, char, boost::incrementable_traversal_tag>
		{
			Iterator();
		public:
			Iterator(const Iterator &i);
			~Iterator();

			char *bufferGet(size_t &size);
			boost::system::error_code bufferInc(size_t size);

			boost::system::error_code write(const char *data, size_t size);
			boost::system::error_code write(const char *dataz);
			boost::system::error_code write(const std::string &data);

		private:
			friend class boost::iterator_core_access;

			reference dereference() const;
			boost::system::error_code increment();

		private:
			friend class impl::OutputMessage;
			Iterator(impl::OutputMessage *message);

		private:
			impl::OutputMessage	*_message;
		};

	protected:
		OutputMessage(const ImplPtr &impl);

	public:
		~OutputMessage();

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
		boost::system::error_code		header(const char *name, size_t nameSize, const HeaderValue<HeaderValueTag> &value);
		template <class HeaderValueTag>
		boost::system::error_code		header(const char *namez, const HeaderValue<HeaderValueTag> &value);
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
	};





	///////////////////////////////////////////////////////////////
	template <class HeaderValueTag>
	boost::system::error_code OutputMessage::header(const char *name, size_t nameSize, const HeaderValue<HeaderValueTag> &value)
	{
		Iterator iter = headersIterator();

		boost::system::error_code ec;
		if((ec = iter.write(name, nameSize)))
		{
			return ec;
		}
		if((ec = iter.write(": ", 2)))
		{
			return ec;
		}
		if(!value.generate(iter))
		{
			return boost::system::errc::make_error_code(boost::system::errc::invalid_argument);
		}
		if((ec = iter.write("\r\n", 2)))
		{
			return ec;
		}

		return ec;
	}

	///////////////////////////////////////////////////////////////
	template <class HeaderValueTag>
	boost::system::error_code OutputMessage::header(const char *namez, const HeaderValue<HeaderValueTag> &value)
	{
		return header(namez, strlen(namez), value);
	}

	///////////////////////////////////////////////////////////////
	template <class HeaderValueTag>
	boost::system::error_code OutputMessage::header(const std::string &name, const HeaderValue<HeaderValueTag> &value)
	{
		return header(name.data(), name.size(), value);
	}

	///////////////////////////////////////////////////////////////
	template <class HeaderValueTag>
	boost::system::error_code OutputMessage::header(const HeaderName &name, const HeaderValue<HeaderValueTag> &value)
	{
		return header(name.csz, name.size, value);
	}

}

#endif
