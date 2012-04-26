#ifndef _NET_HTTP_SERVER_IMPL_RESPONSE_HPP_
#define _NET_HTTP_SERVER_IMPL_RESPONSE_HPP_

#include "net/channel.hpp"
#include <boost/shared_ptr.hpp>
#include "net/http/statusCode.hpp"
#include "net/http/version.hpp"
#include "net/http/contentEncoding.hpp"
#include "net/http/transferEncoding.hpp"
#include "net/http/headerValue.hpp"
#include "net/impl/message.hpp"
#include "net/http/impl/contentFilter.hpp"

namespace net { namespace http { namespace impl
{
	class Server;
	typedef boost::shared_ptr<Server> ServerPtr;
}}}

namespace net { namespace http { namespace server { namespace impl
{
	class Response
		: public net::impl::Message
		, public net::http::impl::ContentFilter

	{
	public:
		typedef net::Message::Segment Segment;

	public:
		Response(const net::http::impl::ServerPtr &server, const Channel &channel);
		~Response();

		void version(const Version &version);
		void statusCode(const EStatusCode &statusCode);

		void statusLine();
		void header(const char *data, size_t size);
		void header(const char *dataz);
		void header(const std::string &data);

		void header(const HeaderName &name, const char *value, size_t valueSize);
		void header(const HeaderName &name, const char *valuez);
		void header(const HeaderName &name, const std::string &value);

		template <class HeaderValueTag>
		void header(const char *namez, const HeaderValue<HeaderValueTag> &value);

		template <class HeaderValueTag>
		void header(const std::string &name, const HeaderValue<HeaderValueTag> &value);

		template <class HeaderValueTag>
		void header(const HeaderName &name, const HeaderValue<HeaderValueTag> &value);

		void body(const char *data, size_t size);

		bool flush();

	public:
		void setBodySize(size_t size);
		void setBodyCompress(int level, size_t buffer=0);

		Message::Iterator beginWriteHeader(const char *name, size_t size);
		void endWriteHeader(Message::Iterator iter);

	private:
		net::http::impl::ServerPtr	_server;
		Channel						_channel;

		void pushLastChunk2Filter();

		virtual bool obtainMoreChunks();

		net::http::impl::ContentFilter *_mostContentFilter;
		std::vector<net::http::impl::ContentFilterPtr> _filterKeeper;
		virtual bool filterPush(const Packet &packet, size_t offset);
		virtual bool filterFlush();

	private:
		size_t _outputGranula;
		Packet _output;

		size_t	_bodySize;
		int		_bodyCompressLevel;
		size_t	_bodyCompressBuffer;

	private:
		void systemHeaders();

	private:

		enum EWritePart// что пишется в текущий момент
		{
			ewp_statusLine,
			ewp_headers,
			ewp_body,
		} _ewp;

		Iterator	_writePosition;
		Iterator	_bodyPosition;


		Version _version;
		EStatusCode _statusCode;

		//std::vector<SHeader> _headersVector;

	};
	typedef boost::shared_ptr<Response> ResponsePtr;





	//////////////////////////////////////////////////////////////////////////
	template <class HeaderValueTag>
	void Response::header(const char *namez, const HeaderValue<HeaderValueTag> &value)
	{
		Message::Iterator outIter = beginWriteHeader(namez, strlen(namez));
		value.generate(outIter);
		endWriteHeader(outIter);
	}

	//////////////////////////////////////////////////////////////////////////
	template <class HeaderValueTag>
	void Response::header(const std::string &name, const HeaderValue<HeaderValueTag> &value)
	{
		Message::Iterator outIter = beginWriteHeader(name.data(), name.length());
		value.generate(outIter);
		endWriteHeader(outIter);
	}

	//////////////////////////////////////////////////////////////////////////
	template <class HeaderValueTag>
	void Response::header(const HeaderName &name, const HeaderValue<HeaderValueTag> &value)
	{
		Message::Iterator outIter = beginWriteHeader(name.str.data(), name.str.length());
		value.generate(outIter);
		endWriteHeader(outIter);
	}

}}}}

#endif
