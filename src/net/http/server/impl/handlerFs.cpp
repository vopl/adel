#include "pch.hpp"
#include "net/http/server/impl/handlerFs.hpp"
#include "net/http/server/log.hpp"
#include "net/http/server/request.hpp"

#include <algorithm>

namespace net { namespace http { namespace server { namespace impl
{
	namespace po = boost::program_options;
	using namespace boost::filesystem;

	//////////////////////////////////////////////////////////////////////////
	utils::OptionsPtr HandlerFs::prepareOptions(const char *prefix)
	{
		utils::OptionsPtr options(new utils::Options(prefix));

		options->addOption(
			"root",
			po::value<std::string>()->default_value("../statics"),
			"directory with static files");

		return options;
	}

	//////////////////////////////////////////////////////////////////////////
	HandlerFs::HandlerFs(utils::OptionsPtr options)
	{
		utils::Options &o = *options;

		_root = o["root"].as<std::string>();

		_root = absolute(_root);
		while(is_symlink(_root))
		{
			_root = read_symlink(_root);
		}

		if(is_directory(_root))
		{
			_root = canonical(_root);
			ILOG("HandlerFs: root="<<_root);
		}
		else
		{
			ELOG("HandlerFs: root is not a directory ("<<_root<<")");
		}
	}

	//////////////////////////////////////////////////////////////////////////
	HandlerFs::~HandlerFs()
	{
		//assert(0);
	}

	//////////////////////////////////////////////////////////////////////////
	void HandlerFs::onRequest(net::http::server::Request r)
	{
		path uri = std::string(r.uri_().begin(), r.uri_().end());

		{
			path::iterator iter = uri.begin();
			path::iterator end = uri.end();

			for(; iter!=end; ++iter)
			{
				if(*iter == "..")
				{
					return notFound(r, uri);
				}
			}
		}

		path p = _root / uri;

		while(is_symlink(p))
		{
			p = read_symlink(p);
		}

		if(!is_regular_file(p))
		{
			return notFound(r, uri);
		}

		std::ifstream ifstr(p.native().c_str(), std::ios::in|std::ios::binary);
		if(!ifstr)
		{
			return notFound(r, uri);
		}

		net::http::server::Response response = r.response();
		response
			.statusCode(esc_200)
			.header("Content-Type: text/plain");
		//TODO: mime types
		/*
		 *
		 */

		//TODO: setup headers
		//TODO: header classes
		/*???
		 * response.header(net::http::server::response::ContentType(220))
		 */

		ifstr.seekg(0, std::ios::end);
		size_t size = ifstr.tellg();
		ifstr.seekg(0, std::ios::beg);

		{
			std::vector<char> buffer(std::min(size, (size_t)1024));
			while(size)
			{
				size_t rsize = std::min(size, (size_t)1024);
				ifstr.read(&buffer[0], rsize);
				response.body(&buffer[0], rsize);
				size -= rsize;
			}
		}
		ifstr.close();

		response.flush();
	}

	/////////////////////////////////////////////////////////////////////////
	void HandlerFs::notFound(net::http::server::Request r, const path &uri)
	{
		net::http::server::Response response = r.response();
		response.statusCode(esc_404);
		response.flush();
	}


}}}}
