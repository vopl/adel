#include "pch.hpp"
#include "http/server/impl/handlerFs.hpp"
#include "http/server/log.hpp"
#include "http/server/request.hpp"
#include "http/headerValue.hpp"

#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/karma_string.hpp>
#include <boost/spirit/include/karma_char.hpp>

#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_container.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef _MSC_VER
#	include <io.h>
#endif
#include <stddef.h>
#include <algorithm>

namespace http { namespace server { namespace impl
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

		options->addOption(
			"allowETag",
			po::value<bool>()->default_value(false),
			"use ETag, If-Match and If-None-Match header fields for entities");

		options->addOption(
			"allowLastModified",
			po::value<bool>()->default_value(true),
			"use Last-Modified, If-Modified-Since and If-Unmodified-Since header fields for entities");

		return options;
	}

	//////////////////////////////////////////////////////////////////////////
	HandlerFs::HandlerFs(utils::OptionsPtr options)
	{
		utils::Options &o = *options;

		_root = o["root"].as<std::string>();
		_allowETag = o["allowETag"].as<bool>();
		_allowLastModified = o["allowLastModified"].as<bool>();

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
	void HandlerFs::onRequest(http::server::Request &r)
	{
		path originalPath = std::string(r.path().begin(), r.path().end());

		{
			path::iterator iter = originalPath.begin();
			path::iterator end = originalPath.end();

			for(; iter!=end; ++iter)
			{
				if(".." == *iter)
				{
					return notFound(r, originalPath);
				}
			}
		}


		path p = _root / originalPath;

		struct stat st;
		if(stat(p.string().c_str(), &st))
		{
			return notFound(r, originalPath);
		}

		if(S_IFREG != (st.st_mode & S_IFMT))
		{
			return notFound(r, originalPath);
		}

		std::string etag;
		if(_allowETag)
		{
			using namespace boost::spirit::karma;
			namespace karma = boost::spirit::karma;
			namespace px = boost::phoenix;

			std::back_insert_iterator<std::string> out(etag);
			bool gres = generate(out,
				karma::lit('"') <<
				int_generator<time_t, 16>()[karma::_1 = st.st_mtime] <<
				'-' <<
				int_generator<off_t, 16>()[karma::_1 = st.st_size] <<
				'-' <<
				uint_generator<ino_t, 16>()[karma::_1 = st.st_ino] <<
				'"');
			assert(gres);
			(void)gres;

			const InputMessage::Segment *seg = r.header(hn::ifNoneMatch);
			if(	seg &&
				std::string(seg->begin(), seg->end()) == etag)
			{
				return notModified(r, originalPath);
			}
		}

		if(_allowLastModified)
		{
			HeaderValue<Date> ims(r.header(hn::ifModifiedSince));
			if(ims.isCorrect() && ims.value() >= st.st_mtime)
			{
				return notModified(r, originalPath);
			}
		}

		int fd = -1;

		if(st.st_size)
		{
			fd = open(p.string().c_str(), O_RDONLY
#ifdef _MSC_VER
				|O_BINARY
#endif
#pragma warning (suppress: 4996)
				);
			if(!fd)
			{
				return notFound(r, originalPath);
			}
		}

		const ExtInfo &extInfo = getExtInfo(originalPath.extension());

		http::server::Response response = r.response();
		if(response.firstLine(esc_200)) return;
		if(response.header(hn::contentType, extInfo._mimeType)) return;

		if(_allowETag)
		{
			if(response.header(hn::eTag, etag))
			{
				//connection lost?
				return;
			}
		}
		if(_allowLastModified)
		{
			if(response.header(hn::lastModified, HeaderValue<Date>(st.st_mtime)))
			{
				//connection lost?
				return;
			}
		}

		response.setContentLength(st.st_size);

		if(extInfo._level>0 && st.st_size && st.st_size >= extInfo._minSize)
		{
			response.setContentCompress(extInfo._level);
		}
		else
		{
			response.setContentCompress(0);
		}

		if(fd>=0)
		{
			http::server::Response::Iterator iter = response.bodyIterator();
			size_t size = st.st_size;
			while(size)
			{
				size_t bufSize = size;
				char *buf = iter.bufferGet(bufSize);
				assert(buf && bufSize);

#pragma warning (suppress: 4996)
				int rres = read(fd, buf, (off_t)bufSize);
				(void)rres;

				if(iter.bufferInc(bufSize))
				{
					//connection lost?
					//assert(0);
					break;
				}
				size -= bufSize;
			}

#pragma warning (suppress: 4996)
			close(fd);
		}

		response.bodyFlush();
	}

	/////////////////////////////////////////////////////////////////////////
	void HandlerFs::notFound(http::server::Request &r, const path &uri)
	{
		http::server::Response response = r.response();
		if(response.firstLine(esc_404))
		{
			return;
		}
		response.setContentLength(0);
		if(response.bodyFlush())
		{
			return;
		}
	}

	/////////////////////////////////////////////////////////////////////////
	void HandlerFs::notModified(http::server::Request &r, const path &uri)
	{
		http::server::Response response = r.response();
		if(response.firstLine(esc_304))
		{
			return;
		}
		response.setContentLength(0);
		if(response.bodyFlush())
		{
			return;
		}
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
		if(!v.load(p.string().c_str(), &err))
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
				res._level = v["level"].to<int>();
			}
			if(v["minSize"].isScalar())
			{
				res._minSize = v["minSize"].to<size_t>();
			}
		}
		else
		{
			res._level = v.to<int>();
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

}}}
