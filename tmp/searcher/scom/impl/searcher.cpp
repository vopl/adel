#define _HAS_ITERATOR_DEBUGGING 0
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
		_states.reserve(size1*2);
		_states.push_back(State());
		_states.back().len = 0;
		_states.back().link = (size_t)-1;
		
		size_t last = 0;

		for(size_t cidx(0); cidx<_size1; cidx++)
		{
			char c = _str1[cidx];
			
			size_t cur = _states.size();
			_states.push_back(State());
			
			_states[cur].len = _states[last].len + 1;
			size_t p;
			for (p=last; p!=(size_t)-1 && !_states[p].next.count(c); p=_states[p].link)
			{
				_states[p].next[c]._idx = cur;
				_states[p].next[c]._pos1 = cidx;
			}
			
			if (p == (size_t)-1)
			{
				_states[cur].link = 0;
			}
			else
			{
				size_t q = _states[p].next[c]._idx;
				
				if (_states[p].len + 1 == _states[q].len)
				{
					_states[cur].link = q;
				}
				else
				{
					size_t clone = _states.size();
					_states.push_back(State());
					
					_states[clone].len = _states[p].len + 1;
					_states[clone].next = _states[q].next;
					_states[clone].link = _states[q].link;
					for (; p!=(size_t)-1 && _states[p].next[c]._idx==q; p=_states[p].link)
					{
						_states[p].next[c]._idx = clone;
					}
					_states[q].link = _states[cur].link = clone;
				}
			}
			last = cur;
		}
		//_states.swap(TVStates(_states));
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

		State::TMNext *next = NULL;
		State::TMNext::iterator nextIter;

		for(; _pos2<_size2; _pos2++)
		{
			next = &_states[_vIdx].next;
			nextIter = next->find(_str2[_pos2]);

			if(_vIdx && next->end()==nextIter)
			{
				if(len)
				{
					pos2 = _pos2 - len;
					pos1 = _pos1+1-len;
					
					_vIdx = _states[_vIdx].link;
					
					return true;
				}
			
				_vIdx = _states[_vIdx].link;
				len = _states[_vIdx].len;
				
				while(_vIdx)
				{
					next = &_states[_vIdx].next;
					nextIter = next->find(_str2[_pos2]);

					if(next->end() != nextIter)
					{
						break;
					}

					_vIdx = _states[_vIdx].link;
					len = _states[_vIdx].len;
				}
			}		
			
			if(next && next->end() != nextIter)
			{
				_pos1 = nextIter->second._pos1;
				_vIdx = nextIter->second._idx;
				len++;
			}
		}
		return false;
	}

}}
