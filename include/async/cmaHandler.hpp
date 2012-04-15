#ifndef _ASYNC_CMAHANDLER_HPP_
#define _ASYNC_CMAHANDLER_HPP_


namespace async
{
	template <std::size_t alloc_size>
	class CmaAllocator
		: private boost::noncopyable
	{  
	public:
		CmaAllocator()
			: in_use_(false)
		{
		}

		~CmaAllocator()
		{
		}

		void* allocate(std::size_t size)
		{
			if (!in_use_ && size <= storage_.size)
			{
				in_use_ = true;
				return storage_.address();
			}      
			return ::operator new(size);      
		}

		void deallocate(void* pointer)
		{
			if (pointer == storage_.address())
			{        
				in_use_ = false;
				return;
			}      
			::operator delete(pointer);      
		}

	private:    
		boost::aligned_storage<alloc_size> storage_;    
		bool in_use_;
	}; //class CmaAllocator


	//////////////////////////////////////////////////////////////////////////
	template <typename Allocator, typename Handler>
	class CmaHandler
	{
	private:
		typedef CmaHandler<Allocator, Handler> this_type;
		this_type& operator=(const this_type&);

	public:
		typedef void result_type;


		CmaHandler(Allocator& allocator, const Handler& handler)
			: allocator_(allocator)
			, handler_(handler)
		{
		}



		~CmaHandler()
		{
		}

		friend void* asio_handler_allocate(std::size_t size, this_type* context)
		{
			return context->allocator_.allocate(size);
		}

		friend void asio_handler_deallocate(void* pointer, std::size_t /*size*/, 
			this_type* context)
		{
			context->allocator_.deallocate(pointer);
		} 

// 		template <typename Function>
// 		friend void asio_handler_invoke(const Function& function, this_type* context)
// 		{
// 			boost::asio::asio_handler_invoke(function, boost::addressof(context->handler_));
// 		}

		void operator()()
		{
			handler_();
		}    

		template <typename Arg1>
		void operator()(const Arg1& arg1)
		{
			handler_(arg1);
		}

		template <typename Arg1, typename Arg2>
		void operator()(const Arg1& arg1, const Arg2& arg2)
		{
			handler_(arg1, arg2);
		}

		template <typename Arg1, typename Arg2, typename Arg3>
		void operator()(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3)
		{
			handler_(arg1, arg2, arg3);
		}

		template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
		void operator()(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, 
			const Arg4& arg4)
		{
			handler_(arg1, arg2, arg3, arg4);
		}

		template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, 
			typename Arg5>
			void operator()(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, 
			const Arg4& arg4, const Arg5& arg5)
		{
			handler_(arg1, arg2, arg3, arg4, arg5);
		}  

		void operator()() const
		{
			handler_();
		}

		template <typename Arg1>
		void operator()(const Arg1& arg1) const
		{
			handler_(arg1);
		}

		template <typename Arg1, typename Arg2>
		void operator()(const Arg1& arg1, const Arg2& arg2) const
		{
			handler_(arg1, arg2);
		}

		template <typename Arg1, typename Arg2, typename Arg3>
		void operator()(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3) const
		{
			handler_(arg1, arg2, arg3);
		}

		template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
		void operator()(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, 
			const Arg4& arg4) const
		{
			handler_(arg1, arg2, arg3, arg4);
		}

		template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, 
			typename Arg5>
			void operator()(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, 
			const Arg4& arg4, const Arg5& arg5) const
		{
			handler_(arg1, arg2, arg3, arg4, arg5);
		}

	private:

		Allocator& allocator_;
		Handler handler_;
	}; //class CmaHandler

	template <typename Allocator, typename Handler>
	inline CmaHandler<Allocator, Handler> 
		makeCmaHandler(Allocator& allocator, const Handler& handler)
	{
		return CmaHandler<Allocator, Handler>(allocator, handler);
	}

	typedef CmaAllocator<512> Allocator;
	typedef boost::shared_ptr<Allocator> AllocatorPtr;

}

#endif
