#include "pch.hpp"

#include "utils/options.hpp"
#include "net/http/server/request.hpp"

#include <boost/filesystem.hpp>


namespace net { namespace http { namespace server { namespace impl
{

	class HandlerFs
	{
	public:
		static utils::OptionsPtr prepareOptions(const char *prefix);
		HandlerFs(utils::OptionsPtr options);
		~HandlerFs();

		void onRequest(net::http::server::Request r);

	private:
		boost::filesystem::path	_root;

	private:
		void notFound(net::http::server::Request r, const boost::filesystem::path &uri);

	};

}}}}
