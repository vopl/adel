#ifndef _NET_HTTP_SERVER_RESPONSE_HPP_
#define _NET_HTTP_SERVER_RESPONSE_HPP_

#include "net/http/version.hpp"
#include "net/http/statusCode.hpp"
#include "net/http/headerValue.hpp"
#include "net/http/headerName.hpp"

namespace net { namespace http { namespace server
{

	namespace impl
	{
		class Response;
		typedef boost::shared_ptr<Response> ResponsePtr;
	}
	///////////////////////////////////////////////////////
	class Response
	{
	protected:
		typedef impl::ResponsePtr ImplPtr;
		ImplPtr _impl;

	protected:
		Response();

	public:
		~Response();

		Response &version(const Version &version);
		Response &statusCode(const EStatusCode &statusCode);

		Response &header(const char *data, size_t size);
		Response &header(const char *dataz);
		Response &header(const std::string &data);

		Response &header(const HeaderName &name, const char *value, size_t valueSize);
		Response &header(const HeaderName &name, const char *valuez);
		Response &header(const HeaderName &name, const std::string &value);

		template <class HeaderValueTag>
		Response &header(const std::string &name, const HeaderValue<HeaderValueTag> &value);

		template <class HeaderValueTag>
		Response &header(const char *namez, const HeaderValue<HeaderValueTag> &value);

		template <class HeaderValueTag>
		Response &header(const HeaderName &name, const HeaderValue<HeaderValueTag> &value);

		Response &body(const char *data, size_t size);
		Response &body(const char *dataz);
		Response &body(const std::string &data);

		bool flush();

	public:
		void setBodySize(size_t size);
		void setBodyCompress(int level, size_t buffer=0);

	private:
		Message::Iterator beginWriteHeader(const char *name, size_t size);
		void endWriteHeader(Message::Iterator iter);
	};




	//////////////////////////////////////////////////////////////////////////////////
	template <class HeaderValueTag>
	Response &Response::header(const std::string &name, const HeaderValue<HeaderValueTag> &value)
	{
		Message::Iterator outIter = beginWriteHeader(name.data(), name.size());
		value.generate(outIter);
		endWriteHeader(outIter);
		return *this;
	}

	template <class HeaderValueTag>
	Response &Response::header(const char *namez, const HeaderValue<HeaderValueTag> &value)
	{
		Message::Iterator outIter = beginWriteHeader(namez, strlen(namez));
		value.generate(outIter);
		endWriteHeader(outIter);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////////////
	template <class HeaderValueTag>
	Response &Response::header(const HeaderName &name, const HeaderValue<HeaderValueTag> &value)
	{
		Message::Iterator outIter = beginWriteHeader(name.str.data(), name.str.size());
		value.generate(outIter);
		endWriteHeader(outIter);
		return *this;
	}


}}}

#endif
