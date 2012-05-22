#include "pch.hpp"

#include "http/impl/bodyExtractorChunked.hpp"
#include "http/error.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_string.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_uint.hpp>

#include <boost/spirit/include/phoenix_core.hpp>

namespace http { namespace impl{

	/////////////////////////////////////////////////////////////////////////////////////////////////
	BodyExtractorChunked::BodyExtractorChunked(const ContentDecoderPtr &bodyDecoder, ContentDecoderPtr tailDecoder)
		: BodyExtractor(bodyDecoder, tailDecoder)
		, _es(es_caption)
		, _bodySize(0)
	{
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	BodyExtractorChunked::~BodyExtractorChunked()
	{
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	boost::system::error_code BodyExtractorChunked::push(const net::Packet &p, size_t offset)
	{
		switch(_es)
		{
		case es_caption:
			return pushCaption(p, offset);
		case es_body:
			return pushBody(p, offset);
		case es_trailerHeader:
			return pushTrailerHeader(p, offset);
		case es_done:
			return _tailDecoder->push(p, offset);
		case es_error:
			return http::error::make(http::error::unexpected);
		default:
			assert(0);
			return http::error::make(http::error::unexpected);
		}
		return http::error::make(http::error::unexpected);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	boost::system::error_code BodyExtractorChunked::pushCaption(const net::Packet &p, size_t offset)
	{
		assert(es_caption == _es);

		boost::system::error_code ec;

		if((ec = _accumuler.push(p, offset)))
		{
			return ec;
		}

		namespace qi = boost::spirit::qi;
		using namespace qi;
		namespace px = boost::phoenix;

		http::InputMessage::Iterator begin = _accumuler.begin();
		http::InputMessage::Iterator end = _accumuler.end();

		bool error = false;
		bool res = qi::parse(begin, end,
			-lit("\r\n") >> // <- это терминатор предыдущего тела, в первом чанке его может не быть
			(uint_parser<size_t, 16>()[px::ref(_bodySize) = qi::_1] || (eps[px::ref(error) = true] >> !eps))>> //надо обязательно цифры в начале чанка
			*(char_-'\r') >>
			"\r\n");

		if(!res)
		{
			if(error || _accumuler.size() > 1024)
			{
				_es = es_error;
				return http::error::make(http::error::invalid_message);
			}

			//начало чанка не распознано но еще есть шанс, оставляю акумулятор
			return http::error::make();
		}

		//начало чанка распознано

		//_bodySize valid
		if(!_bodySize)
		{
			_es = es_trailerHeader;
		}
		else
		{
			_es = es_body;
		}

		_accumuler.dropFront(begin);
		return pushFromAccumuler();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	boost::system::error_code BodyExtractorChunked::pushBody(const net::Packet &p, size_t offset)
	{
		assert(es_body == _es);
		assert(offset < p._size);
		boost::system::error_code ec;

		size_t incomingSize = p._size - offset;

		if(incomingSize <= _bodySize)
		{
			if((ec = _bodyDecoder->push(p, offset)))
			{
				return ec;
			}

			_bodySize -= incomingSize;
			if(!_bodySize)
			{
				_es = es_caption;
			}
			return http::error::make();
		}

		//incomingSize > _bodySize

		//сколько осталось тела - вылить в декодер тела, остальное парсить как новый чанк
		net::Packet p2(p);
		p2._size -= incomingSize - _bodySize;
		assert(offset < p2._size);
		if((ec = _bodyDecoder->push(p2, offset)))
		{
			return ec;
		}

		_es = es_caption;
		size_t offset2 = offset + _bodySize;
		return push(p, offset2);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	boost::system::error_code BodyExtractorChunked::pushTrailerHeader(const net::Packet &p, size_t offset)
	{
		assert(es_trailerHeader == _es);

		boost::system::error_code ec;

		if((ec = _accumuler.push(p, offset)))
		{
			return ec;
		}

		namespace qi = boost::spirit::qi;
		using namespace qi;
		namespace px = boost::phoenix;

		http::InputMessage::Iterator begin = _accumuler.begin();
		http::InputMessage::Iterator end = _accumuler.end();

		bool headerPresent = false;
		bool res = qi::parse(begin, end,
			((+(char_-'\r'))[px::ref(headerPresent)] || eps) >>
			"\r\n");

		if(!res)
		{
			if(_accumuler.size() > 1024)
			{
				_es = es_error;
				return http::error::make(http::error::invalid_message);
			}

			//не распознано но еще есть шанс, оставляю акумулятор
			return http::error::make();
		}

		//распознано

		//_bodySize valid
		if(headerPresent)
		{
			_es = es_trailerHeader;
		}
		else
		{
			_es = es_done;
		}

		_accumuler.dropFront(begin);
		return pushFromAccumuler();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	bool BodyExtractorChunked::isDone()
	{
		return es_done == _es;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	boost::system::error_code BodyExtractorChunked::pushFromAccumuler()
	{
		boost::system::error_code ec;
		if(_accumuler.size())
		{
			InputMessageBufferPtr buf = _accumuler.begin().buffer()->shared_from_this();
			_accumuler = ContentDecoderAccumuler();

			if(buf)
			{
				for(;;)
				{
					size_t offset2;
					net::Packet p2 = buf->asPacket(offset2);
					if((ec = push(p2, offset2)))
					{
						return ec;
					}
					if(!buf->next())
					{
						break;
					}
					buf = buf->next()->shared_from_this();
				}
			}
		}

		return http::error::make();
	}

}}
