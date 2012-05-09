#ifndef _ASYNC_FUTURE_HPP_
#define _ASYNC_FUTURE_HPP_

#include "async/event.hpp"

//////////////////////////////////////////////////////////////////////////
namespace async
{
	template <class Data>
	class Future
		: public Event
	{
	private:
		struct State
		{
			Data	_data;
			//bool	_ready;

			State()
				//: _ready(false)
			{
			}
		};
		boost::shared_ptr<State> _state;

	public:
		Future()
			: _state(new State)
			, Event(false)
		{
		}

		void reset()
		{
			Event::reset();
			//_state->_ready = false;
			_state->_data = Data();
		}
		
		void wait()
		{
			//if(!_state->_ready)
			{
				Event::wait();
				//_state->_ready = true;
			}
		}

		Data &data()
		{
			wait();
			return _state->_data;
		}

		operator Data &()
		{
			wait();
			return _state->_data;
		}

		Data &dataNoWait()
		{
			return _state->_data;
		}

	public:

		typedef void result_type;

		void operator()(const Data &data)
		{
			_state->_data = data;
			set();
		}

	public:
		void swap(Future &with)
		{
			_state.swap(with._state);
			Event::swap(with);
		}
	};

	template <class Data1, class Data2>
	class Future2
		: public Event
	{
	private:
		struct State
		{
			Data1	_data1;
			Data2	_data2;
			//bool	_ready;

			State()
			//	: _ready(false)
			{
			}
		};
		boost::shared_ptr<State> _state;

	public:
		Future2()
			: _state(new State)
			, Event(false)
		{
		}

		void reset()
		{
			Event::reset();
			//_state->_ready = false;
			_state->_data1 = Data1();
			_state->_data2 = Data2();
		}

		void wait()
		{
			//if(!_state->_ready)
			{
				Event::wait();
				//_state->_ready = true;
			}
		}

		Data1 &data1()
		{
			wait();
			return _state->_data1;
		}
		Data1 &data1NoWait()
		{
			return _state->_data1;
		}

		Data2 &data2()
		{
			wait();
			return _state->_data2;
		}
		Data2 &data2NoWait()
		{
			return _state->_data2;
		}

	public:
		typedef void result_type;

		void operator()(const Data1 &data1, const Data2 &data2)
		{
			_state->_data1 = data1;
			_state->_data2 = data2;
			set();
		}

	public:
		void swap(Future2 &with)
		{
			_state.swap(with._state);
			Event::swap(with);
		}
	};

	template <class Data1, class Data2, class Data3>
	class Future3
		: public Event
	{
	private:
		struct State
		{
			Data1	_data1;
			Data2	_data2;
			Data3	_data3;
			//bool	_ready;

			State()
				//: _ready(false)
			{
			}
		};
		boost::shared_ptr<State> _state;

	public:
		Future3()
			: _state(new State)
			, Event(false)
		{
		}

		void reset()
		{
			Event::reset();
			//_state->_ready = false;
			_state->_data1 = Data1();
			_state->_data2 = Data2();
			_state->_data3 = Data3();
		}

		void wait()
		{
			//if(!_state->_ready)
			{
				Event::wait();
				//_state->_ready = true;
			}
		}

		Data1 &data1()
		{
			wait();
			return _state->_data1;
		}
		Data1 &data1NoWait()
		{
			return _state->_data1;
		}

		Data2 &data2()
		{
			wait();
			return _state->_data2;
		}
		Data2 &data2NoWait()
		{
			return _state->_data2;
		}

		Data3 &data3()
		{
			wait();
			return _state->_data3;
		}
		Data2 &data3NoWait()
		{
			return _state->_data3;
		}
	public:
		typedef void result_type;

		void operator()(const Data1 &data1, const Data2 &data2, const Data3 &data3)
		{
			_state->_data1 = data1;
			_state->_data2 = data2;
			_state->_data3 = data3;
			set();
		}

	public:
		void swap(Future3 &with)
		{
			_state.swap(with._state);
			Event::swap(with);
		}
	};
}

#endif
