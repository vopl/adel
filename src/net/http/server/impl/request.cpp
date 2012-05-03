#include "pch.hpp"
#include "net/http/server/impl/request.hpp"
#include "net/http/impl/server.hpp"

#include "net/http/impl/server.hpp"
#include "net/http/impl/server.hpp"
#include "net/http/headerName.hpp"

#include "net/http/server/log.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_string.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_lit.hpp>

#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_container.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>

namespace net { namespace http { namespace server { namespace impl
{
	//////////////////////////////////////////////////////////////
	RequestPtr Request::shared_from_this()
	{
		return boost::static_pointer_cast<Request>(net::http::impl::Message::shared_from_this());
	}

	//////////////////////////////////////////////////////////////
	Request::Request(const net::http::impl::ServerPtr &server, const Channel &channel)
		: _server(server)
		, _channel(channel)
		, _method(em_UNKNOWN)
		, _version()
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

		if(!_firstBuffer && !obtainMoreBuffers(true))
		{
			return false;
		}

		using namespace boost::spirit::qi;
		namespace qi = boost::spirit::qi;
		namespace px = boost::phoenix;

		MessageIterator iter = begin();
		bool parseResult = parse(iter, endInfinity(),
			raw[
				raw[
					g_parserMethod[px::ref(_method) = qi::_1] |
					g_parserToken[px::ref(_method) = em_UNKNOWN]
				][px::ref(_method_) = qi::_1] >>

				' ' >>

				raw[
				    raw[*(char_-char_(" ?"))][px::ref(_path_) = qi::_1] >>
				    (
				    	(
				    		char_('?') >>
				    		raw[*(char_-' ')][px::ref(_queryString_) = qi::_1]
				    	) |
				    	raw[eps][px::ref(_queryString_) = qi::_1]
				    )
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

		std::pair<size_t, SHeader> hdr;

		MessageIterator iter = _request_.end();
		bool parseResult = parse(iter, endInfinity(),
			raw[
			    *(
					(
						raw[
							g_parserToken
						][
						  px::ref(hdr.second._name_) = qi::_1,
						  px::ref(hdr.first) = px::bind((size_t(*)(const Segment&))&hn::hash<MessageIterator>, qi::_1)] >>

						':' >> *space >>

						(
							raw[
								*(char_-"\r\n")
							][px::ref(hdr.second._value_) = qi::_1] |
							eps[px::ref(hdr.second._value_) = Segment(endInfinity(), endInfinity())]
						)
					)[px::ref(hdr.second._header_) = qi::_1]>>
					lit("\r\n")[
					            px::insert(
					            	px::ref(_headersMap),
					            	px::ref(hdr)
					            )]
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
	bool Request::readBody()
	{
		assert(0);
		return false;
	}

	//////////////////////////////////////////////////////////////
	bool Request::ignoreBody()
	{
		assert(0);
		return false;
	}

	//////////////////////////////////////////////////////////////
	const net::http::Message::Segment &Request::requestLine_() const
	{
		return _requestLine_;
	}

	//////////////////////////////////////////////////////////////
	const EMethod &Request::method() const
	{
		return _method;
	}

	//////////////////////////////////////////////////////////////
	const Request::Segment &Request::method_() const
	{
		return _method_;
	}

	//////////////////////////////////////////////////////////////
	const Version &Request::version() const
	{
		return _version;
	}

	//////////////////////////////////////////////////////////////
	const Request::Segment &Request::version_() const
	{
		return _version_;
	}

	//////////////////////////////////////////////////////////////
	const Request::Segment &Request::uri_() const
	{
		return _uri_;
	}

	//////////////////////////////////////////////////////////////
	const Request::Segment &Request::path_() const
	{
		return _path_;
	}

	//////////////////////////////////////////////////////////////
	const Request::Segment &Request::queryString_() const
	{
		return _queryString_;
	}

	//////////////////////////////////////////////////////////////
	const Request::Segment *Request::header(const HeaderName &name) const
	{
		return header(name.hash);
	}

	//////////////////////////////////////////////////////////////
	const Request::Segment *Request::header(size_t hash) const
	{
		TMHeaders::const_iterator iter = _headersMap.find(hash);
		if(_headersMap.end() == iter)
		{
			return NULL;
		}

		return &iter->second._value_;
	}

	//////////////////////////////////////////////////////////////
	const Request::Segment *Request::header(const std::string &name) const
	{
		return header(hn::hash(name));
	}

	//////////////////////////////////////////////////////////////
	const Request::Segment *Request::header(const char *namez) const
	{
		return header(hn::hash(namez));
	}

	//////////////////////////////////////////////////////////////
	const Request::Segment *Request::header(const char *name, size_t nameSize) const
	{
		return header(hn::hash(name, nameSize));
	}


	//////////////////////////////////////////////////////////////
	bool Request::obtainMoreBuffers(bool force)
	{
		if(!force)
		{
			return false;
		}
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
				pushBuffer(ret.data2NoWait());
				return true;
			}

			assert(!"zero-length data incoming? wtf?");
		}

		assert(!"never here");
		return false;
	}

	//////////////////////////////////////////////////////////////
	ResponsePtr Request::response()
	{
		if(!_response)
		{
			_response.reset(new Response(_server, _channel, this));
			_response->statusCode(esc_200);
			_response->version(_version);
		}

		return _response;
	}

	//////////////////////////////////////////////////////////////
	void Request::reinit()
	{
		MessageIterator lastReadedPos = _request_.end();
		if(lastReadedPos.buffer())
		{
			dropFront(lastReadedPos.absolutePosition());
		}
		else
		{
			dropAll();
		}


		_request_ = Segment();

		_requestLine_ = Segment();
		_method = em_UNKNOWN;
		_method_ = Segment();
		_uri_ = Segment();
		_path_ = Segment();
		_queryString_ = Segment();
		_version = Version();
		_version_ = Segment();
		_headers_ = Segment();
		_headersMap.clear();

		_body_ = Segment();
		_response.reset();

	}


}}}}
