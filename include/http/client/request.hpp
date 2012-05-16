#ifndef _HTTP_CLIENT_REQUEST_HPP_
#define _HTTP_CLIENT_REQUEST_HPP_

#include "http/client/response.hpp"
#include "http/outputMessage.hpp"
#include "http/method.hpp"
#include "http/version.hpp"

namespace http { namespace client
{

	namespace impl
	{
		class Request;
		typedef boost::shared_ptr<Request> RequestPtr;
	}
	///////////////////////////////////////////////////////
	class Request
		: public http::OutputMessage
	{
	protected:
		typedef impl::RequestPtr ImplPtr;
		ImplPtr _impl;

	public:
		Request();
		Request(const ImplPtr &impl);
		~Request();

		boost::system::error_code firstLine(EMethod method, const char *path, size_t pathSize, const Version &version);
		boost::system::error_code firstLine(EMethod method, const char *pathz, const Version &version);
		boost::system::error_code firstLine(EMethod method, const std::string &path, const Version &version);

		//keep alive
		//body size
		//compress
	};
}}

#endif
