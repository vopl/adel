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
	std::string Url::generate()
	{
		std::string res;

		if(!_scheme.empty())
		{
			res += _scheme;
			res += "://";
		}

		if(!_host.empty())
		{
			res += _host;
		}

		if(!_port.empty())
		{
			res += ":";
			res += _port;
		}

		if(!_path.empty())
		{
			res += _path;
		}

		if(!_file.empty())
		{
			res += _file;
		}

		if(!_qs.empty())
		{
			res += "?";
			res += _qs;
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

		if(_host.empty())
		{
			_host = base._host;
		}

		if(_port.empty())
		{
			_port = base._port;
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
}
