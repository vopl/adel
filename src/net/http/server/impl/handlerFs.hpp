#include "pch.hpp"

#include "utils/options.hpp"
#include "net/http/server/request.hpp"

namespace net { namespace http { namespace server { namespace impl
{

	class HandlerFs
	{
	public:
		static utils::OptionsPtr prepareOptions(const char *prefix);
		HandlerFs(utils::OptionsPtr options);
		~HandlerFs();

		void onRequest(const net::http::server::Request &r);
	};

}}}}
