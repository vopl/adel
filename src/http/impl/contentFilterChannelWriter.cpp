#include "pch.hpp"
#include "http/impl/contentFilterChannelWriter.hpp"
#include "http/error.hpp"


namespace http { namespace impl
{
	//////////////////////////////////////////////////////////////////////////////
	ContentFilterChannelWriter::ContentFilterChannelWriter(const net::Channel &channel, size_t granula)
		: _channel(channel)
		, _granula(granula)
		, _dataSize(0)
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	ContentFilterChannelWriter::~ContentFilterChannelWriter()
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	boost::system::error_code ContentFilterChannelWriter::filterPush(const net::Packet &packet, size_t offset)
	{
		assert(packet._size > offset);
		size_t bufferSize = packet._size - offset;

		_buffers.push_back(std::make_pair(
			packet._data.get()+offset,
			bufferSize));
		_dataSize += bufferSize;
		_packets.push_back(packet);

		if(_dataSize >= _granula)
		{
			return filterFlush();
		}
		return http::error::make();
	}
	
	//////////////////////////////////////////////////////////////////////////////
	boost::system::error_code ContentFilterChannelWriter::filterFlush()
	{
		if(_dataSize)
		{
			std::vector<std::pair<const char *, size_t> >	buffers;
			std::vector<net::Packet>								packets;

			buffers.swap(_buffers);
			packets.swap(_packets);
			_dataSize = 0;

			return _channel.send(buffers, packets).data();
		}

		return http::error::make();
	}

}}
