#ifndef _SCOM_IMPL_SEARCHER_HPP_
#define _SCOM_IMPL_SEARCHER_HPP_

#include <map>
#include <deque>

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
			int		_idx;
			size_t	_pos1;
			
			Next()
				: _idx(0)
				, _pos1(0)
			{
			}
		};
		
		struct State
		{
			int len, link;
			std::map<char,Next> next;
		};
		std::deque<State> _states;
		
		
		
	public:
		/*std::string lcs (std::string s, std::string t)
		{
	 
			int v = 0,  l = 0;
			int pos1 = 0,  pos2 = 0;
		
			for (int i=0; i<(int)t.length(); ++i)
			{
				while (v && !st[v].next.count(t[i]))
				{
					if(l)
					{
						std::cout<<"sub: "<<pos2<<", "<<l<<std::endl;
					}
					pos1 = v;
					pos2 = i;
					
					v = st[v].link;
					l = st[v].len;
				}
			
				if (st[v].next.count(t[i]))
				{
					v = st[v].next[t[i]];
					++l;
				}
			}
			return "";//t.substr (bestpos-best+1, best);
		}*/
	};
}}

#endif

