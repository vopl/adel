#include "pch.hpp"

#include "http/impl/bodyExtractor.hpp"
#include "http/error.hpp"

namespace http { namespace impl{

	BodyExtractor::BodyExtractor(const ContentDecoderPtr &bodyDecoder, ContentDecoderPtr tailDecoder)
		: _bodyDecoder(bodyDecoder)
		, _tailDecoder(tailDecoder)
	{
	}

	BodyExtractor::~BodyExtractor()
	{
	}

	boost::system::error_code BodyExtractor::read(const ContentDecoderAccumulerPtr &from, const http::InputMessage::Iterator &begin)
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

	boost::system::error_code BodyExtractor::read(net::Channel channel, size_t granula)
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

	boost::system::error_code BodyExtractor::flush()
	{
		boost::system::error_code ec;

		if((ec = _bodyDecoder->flush()))
		{
			return ec;
		}
		if((ec = _tailDecoder->flush()))
		{
			return ec;
		}
		return http::error::make();
	}

}}
