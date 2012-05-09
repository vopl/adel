#include "pch.hpp"

#include "utils/options.hpp"
#include "net/http/server/request.hpp"
#include "utils/variant.hpp"

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
		bool	_allowETag;
		bool	_allowLastModified;

	private:
		struct ExtInfo
		{
			std::string _mimeType;
			int _level;
			size_t _minSize;

			ExtInfo()
				: _mimeType("application/octet-stream")
				, _level(1)
				, _minSize(128)
			{}
		};
		typedef std::map<boost::filesystem::path, ExtInfo> Ext2Info;

		ExtInfo _defaultExtInfo;
		Ext2Info _ext2Info;

	private:
		void notFound(net::http::server::Request &r, const boost::filesystem::path &uri);
		void notModified(net::http::server::Request &r, const boost::filesystem::path &uri);
		void loadConf(const std::string &fname);
		ExtInfo parseCompress(const utils::Variant &v, const ExtInfo &dflt);

		const ExtInfo &getExtInfo(const boost::filesystem::path &ext);

	};

}}}}
