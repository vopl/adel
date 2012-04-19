#include "pch.hpp"
#include "net/http/server/impl/request.hpp"
#include "net/http/impl/server.hpp"

#include "net/http/impl/server.hpp"
#include "net/http/impl/server.hpp"

#include "net/http/server/log.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_string.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_lit.hpp>

#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_container.hpp>

namespace net { namespace http { namespace server { namespace impl
{
	//////////////////////////////////////////////////////////////
	Request::Request(const net::http::impl::ServerPtr &server, const Channel &channel)
		: _server(server)
		, _channel(channel)
	{
	}

	//////////////////////////////////////////////////////////////
	Request::~Request()
	{
	}

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

		static const rule<Message::Iterator> g_parserToken =
				+(
						char_ -
						(
								char_(0, 31) | char_(127) |
								'(' | ')' | '<' | '>' | '@' |
								',' | ';' | ':' | '\\' | '"' |
								'/' | '[' | ']' | '?' | '=' |
								'{' | '}' | ' ' | char_(9)
						)
				);
	}
	//////////////////////////////////////////////////////////////
	bool Request::readRequestLine()
	{
		assert(endInfinity() == _request_.begin());
		assert(endInfinity() == _request_.end());

		if(!obtainMoreChunks())
		{
			return false;
		}

		using namespace boost::spirit::qi;
		namespace qi = boost::spirit::qi;
		namespace px = boost::phoenix;

		Iterator iter = begin();
		bool parseResult = parse(iter, endInfinity(),
			raw[
				raw[
					g_parserMethod[px::ref(_method) = qi::_1] |
					g_parserToken[px::ref(_method) = em_UNKNOWN]
				][px::ref(_method_) = qi::_1] >>

				' ' >>

				raw[
					*(char_-' ')
				][px::ref(_uri_) = qi::_1] >>

				" HTTP/" >>

				raw[
					digit[px::ref(_version._hi) = qi::_1-'0'] >>
					'.' >>
					digit[px::ref(_version._lo) = qi::_1-'0']
				][px::ref(_version_) = qi::_1]
			][px::ref(_requestLine_) = qi::_1] >>
			"\r\n"
		);

		if(!parseResult)
		{
			return false;
		}

		_request_ = Segment(begin(), iter);
		return true;
	}

	//////////////////////////////////////////////////////////////
	bool Request::readHeaders()
	{
		assert(begin() == _request_.begin());
		//assert(endInfinity() != _request_.end());

		using namespace boost::spirit::qi;
		namespace qi = boost::spirit::qi;
		namespace px = boost::phoenix;

		SHeader header;

		Iterator iter = _request_.end();
		bool parseResult = parse(iter, endInfinity(),
			raw[
			    *(
					(
						raw[
							g_parserToken
						][px::ref(header._name_) = qi::_1] >>

						':' >> *space >>

						(
							raw[
								*(char_-"\r\n")
							][px::ref(header._value_) = qi::_1] |
							eps[px::ref(header._value_) = Segment(endInfinity(), endInfinity())]
						)
					)[px::ref(header._header_) = qi::_1]>>
					lit("\r\n")[px::push_back(px::ref(_headersVector), px::ref(header))]
			    )
			][px::ref(_headers_) = qi::_1] >>
			"\r\n"
		);

		if(!parseResult)
		{
			return false;
		}

		_request_ = Segment(begin(), iter);
		return true;
	}

	//////////////////////////////////////////////////////////////
	net::Message::Segment Request::requestLine_() const
	{
		return _requestLine_;
	}

	//////////////////////////////////////////////////////////////
	EMethod Request::method() const
	{
		return _method;
	}

	//////////////////////////////////////////////////////////////
	net::Message::Segment Request::method_() const
	{
		return _method_;
	}

	//////////////////////////////////////////////////////////////
	Version Request::version() const
	{
		return _version;
	}

	//////////////////////////////////////////////////////////////
	net::Message::Segment Request::version_() const
	{
		return _version_;
	}

	//////////////////////////////////////////////////////////////
	net::Message::Segment Request::uri_() const
	{
		return _uri_;
	}

	//////////////////////////////////////////////////////////////
	bool Request::obtainMoreChunks()
	{
		for(;;)
		{
			async::Future2<boost::system::error_code, Packet> ret =
				_channel.receive(_server->requestReadGranula());

			ret.wait();
			if(ret.data1NoWait())
			{
				return false;
			}

			if(ret.data2NoWait()._size)
			{
				SChunk chunk = {_size, ret.data2NoWait()};
				_chunks.push_back(chunk);
				_size += ret.data2NoWait()._size;
				return true;
			}

			assert(!"zero-length data incoming? wtf?");
		}

		assert(!"never here");
		return false;
	}

}}}}
