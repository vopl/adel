#ifndef _NET_ACCEPTOR_HPP_
#define _NET_ACCEPTOR_HPP_

#include "net/channel.hpp"
#include "async/future.hpp"
#include <boost/signals2.hpp>

namespace net
{
	//////////////////////////////////////////////////////////////////////////
	namespace impl
	{
		class Acceptor;
		typedef boost::shared_ptr<Acceptor> AcceptorPtr;

	}

	//////////////////////////////////////////////////////////////////////////
	class Acceptor
	{
	protected:
		typedef impl::AcceptorPtr ImplPtr;
		ImplPtr	_impl;

	public:
		Acceptor();
		~Acceptor();

		typedef boost::function<void(boost::system::error_code, Channel)> TOnAccept;
		boost::signals2::connection connectOnAccept(const TOnAccept &f);
		async::Future<boost::system::error_code> listen(const char *host, const char *service, bool useSsl=false);
		void unlisten();
	};
}
#endif
