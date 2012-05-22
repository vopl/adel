#include "pch.hpp"

#include "http/impl/bodyExtractor.hpp"
#include "http/error.hpp"

namespace http { namespace impl{

	////////////////////////////////////////////////////////////////////////
	BodyExtractor::BodyExtractor(const ContentDecoderPtr &bodyDecoder, ContentDecoderPtr tailDecoder)
		: _bodyDecoder(bodyDecoder)
		, _tailDecoder(tailDecoder)
	{
	}

	////////////////////////////////////////////////////////////////////////
	BodyExtractor::~BodyExtractor()
	{
	}

	////////////////////////////////////////////////////////////////////////
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
			offset += pos - buf->begin();
			if((ec = _stream.push(p, offset)))
			{
				return ec;
			}

			buf = buf->next();

			while(buf)
			{
				size_t offset;
				net::Packet p = buf->asPacket(offset);

				if((ec = _stream.push(p, offset)))
				{
					return ec;
				}

				buf = buf->next();
			}
		}
		from->dropTail(begin);

		return process();
	}

	////////////////////////////////////////////////////////////////////////
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
			if((ec = _stream.push(res.data2NoWait(), 0)))
			{
				return ec;
			}

			if((ec = process()))
			{
				return ec;
			}
		}

		//
		assert(!_stream.size());

		return http::error::make();
	}

	////////////////////////////////////////////////////////////////////////
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

	////////////////////////////////////////////////////////////////////////
	boost::system::error_code BodyExtractor::process()
	{
		boost::system::error_code ec;

		while(_stream.size())
		{
#ifndef NDEBUG
			size_t streamSize = _stream.size();
#endif
			ec = this->process(_stream);

			if(ec)
			{
				if(http::error::need_more_data == ec)
				{
					return http::error::make();
				}

				return ec;
			}
			assert(streamSize > _stream.size());
		}

		return http::error::make();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	boost::system::error_code BodyExtractor::processBody(ContentDecoderAccumuler &data, size_t &bodySize)
	{
		boost::system::error_code ec;

		assert(data.size());

		size_t offset;
		net::Packet packet = data.firstBuffer()->asPacket(offset);
		size_t packetSize = packet._size - offset;

		if(packetSize <= bodySize)
		{
			if((ec = _bodyDecoder->push(packet, offset)))
			{
				return ec;
			}

			bodySize -= packetSize;
			data.dropFront(data.begin()+packetSize);
			return http::error::make();
		}

		//incomingSize > _bodySize

		//сколько осталось тела - вылить в декодер тела, остальное парсить как новый чанк
		net::Packet p2(packet);
		p2._size -= packetSize - bodySize;
		assert(offset < p2._size);
		if((ec = _bodyDecoder->push(p2, offset)))
		{
			return ec;
		}

		data.dropFront(data.begin()+bodySize);

		bodySize = 0;
		return http::error::make();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	boost::system::error_code BodyExtractor::processTail(ContentDecoderAccumuler &data)
	{
		size_t offset;
		net::Packet packet = data.firstBuffer()->asPacket(offset);

		boost::system::error_code ec;
		if((ec = _tailDecoder->push(packet, offset)))
		{
			return ec;
		}

		data.dropFront(data.begin()+ (packet._size-offset));
		return http::error::make();
	}

}}
