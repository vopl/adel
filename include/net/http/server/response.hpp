#ifndef _NET_HTTP_SERVER_RESPONSE_HPP_
#define _NET_HTTP_SERVER_RESPONSE_HPP_

#include "net/http/version.hpp"
#include "net/http/statusCode.hpp"

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
		Response &header(const char *line);
		Response &header(const std::string &data);

		Response &body(const char *data, size_t size);
		Response &body(const char *line);
		Response &body(const std::string &line);

		bool flush();
	};
}}}

#endif
