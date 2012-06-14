#include "pch.hpp"
#include "http/client/impl/response.hpp"
#include "http/client/impl/request.hpp"
#include "http/impl/client.hpp"
#include "http/impl/contentEncoderChunked.hpp"
#include "http/impl/contentEncoderZlib.hpp"
#include "http/headerName.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_string.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_uint.hpp>
#include <boost/spirit/include/qi_lit.hpp>

#include <boost/spirit/include/phoenix_core.hpp>


namespace http { namespace client { namespace impl
{

	////////////////////////////////////////////////////////////////////////////////////////
	Response::Response(const http::impl::ClientPtr &client, const net::Channel &channel, Request *request)
		: http::impl::InputMessage(channel, client->responseReadGranula())
	{
	}

	////////////////////////////////////////////////////////////////////////////////////////
	Response::~Response()
	{
	}

	////////////////////////////////////////////////////////////////////////////////////////
	EStatusCode Response::status() const
	{
		return _status;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	const Version &Response::version() const
	{
		return _version;
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code Response::readFirstLine()
	{
		boost::system::error_code ec;
		ec = InputMessage::readFirstLine();
		if(ec)
		{
			return ec;
		}

		using namespace boost::spirit::qi;
		namespace qi = boost::spirit::qi;
		namespace px = boost::phoenix;

		Iterator iter = _firstLine.begin();
		Iterator end = _firstLine.end();
		unsigned status;
		bool parseResult = parse(iter, end,

			lit("HTTP/") >>

			digit[px::ref(http::impl::InputMessage::_version._hi) = qi::_1-'0'] >>
			'.' >>
			digit[px::ref(http::impl::InputMessage::_version._lo) = qi::_1-'0'] >>

			lit(' ') >>

			uint_parser<unsigned, 10>()[px::ref(status)=qi::_1]
		);

		if(!parseResult)
		{
			return http::error::make(http::error::bad_message);
		}
		_status = (EStatusCode)status;
		return ec;
	}

}}}
