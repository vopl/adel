#define _HAS_ITERATOR_DEBUGGING 0
#include <iostream>
#include <fstream>
#include "scom/impl/searcher.hpp"
#include <vector>
#include <cassert>
#include <cstring>

void load(std::vector<char> &buf, const char *fname)
{
	std::ifstream is(fname, std::ios_base::binary);
	
	is.seekg(0, std::ios_base::end);
	buf.resize((size_t)is.tellg());
	is.seekg(0, std::ios_base::beg);
	
	if(!buf.empty())
	{
		is.read(&buf[0], buf.size());
	}
}

int main()
{

	std::vector<char> buf1,buf2;
	
	load(buf1, "t1.html");
	load(buf2, "t2.html");


	scom::impl::Searcher s(&buf1[0], buf1.size());
	for(size_t i(0); i<10000; i++)
	{
		s.reset(&buf2[0], buf2.size());
	
		size_t pos1, pos2, len;
		while(s.next(pos1, pos2, len))
		{
			//if(len>1000000)
			{
				std::cout<<pos1<<", "<<pos2<<", "<<len<<std::endl;
			}
			
			assert(pos1 + len <= buf1.size());
			assert(pos2 + len <= buf2.size());
			assert((!memcmp(&buf1[pos1], &buf2[pos2], len)));
		}
		//return 0;

		if(!(i%100))
		{
			std::cout<<i<<std::endl;
		}
	}
	return 0;
}
