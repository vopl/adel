#ifndef _UTILS_LEAKEDOBJECT_HPP_
#define _UTILS_LEAKEDOBJECT_HPP_

namespace utils
{
	template <class T>
	class LeakedObject
	{
		static size_t _cnt;
	public:
		LeakedObject()
		{
			_cnt++;
			if(! (_cnt%1000))
			{
				std::cout<<__FUNCTION__<<": "<<_cnt<<std::endl;
			}
		}

		~LeakedObject()
		{
			_cnt--;
		}
	};

	template <class T>
	size_t LeakedObject<T>::_cnt = 0;
}

#endif
