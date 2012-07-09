#ifndef _SCOM_IMPL_SEARCHER_HPP_
#define _SCOM_IMPL_SEARCHER_HPP_

//#include <deque>
#include <vector>
//#include <map>
//#include <boost/unordered_map.hpp>
#include <boost/container/flat_map.hpp>

namespace scom { namespace impl
{
	class Searcher
	{
	public:
		Searcher(const char *str1, size_t size1);
		~Searcher();

		void reset(const char *str2, size_t size2);
		bool next(size_t &pos1, size_t &pos2, size_t &len);

	private:
		const char	*_str1;
		size_t		_size1;
		const char	*_str2;
		size_t		_size2;

		size_t		_vIdx;
		size_t		_pos1;
		size_t		_pos2;

	private:
		//http://e-maxx.ru/algo/suffix_automata

		struct Next
		{
			size_t	_idx;
			size_t	_pos1;

			Next()
				: _idx(0)
				, _pos1(0)
			{
			}
		};

		struct State
		{
			size_t len;
			size_t link;

			//typedef std::map<char,Next> TMNext;
			//typedef boost::unordered_map<char,Next> TMNext;
			typedef boost::container::flat_map<char, Next> TMNext;
			TMNext next;

			State &operator=(const State &from)
			{
				len = from.len;
				link = from.link;
				next = from.next;
				return *this;
			}
		};

		typedef std::vector<State> TVStates;
		TVStates _states;
	};
}}

#endif

