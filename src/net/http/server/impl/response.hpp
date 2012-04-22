#ifndef _NET_HTTP_SERVER_IMPL_RESPONSE_HPP_
#define _NET_HTTP_SERVER_IMPL_RESPONSE_HPP_

#include "net/channel.hpp"
#include <boost/shared_ptr.hpp>
#include "net/http/statusCode.hpp"
#include "net/http/version.hpp"
#include "net/http/contentEncoding.hpp"
#include "net/http/transferEncoding.hpp"
#include "net/impl/message.hpp"

namespace net { namespace http { namespace impl
{
	class Server;
	typedef boost::shared_ptr<Server> ServerPtr;
}}}

namespace net { namespace http { namespace server { namespace impl
{
	class Response
		: public net::impl::Message
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

		bool flush(bool withTail = false);

	private:
		net::http::impl::ServerPtr	_server;
		Channel						_channel;

		virtual bool obtainMoreChunks();

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
		size_t		_sendChunk;


		Version _version;
		EStatusCode _statusCode;

		//std::vector<SHeader> _headersVector;

	};
	typedef boost::shared_ptr<Response> ResponsePtr;

}}}}

#endif
