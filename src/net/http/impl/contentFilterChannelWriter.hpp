#ifndef _NET_HTTP_IMPL_CONTENTFILTERCHANNELWRITER_HPP_
#define _NET_HTTP_IMPL_CONTENTFILTERCHANNELWRITER_HPP_

#include "net/http/impl/contentFilter.hpp"
#include "net/channel.hpp"

namespace net { namespace http { namespace impl
{
	class ContentFilterChannelWriter
		: public ContentFilter
	{
	public:
		ContentFilterChannelWriter(const Channel &channel, size_t granula);
		virtual ~ContentFilterChannelWriter();
		
		virtual bool filterPush(const Packet &packet, size_t offset=0);
		virtual bool filterFlush();

	protected:
		Channel	_channel;
		size_t	_granula;

		std::vector<std::pair<const char *, size_t> >	_buffers;
		std::vector<Packet>								_packets;
		size_t											_dataSize;
	};
	typedef boost::shared_ptr<ContentFilterChannelWriter> ContentFilterChannelWriterPtr;
}}}
#endif
