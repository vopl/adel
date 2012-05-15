#ifndef _HTTP_IMPL_CONTENTFILTERCHANNELWRITER_HPP_
#define _HTTP_IMPL_CONTENTFILTERCHANNELWRITER_HPP_

#include "http/impl/contentFilter.hpp"
#include "net/channel.hpp"

namespace http { namespace impl
{
	class ContentFilterChannelWriter
		: public ContentFilter
	{
	public:
		ContentFilterChannelWriter(const net::Channel &channel, size_t granula);
		virtual ~ContentFilterChannelWriter();
		
		virtual boost::system::error_code filterPush(const net::Packet &packet, size_t offset=0);
		virtual boost::system::error_code filterFlush();

	protected:
		net::Channel	_channel;
		size_t	_granula;

		std::vector<std::pair<const char *, size_t> >	_buffers;
		std::vector<net::Packet>								_packets;
		size_t											_dataSize;
	};
	typedef boost::shared_ptr<ContentFilterChannelWriter> ContentFilterChannelWriterPtr;
}}
#endif
