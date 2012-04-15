#ifndef _ASYNC_ASIOBRIDGE_HPP_
#define _ASYNC_ASIOBRIDGE_HPP_

#include <boost/bind.hpp>

namespace async
{
	template <typename Handler>
	class AsioBridge
	{
	private:
		typedef AsioBridge<Handler> this_type;
		this_type& operator=(const this_type&);

	public:
		/// тип возващаемого значения при вызове объекта этого класса как функтора, необходимо для  boost::bind
		typedef void result_type;

		/// \param handler пользовательский функтор, от него будет произведена копия, которая будет уничтожена при разрушении AsioBridge
		AsioBridge(const Handler& handler)
			: _handler(handler)
		{
		}

		/// объект пользовательского функтора будет уничтожен
		~AsioBridge()
		{
		}

		
		///	@name вызовы с раличным количеством аргументов (0-5) и константностью
		/*!	вызовы инициируются в недрах boost::asio, эти методы пробрасывают 
			вызов в async::exec, который исполняет функтор в асинхронном режиме
		*/
		//@{
		void operator()()
		{
			async::exec(_handler);
		}

		template <typename Arg1>
		void operator()(const Arg1& arg1)
		{
			async::exec(boost::bind(_handler, boost::cref(arg1)));
		}

		template <typename Arg1, typename Arg2>
		void operator()(const Arg1& arg1, const Arg2& arg2)
		{
			async::exec(boost::bind(_handler, boost::cref(arg1), boost::cref(arg2)));
		}

		template <typename Arg1, typename Arg2, typename Arg3>
		void operator()(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3)
		{
			async::exec(boost::bind(_handler, boost::cref(arg1), boost::cref(arg2), boost::cref(arg3)));
		}

		template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
		void operator()(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3,
			const Arg4& arg4)
		{
			async::exec(boost::bind(_handler, boost::cref(arg1), boost::cref(arg2), boost::cref(arg3), boost::cref(arg4)));
		}

		template <typename Arg1, typename Arg2, typename Arg3, typename Arg4,
			typename Arg5>
			void operator()(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3,
			const Arg4& arg4, const Arg5& arg5)
		{
			async::exec(boost::bind(_handler, boost::cref(arg1), boost::cref(arg2), boost::cref(arg3), boost::cref(arg4), boost::cref(arg5)));
		}

		void operator()() const
		{
			async::exec(_handler);
		}

		template <typename Arg1>
		void operator()(const Arg1& arg1) const
		{
			async::exec(boost::bind(_handler, boost::cref(arg1)));
		}

		template <typename Arg1, typename Arg2>
		void operator()(const Arg1& arg1, const Arg2& arg2) const
		{
			async::exec(boost::bind(_handler, boost::cref(arg1), boost::cref(arg2)));
		}

		template <typename Arg1, typename Arg2, typename Arg3>
		void operator()(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3) const
		{
			async::exec(boost::bind(_handler, boost::cref(arg1), boost::cref(arg2), boost::cref(arg3)));
		}

		template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
		void operator()(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3,
			const Arg4& arg4) const
		{
			async::exec(boost::bind(_handler, boost::cref(arg1), boost::cref(arg2), boost::cref(arg3), boost::cref(arg4)));
		}

		template <typename Arg1, typename Arg2, typename Arg3, typename Arg4,
			typename Arg5>
			void operator()(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3,
			const Arg4& arg4, const Arg5& arg5) const
		{
			async::exec(boost::bind(_handler, boost::cref(arg1), boost::cref(arg2), boost::cref(arg3), boost::cref(arg4), boost::cref(arg5)));
		}

	private:

		Handler _handler;
	};
}

#endif
