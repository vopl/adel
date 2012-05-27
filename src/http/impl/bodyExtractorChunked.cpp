#include "pch.hpp"

#include "http/impl/bodyExtractorChunked.hpp"
#include "http/error.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_string.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_uint.hpp>

#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

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
	bool BodyExtractorChunked::isDone()
	{
		return es_done == _es;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	boost::system::error_code BodyExtractorChunked::process(ContentDecoderAccumuler &data)
	{
		switch(_es)
		{
		case es_caption:
			return processCaption(data);
		case es_body:
		{
			boost::system::error_code ec;
			if((ec = processBody(data, _bodySize)))
			{
				return ec;
			}
			if(!_bodySize)
			{
				_es = es_caption;
			}
			return http::error::make();
		}
		case es_trailerHeader:
			return processTrailerHeader(data);
		case es_done:
			return processTail(data);
		case es_error:
			return http::error::make(http::error::unexpected);
		default:
			assert(0);
			return http::error::make(http::error::unexpected);
		}
		return http::error::make(http::error::unexpected);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	boost::system::error_code BodyExtractorChunked::processCaption(ContentDecoderAccumuler &data)
	{
		assert(es_caption == _es);

		boost::system::error_code ec;

		namespace qi = boost::spirit::qi;
		using namespace qi;
		namespace px = boost::phoenix;

		http::InputMessage::Iterator begin = data.begin();
		http::InputMessage::Iterator end = data.end();

		bool error = false;

		qi::uint_parser<size_t, 16> bodySizeParser;
		bool res = qi::parse(begin, end,
			-lit("\r\n") >> // <- это терминатор предыдущего тела, в первом чанке его может не быть
			(
				bodySizeParser[px::ref(_bodySize) = qi::_1] |
				eoi |
				(qi::eps[px::ref(error) = true] >> !qi::eps) //надо обязательно цифры в начале чанка
			)>> 
			*(char_-'\r') >>
			"\r\n");

		if(!res)
		{
			if(error || data.size() > 1024)
			{
				_es = es_error;
				return http::error::make(http::error::bad_message);
			}

			//начало чанка не распознано но еще есть шанс, оставляю акумулятор
			return http::error::make(http::error::need_more_data);
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

		data.dropFront(begin);
		return http::error::make();
	}


	/////////////////////////////////////////////////////////////////////////////////////////////////
	boost::system::error_code BodyExtractorChunked::processTrailerHeader(ContentDecoderAccumuler &data)
	{
		assert(es_trailerHeader == _es);

		boost::system::error_code ec;

		namespace qi = boost::spirit::qi;
		using namespace qi;
		namespace px = boost::phoenix;

		http::InputMessage::Iterator begin = data.begin();
		http::InputMessage::Iterator end = data.end();

		bool headerPresent = false;
		bool res = qi::parse(begin, end,
			((+(char_-'\r'))[px::ref(headerPresent)=true] | eps) >>
			"\r\n");

		if(!res)
		{
			if(data.size() > 1024)
			{
				_es = es_error;
				return http::error::make(http::error::bad_message);
			}

			//не распознано но еще есть шанс, оставляю акумулятор
			return http::error::make(http::error::need_more_data);
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

		data.dropFront(begin);
		return http::error::make();
	}

}}
