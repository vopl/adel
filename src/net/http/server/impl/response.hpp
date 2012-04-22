#ifndef _NET_HTTP_SERVER_IMPL_RESPONSE_HPP_
#define _NET_HTTP_SERVER_IMPL_RESPONSE_HPP_

#include "net/channel.hpp"
#include <boost/shared_ptr.hpp>
#include "net/http/statusCode.hpp"
#include "net/http/version.hpp"
#include "net/http/contentEncoding.hpp"
#include "net/http/transferEncoding.hpp"
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
		void body(const char *data, size_t size);

		bool flush();

	private:
		net::http::impl::ServerPtr	_server;
		Channel						_channel;

		void pushLastChunk();

		virtual bool obtainMoreChunks();

		net::http::impl::ContentFilter *_mostContentFilter;
		virtual boost::uint32_t filterPush(const Packet &packet, boost::uint32_t offset);
		virtual boost::uint32_t filterFlush();


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

}}}}

#endif
