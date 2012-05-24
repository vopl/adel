#ifndef _SPIDER_URL_HPP_
#define _SPIDER_URL_HPP_

#include <string>

#include <boost/range/iterator_range.hpp>
#include <boost/shared_ptr.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_string.hpp>
#include <boost/spirit/include/qi_char.hpp>

#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_core.hpp>

namespace spider
{

	class Url
	{
	public:
		Url();
		Url(const Url &from);

		template <class Iterator>
		Url(const Iterator &begin, const Iterator &end)
		{
			parse(begin, end);
		}

		template <class Container>
		Url(const Container &container)
		{
			parse(container);
		}

		~Url();

	public:
		std::string generate();

		template <class Iterator>
		bool parse(const Iterator &begin, const Iterator &end);

		template <class Container>
		bool parse(const Container &container)
		{
			return parse(container.begin(), container.end());
		}

		void combine(const Url &base);

	public:
		bool		_isValid;
		std::string	_scheme;
		std::string	_host;
		std::string	_port;
		std::string	_path;
		std::string	_file;
		std::string	_qs;
	};
	typedef boost::shared_ptr<Url> UrlPtr;





	/////////////////////////////////////////////////////////////////
	template <class Iterator>
	bool Url::parse(const Iterator &begin, const Iterator &end)
	{
		_isValid = false;
		_scheme.clear();
		_host.clear();
		_port.clear();
		_path.clear();
		_file.clear();
		_qs.clear();

		// [ [[shceme:]//] [host[:port]] ]

		// [/path/[file] ]
		// [?qs]
		// [#anchor]

		namespace qi = boost::spirit::qi;
		using namespace qi;
		namespace px = boost::phoenix;

		boost::iterator_range<Iterator> scheme, host, port, path, file, qs;

		Iterator iter = begin;
		bool b = qi::parse(iter, end,
			//[ [[shceme:]//] [host[:port]] ]
			(
				// [[shceme:]//]
				-(
					//scheme
					-(raw[lit("http") >> -lit('s')][px::ref(scheme)=qi::_1] >> lit(':')) >>

					// '//'
					lit("//")
				) >>

				// [host[:port]] ]
				-(
					raw[+(char_ - char_(":/?#"))][px::ref(host)=qi::_1] >>
					-(lit(':') >> raw[(+char_("0-9"))][px::ref(port)=qi::_1])
				)
			) >>

			// [/path/[file] ]
			(
				-(raw[+(char_ - char_("?#"))][px::ref(path)=qi::_1])
			) >>

			// [?qs]
			(
				-(lit('?') >> raw[*(char_ - char_("#"))][px::ref(qs)=qi::_1])
			)>>

			// [#anchor]
			(
				-(lit('#') >> +char_)
			)
		);

		if(!b)
		{
			return false;
		}

		_isValid = true;
		_scheme.assign(scheme.begin(), scheme.end());
		_host.assign(host.begin(), host.end());
		_port.assign(port.begin(), port.end());
		_path.assign(path.begin(), path.end());
		_file.assign(file.begin(), file.end());
		_qs.assign(qs.begin(), qs.end());

		if(!_path.empty())
		{
			std::string::size_type pos = _path.find_last_of('/');

			if(std::string::npos == pos)
			{
				_file.swap(_path);
			}
			else
			{
				pos += 1;
				_file.assign(_path.begin()+pos, _path.end());
				_path.erase(pos);
			}
		}

		return true;
	}

}

#endif
