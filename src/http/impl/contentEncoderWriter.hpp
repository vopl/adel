#ifndef _HTTP_IMPL_CONTENTENCODERWRITER_HPP_
#define _HTTP_IMPL_CONTENTENCODERWRITER_HPP_

#include "http/impl/contentEncoder.hpp"
#include "net/channel.hpp"

namespace http { namespace impl
{
	class ContentEncoderWriter
		: public ContentEncoder
	{
	public:
		ContentEncoderWriter(const net::Channel &channel, size_t granula);
		virtual ~ContentEncoderWriter();
		
		virtual boost::system::error_code push(const net::Packet &packet, size_t offset=0);
		virtual boost::system::error_code flush();

	protected:
		net::Channel	_channel;
		size_t			_granula;

		std::vector<std::pair<const char *, size_t> >	_buffers;
		std::vector<net::Packet>						_packets;
		size_t											_dataSize;
	};
	typedef boost::shared_ptr<ContentEncoderWriter> ContentEncoderWriterPtr;
}}
#endif
