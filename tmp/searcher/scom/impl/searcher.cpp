#include "scom/impl/searcher.hpp"
#include <algorithm>
#include <iostream>

namespace scom { namespace impl
{
	//////////////////////////////////////////////////////////////////////////////////
	Searcher::Searcher(const char *str1, size_t size1)
		: _str1(str1)
		, _size1(size1)
		, _str2(NULL)
		, _size2(0)
		, _vIdx(0)
		, _pos1(0)
		, _pos2(0)
	{
		_states.push_back(State());
		_states.back().len = 0;
		_states.back().link = -1;
		
		int last = 0;
		
		for(size_t cidx(0); cidx<_size1; cidx++)
		{
			char c = _str1[cidx];
			
			int cur = _states.size();
			_states.push_back(State());
			
			_states[cur].len = _states[last].len + 1;
			int p;
			for (p=last; p!=-1 && !_states[p].next.count(c); p=_states[p].link)
			{
				_states[p].next[c]._idx = cur;
				_states[p].next[c]._pos1 = 11;
			}
			
			if (p == -1)
			{
				_states[cur].link = 0;
			}
			else
			{
				int q = _states[p].next[c]._idx;
				
				if (_states[p].len + 1 == _states[q].len)
				{
					_states[cur].link = q;
				}
				else
				{
					int clone = _states.size();
					_states.push_back(State());
					
					_states[clone].len = _states[p].len + 1;
					_states[clone].next = _states[q].next;
					_states[clone].link = _states[q].link;
					for (; p!=-1 && _states[p].next[c]._idx==q; p=_states[p].link)
					{
						_states[p].next[c]._idx = clone;
						_states[p].next[c]._pos1 = 12;
					}
					_states[q].link = _states[cur].link = clone;
				}
			}
			last = cur;
		}		
	}

	//////////////////////////////////////////////////////////////////////////////////
	Searcher::~Searcher()
	{
	}

	//////////////////////////////////////////////////////////////////////////////////
	void Searcher::reset(const char *str2, size_t size2)
	{
		_str2 = str2;
		_size2 = size2;
		_vIdx = 0;
		_pos1 = 0;
		_pos2 = 0;
	}

	//////////////////////////////////////////////////////////////////////////////////
	bool Searcher::next(size_t &pos1, size_t &pos2, size_t &len)
	{

		len = 0;
		pos1 = 0;
		pos2 = 0;
	
		for(; _pos2<_size2; _pos2++)
		{
			if(_vIdx && !_states[_vIdx].next.count(_str2[_pos2]))
			{
				if(len)
				{
					pos2 = _pos2 - len;
					pos1 = _pos1;
					
					_vIdx = _states[_vIdx].link;
					
					return true;
				}
			
				_vIdx = _states[_vIdx].link;
				len = _states[_vIdx].len;
				
				while(_vIdx && !_states[_vIdx].next.count(_str2[_pos2]))
				{
					_vIdx = _states[_vIdx].link;
					len = _states[_vIdx].len;
				}
			}		
			
			if(_states[_vIdx].next.count(_str2[_pos2]))
			{
				_vIdx = _states[_vIdx].next[_str2[_pos2]]._idx;
				_pos1 = _states[_vIdx].next[_str2[_pos2]]._pos1;
				len++;
			}
		}
		return false;
	}

}}
