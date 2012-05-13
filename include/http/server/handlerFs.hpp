#ifndef _HTTP_SERVER_HANDLERFS_HPP_
#define _HTTP_SERVER_HANDLERFS_HPP_

#include "utils/options.hpp"
#include "http/server/request.hpp"

namespace http { namespace server
{
	namespace impl
	{
		class HandlerFs;
		typedef boost::shared_ptr<HandlerFs> HandlerFsPtr;
	}
	
	class HandlerFs
	{
	protected:
		typedef impl::HandlerFsPtr ImplPtr;
		ImplPtr _impl;
		
	public:
		static utils::OptionsPtr prepareOptions(const char *prefix);
		HandlerFs(utils::OptionsPtr options);
		~HandlerFs();
		
		void onRequest(Request r);
	};
}}
#endif
