#include "pch.hpp"
#include "spider/url.hpp"

namespace spider
{
	//////////////////////////////////////////////////////////////////
	Url::Url()
	{
	}

	//////////////////////////////////////////////////////////////////
	Url::Url(const Url &from)
		: _scheme(from._scheme)
		, _host(from._host)
		, _port(from._port)
		, _path(from._path)
		, _file(from._file)
		, _qs(from._qs)
	{
	}

	//////////////////////////////////////////////////////////////////
	Url::~Url()
	{

	}

	//////////////////////////////////////////////////////////////////
	std::string Url::string()
	{
		std::string res;

		if(!_scheme.empty())
		{
			res += _scheme;
			res += ":";
		}

		if(!_host.empty())
		{
			res += "//";
			res += _host;
		}

		if(!_port.empty())
		{
			res += ":";
			res += _port;
		}

		if(!_path.empty())
		{
			res += escapePath(_path);
		}

		if(!_file.empty())
		{
			res += escapePath(_file);
		}

		if(!_qs.empty())
		{
			res += "?";
			res += escapePath(_qs);
		}

		return res;
	}

	//////////////////////////////////////////////////////////////////
	void Url::combine(const Url &base)
	{
		if(_scheme.empty())
		{
			_scheme = base._scheme;
		}
		else
		{
			return;
		}

		if(_host.empty())
		{
			_host = base._host;
			_port = base._port;
		}
		else
		{
			return;
		}

		if(_path.empty())
		{
			//empty, give from base
			_path = base._path;
		}
		else if(_path[0] == '/')
		{
			//absolute, keep
		}
		else
		{
			//relative, append to base
			_path = base._path + _path;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	namespace
	{
		char *hexChar(char c, char *res)
		{
			unsigned char c1 = (c >> 4) & 0xf;
			unsigned char c2 = c & 0xf;
			res[0] = '%';
			res[1] = c1<=9?('0'+c1):('A'+c1-10);
			res[2] = c2<=9?('0'+c2):('A'+c2-10);

			return res;
		}
	}
	//////////////////////////////////////////////////////////////////////////
	std::string Url::escapePath(const std::string &s)
	{
		char hd[4] = {};
		std::string res;
		for(size_t i(0); i<s.size(); i++)
		{
			const unsigned char c = s[i];

			if(c<32 || c>127)
			{
				res += hexChar(c, hd);
			}
			else
			{
				res += c;
			}
		}

		return res;
	}

}
