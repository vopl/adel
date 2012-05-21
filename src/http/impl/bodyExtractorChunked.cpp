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
		, _chunkSize(_badChunkSize)
		, _crFound(false)
	{
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	BodyExtractorChunked::~BodyExtractorChunked()
	{
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	boost::system::error_code BodyExtractorChunked::read(const ContentDecoderAccumulerPtr &from, const http::InputMessage::Iterator &begin)
	{
		impl::InputMessageBuffer *buf = begin.buffer();
		const char *pos = begin.position();

		if(pos == buf->end())
		{
			buf = buf->next();
			if(buf)
			{
				pos = buf->begin();
			}
		}

		boost::system::error_code ec;
		if(buf)
		{
			size_t offset;
			net::Packet p = buf->asPacket(offset);
			if((ec = push(p, offset)))
			{
				return ec;
			}

			buf = buf->next();

			while(buf)
			{
				size_t offset;
				net::Packet p = buf->asPacket(offset);

				if((ec = push(p, offset)))
				{
					return ec;
				}

				buf = buf->next();
			}
		}

		return http::error::make();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	boost::system::error_code BodyExtractorChunked::read(net::Channel channel, size_t granula)
	{
		boost::system::error_code ec;
		while(!isDone())
		{
			async::Future2<boost::system::error_code, net::Packet> res =
				channel.receive(granula);
			res.wait();
			if(res.data1NoWait())
			{
				return res.data1NoWait();
			}
			if((ec = push(res.data2NoWait(), 0)))
			{
				return ec;
			}
		}

		return http::error::make();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	boost::system::error_code BodyExtractorChunked::flush()
	{
		assert(0);
		return boost::system::error_code();
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
		case es_header:
			return pushHeader(p, offset);
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


		http::InputMessage::Iterator begin = _accumuler.begin();
		http::InputMessage::Iterator end = _accumuler.end();

		bool res = qi::parse(begin, end,
			uint_parser<size_t, 16>()[px::ref(_bodySize) = qi::_1] >>
			*(char_-'\r') >>
			string_("\r\n"));

		if(!res)
		{
			if(_accumuler.size() > 1024)
			{
				_es = es_error;
				return http::error::make(http::error::invalid_message);
			}

			//начало чанка не распознано но еще есть шанс, оставляю акумулятор
			return http::error::make();
		}

		//начало чанка распознано
		_es = es_body;
		//_bodySize valid

		_accumuler.dropFront(begin);

		InputMessageBuffer *buf = _accumuler.begin().buffer();
		while(buf)
		{
			size_t offset2;
			net::Packet p2 = buf->asPacket(offset2);
			if((ec = pushBody(p2, offset2)))
			{
				return ec;
			}
			buf = buf->next();
		}

		_accumuler = ContentDecoderAccumuler();

		return http::error::make();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	boost::system::error_code BodyExtractorChunked::pushBody(const net::Packet &p, size_t offset)
	{
		assert(0);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	boost::system::error_code BodyExtractorChunked::pushHeader(const net::Packet &p, size_t offset)
	{
		assert(0);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	bool BodyExtractorChunked::isDone()
	{
		return es_done == _es;
	}

}}
