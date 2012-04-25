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

		options->addOption(
			"conf",
			po::value<std::string>()->default_value("../etc/default.hfs"),
			"json conf file, contain info about mime types and compression settings");

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

		loadConf(o["conf"].as<std::string>());
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

		path p = _root / uri;

		boost::system::error_code ec;
		p = canonical(p, ec);
		if(ec)
		{
			return notFound(r, uri);
		}

		{
			path::iterator piter = p.begin();
			path::iterator pend = p.end();

			path::iterator riter = _root.begin();
			path::iterator rend = _root.end();

			while(riter!=rend)
			{
				if(piter==pend || *piter!=*riter)
				{
					return notFound(r, uri);
				}
				++piter;
				++riter;
			}
		}

		if(!is_regular_file(p, ec) || ec)
		{
			return notFound(r, uri);
		}

		//TODO: etag, expires
		/*
		 * ETag
		 * 		If-Match, If-None-Match,
		 *
		 * Last-Modified
		 * 		If-Modified-Since, If-Unmodified-Since
		 */

		std::ifstream ifstr(p.native().c_str(), std::ios::in|std::ios::binary);
		if(!ifstr)
		{
			return notFound(r, uri);
		}
		ifstr.seekg(0, std::ios::end);
		size_t size = ifstr.tellg();
		ifstr.seekg(0, std::ios::beg);

		const ExtInfo &extInfo = getExtInfo(uri.extension());

		net::http::server::Response response = r.response();
		response
			.statusCode(esc_200)
			.header("Content-Type: "+extInfo._mimeType);

		response.setBodySize(size);

		if(extInfo._level>0 && size >= extInfo._minSize)
		{
			response.setBodyCompress(extInfo._level, extInfo._buffer);
		}
		else
		{
			response.setBodyCompress(0);
		}

		if(size)
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

	/////////////////////////////////////////////////////////////////////////
	void HandlerFs::loadConf(const std::string &fname)
	{
		path p = fname;
		while(is_symlink(p))
		{
			p = read_symlink(p);
		}

		if(!is_regular_file(p))
		{
			ELOG("HandlerFs: conf file not found ("<<p<<")");
			return;
		}

		utils::Variant v;

		std::string err;
		if(!v.load(p.c_str(), &err))
		{
			ELOG("HandlerFs: unable to load conf file ("<<p<<"): "<<err);
			return;
		}

		if(!v.is<utils::Variant::MapStringVariant>())
		{
			ELOG("HandlerFs: unable to load conf file ("<<p<<"): "<<"bad format");
			return;
		}

		if(!v["compress"].isNull())
		{
			utils::Variant compress = v["compress"];
			if(!compress.is<utils::Variant::MapStringVariant>())
			{
				ELOG("HandlerFs: wrong conf file ("<<p<<"): "<<"compress must be a map");
				return;
			}

			if(compress["default"])
			{
				_defaultExtInfo = parseCompress(compress["default"], _defaultExtInfo);
			}

			BOOST_FOREACH(utils::Variant::MapStringVariant::value_type &prm, compress.as<utils::Variant::MapStringVariant>())
			{
				if(prm.first == "default")
				{
					continue;
				}

				_ext2Info['.'+prm.first] = parseCompress(prm.second, _defaultExtInfo);
			}
		}

		if(!v["mimeTypes"].isNull())
		{
			utils::Variant mimeTypes = v["mimeTypes"];
			if(!mimeTypes.is<utils::Variant::MapStringVariant>())
			{
				ELOG("HandlerFs: wrong conf file ("<<p<<"): "<<"mimeTypes must be a map");
				return;
			}


			BOOST_FOREACH(utils::Variant::MapStringVariant::value_type &prm, mimeTypes.as<utils::Variant::MapStringVariant>())
			{
				if(prm.second.isArray())
				{
					BOOST_FOREACH(utils::Variant &mt, prm.second.as<utils::Variant::DequeVariant>(true))
					{
						std::string smt = '.'+mt.as<std::string>(true);
						if(_ext2Info.end() == _ext2Info.find(smt))
						{
							_ext2Info[smt] = _defaultExtInfo;
						}
						_ext2Info[smt]._mimeType = prm.first;
					}
				}
				else
				{
					std::string smt = '.'+prm.second.as<std::string>(true);
					if(_ext2Info.end() == _ext2Info.find(smt))
					{
						_ext2Info[smt] = _defaultExtInfo;
					}
					_ext2Info[smt]._mimeType = prm.first;
				}
			}
		}
		BOOST_FOREACH(utils::Variant::MapStringVariant::value_type &pr, v.as<utils::Variant::MapStringVariant>())
		{
			if(pr.first == "mimeTypes")
			{
				continue;
			}
			else if(pr.first == "compress")
			{
				continue;
			}
			else
			{
				WLOG("HandlerFs: wrong conf file ("<<p<<"): excess "<<pr.first);
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////
	HandlerFs::ExtInfo HandlerFs::parseCompress(const utils::Variant &v, const ExtInfo &dflt)
	{
		ExtInfo res = dflt;

		if(v.is<utils::Variant::MapStringVariant>())
		{
			if(v["level"].isScalar())
			{
				res._level = v["level"].to<size_t>();
			}
			if(v["buffer"].isScalar())
			{
				res._buffer = v["buffer"].to<size_t>();
			}
			if(v["minSize"].isScalar())
			{
				res._minSize = v["minSize"].to<size_t>();
			}
		}
		else
		{
			res._level = v.to<size_t>();
		}

		return res;
	}

	/////////////////////////////////////////////////////////////////////////
	const HandlerFs::ExtInfo &HandlerFs::getExtInfo(const path &ext)
	{
		Ext2Info::const_iterator iter = _ext2Info.find(ext);
		if(_ext2Info.end() != iter)
		{
			return iter->second;
		}
		return _defaultExtInfo;
	}

}}}}
