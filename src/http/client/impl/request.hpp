#ifndef _HTTP_CLIENT_IMPL_REQUEST_HPP_
#define _HTTP_CLIENT_IMPL_REQUEST_HPP_

#include "net/channel.hpp"
#include <boost/shared_ptr.hpp>
#include "http/method.hpp"
#include "http/version.hpp"
#include "http/impl/outputMessage.hpp"
#include "http/client/impl/response.hpp"

#include <boost/unordered_map.hpp>

namespace http { namespace impl
{
	class Client;
	typedef boost::shared_ptr<Client> ClientPtr;
}}

namespace http { namespace client { namespace impl
{
	class Request;
	typedef boost::shared_ptr<Request> RequestPtr;

	class Response;
	typedef boost::shared_ptr<Response> ResponsePtr;

	///////////////////////////////////////////////////////////////////////////
	class Request
		: public http::impl::OutputMessage
	{
	public:
		Request(const http::impl::ClientPtr &client, const net::Channel &channel);
		~Request();

		boost::system::error_code firstLine(EMethod method, const char *path, size_t pathSize, const Version &version);
		boost::system::error_code firstLine(EMethod method, const char *pathz, const Version &version);
		boost::system::error_code firstLine(EMethod method, const std::string &path, const Version &version);

	public:
		ResponsePtr response();

	private:
		http::impl::ClientPtr	_client;
		ResponsePtr				_response;

	private:
		virtual boost::system::error_code writeSystemHeaders();
		virtual boost::system::error_code setupBodyFilters();

	};
}}}

#endif
