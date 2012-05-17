#include "pch.hpp"
#include "http/server/impl/request.hpp"
#include "http/impl/server.hpp"

#include "http/impl/server.hpp"
#include "http/impl/server.hpp"
#include "http/headerName.hpp"

#include "http/server/log.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_string.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_lit.hpp>

#include <boost/spirit/include/phoenix_core.hpp>

namespace http { namespace server { namespace impl
{

	//////////////////////////////////////////////////////////////////////////
	Request::Request(const http::impl::ServerPtr &server, const net::Channel &channel)
		: http::impl::InputMessage(channel, server->requestReadGranula())
		, _method_(em_UNKNOWN)
		, _server(server)
		, _channel(channel)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	Request::~Request()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	namespace
	{
		using namespace boost::spirit::qi;
		symbols<char, EMethod> g_parserMethodInitier()
		{
			symbols<char, EMethod> p;
			p.add("OPTIONS", em_OPTIONS);
			p.add("GET", em_GET);
			p.add("POST", em_POST);
			p.add("HEAD", em_HEAD);
			p.add("TRACE", em_TRACE);
			p.add("PUT", em_PUT);
			p.add("DELETE", em_DELETE);
			p.add("CONNECT", em_CONNECT);
			return p;
		}
		static const symbols<char, EMethod> g_parserMethod = g_parserMethodInitier();
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code Request::readFirstLine()
	{
		boost::system::error_code ec;
		if((ec = http::impl::InputMessage::readFirstLine()))
		{
			return ec;
		}

		using namespace boost::spirit::qi;
		namespace qi = boost::spirit::qi;
		namespace px = boost::phoenix;

		Iterator iter = _firstLine.begin();
		Iterator end = _firstLine.end();
		bool parseResult = parse(iter, end,
			raw[
			    raw[
			        g_parserMethod[px::ref(_method_) = qi::_1] |
			        (*ascii::alpha)[px::ref(_method_) = em_UNKNOWN]
				][px::ref(_method) = qi::_1] >>

				' ' >>

				raw[*(char_-char_(" ?"))][px::ref(_path) = qi::_1] >>
					(
						(
							char_('?') >>
							raw[*(char_-' ')][px::ref(_queryString) = qi::_1]
						) |
						raw[eps][px::ref(_queryString) = qi::_1]
					)
				][px::ref(_uri) = qi::_1] >>

				" HTTP/" >>

				raw[
					digit[px::ref(http::impl::InputMessage::_version._hi) = qi::_1-'0'] >>
					'.' >>
					digit[px::ref(http::impl::InputMessage::_version._lo) = qi::_1-'0']
				][px::ref(_version) = qi::_1]
			);

		if(!parseResult || iter!=end)
		{
			return error::make(error::invalid_message);
		}

		return error::make();
	}

	//////////////////////////////////////////////////////////////////////////
	RequestPtr Request::shared_from_this()
	{
		return boost::static_pointer_cast<Request>(http::impl::InputMessage::shared_from_this());
	}

	//////////////////////////////////////////////////////////////////////////
	const EMethod &Request::method_() const
	{
		return _method_;
	}

	//////////////////////////////////////////////////////////////////////////
	const Request::Segment &Request::method() const
	{
		return _method;
	}

	//////////////////////////////////////////////////////////////////////////
	const Version &Request::version_() const
	{
		return http::impl::InputMessage::_version;
	}

	//////////////////////////////////////////////////////////////////////////
	const Request::Segment &Request::version() const
	{
		return _version;
	}

	//////////////////////////////////////////////////////////////////////////
	const Request::Segment &Request::uri() const
	{
		return _uri;
	}

	//////////////////////////////////////////////////////////////////////////
	const Request::Segment &Request::path() const
	{
		return _path;
	}

	//////////////////////////////////////////////////////////////////////////
	const Request::Segment &Request::queryString() const
	{
		return _queryString;
	}

	//////////////////////////////////////////////////////////////////////////
	const ResponsePtr &Request::response()
	{
		if(!_response)
		{
			_response.reset(new Response(_server, _channel, this));
		}

		return _response;
	}

	//////////////////////////////////////////////////////////////////////////
	void Request::reinit()
	{
		_method_ = em_UNKNOWN;
		_method = Segment();

		_version = Segment();

		_uri = Segment();
		_path = Segment();
		_queryString = Segment();

		_response.reset();

		return http::impl::InputMessage::reinit();
	}

}}}
