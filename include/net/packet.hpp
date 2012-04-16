#ifndef _NET_PACKET_HPP_
#define _NET_PACKET_HPP_

#include <boost/cstdint.hpp>
#include <boost/shared_array.hpp>

namespace net
{

	//////////////////////////////////////////////////////////////////////////
	struct Packet
	{
		boost::shared_array<char> _data;
		boost::uint32_t _size;

		Packet()
			: _size(0)
		{
		}

		Packet(boost::shared_array<char> data, boost::uint32_t size)
			: _data(data)
			, _size(size)
		{
		}
	};

}
#endif
