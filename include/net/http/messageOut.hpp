#ifndef _NET_HTTP_MESSAGEOUT_HPP_
#define _NET_HTTP_MESSAGEOUT_HPP_

#include "net/http/headerName.hpp"
#include "net/http/headerValue.hpp"

namespace net { namespace http
{
	namespace impl
	{
		class MessageOut;
		typedef boost::shared_ptr<MessageOut> MessageOutPtr;
	}

	///////////////////////////////////////////////////////////////
	class MessageOut
	{
	protected:
		typedef impl::MessageOutPtr ImplPtr;
		ImplPtr	_impl;

	public:
		class Iterator
			: public boost::iterator_facade<Iterator, char, boost::incrementable_traversal_tag>
		{
			Iterator();
		public:
			Iterator(const Iterator &i);
			~Iterator();

			char *getBuffer(size_t &size);
			bool nextBuffer();
			bool write(const char *data, size_t size);
			bool write(const char *dataz);
			bool write(const std::string &data);

		private:
			friend class boost::iterator_core_access;

			reference dereference() const;
			bool increment();

		private:
			friend class impl::MessageOut;
			Iterator(impl::MessageOut *message);

		private:
			impl::MessageOut	*_message;
		};

	protected:
		MessageOut(ImplPtr impl);

	public:
		~MessageOut();

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
		bool		header(const char *name, size_t nameSize, const HeaderValue<HeaderValueTag> &value);
		template <class HeaderValueTag>
		bool		header(const char *namez, const HeaderValue<HeaderValueTag> &value);
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
	};





	///////////////////////////////////////////////////////////////
	template <class HeaderValueTag>
	bool MessageOut::header(const char *name, size_t nameSize, const HeaderValue<HeaderValueTag> &value)
	{
		Iterator iter = headersIterator();

		if(!iter.write(name, nameSize))
		{
			return false;
		}
		if(!iter.write(": ", 2))
		{
			return false;
		}
		if(!value.generate(iter))
		{
			return false;
		}
		if(!iter.write("\r\n", 2))
		{
			return false;
		}

		return true;
	}

	///////////////////////////////////////////////////////////////
	template <class HeaderValueTag>
	bool MessageOut::header(const char *namez, const HeaderValue<HeaderValueTag> &value)
	{
		return header(namez, strlen(namez), value);
	}

	///////////////////////////////////////////////////////////////
	template <class HeaderValueTag>
	bool MessageOut::header(const std::string &name, const HeaderValue<HeaderValueTag> &value)
	{
		return header(name.data(), name.size(), value);
	}

	///////////////////////////////////////////////////////////////
	template <class HeaderValueTag>
	bool MessageOut::header(const HeaderName &name, const HeaderValue<HeaderValueTag> &value)
	{
		return header(name.csz, name.size, value);
	}

}}

#endif
