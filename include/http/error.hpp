#ifndef _HTTP_ERROR_HPP_
#define _HTTP_ERROR_HPP_

#include <boost/system/error_code.hpp>

namespace http { namespace error
{
    enum basic
    {
    	ok = 0,

		unexpected,
    	not_implemented,
		need_more_data,

    	bad_value,
		bad_message,
    	bad_url,
		bad_zlib_stream,
    };
    
    class basic_category : public boost::system::error_category
    {
    public:
      virtual const char *name() const
      {
    	  return "http";
      }
      virtual std::string message(int ev) const
      {
    	  switch(ev)
    	  {
    	  case ok:
    		  return "ok";
    	  case not_implemented:
    		  return "not implemented";
    	  case bad_value:
    		  return "bad value";
    	  case unexpected:
    		  return "unexpected";
    	  case bad_message:
    		  return "bad message";
    	  case need_more_data:
    		  return "need more data";
    	  case bad_url:
    		  return "bad url";
		  case bad_zlib_stream:
			  return "bad zlib stream";
    	  }
    	  return "unknown";
      }
    };

    template <class Category>
    struct CategoryHolder
    {
    	static const Category _category;
    };
    template <class Category>
    const Category CategoryHolder<Category>::_category;

	inline const boost::system::error_category& get_basic_category()
	{
	  return CategoryHolder<basic_category>::_category;
	}



    inline boost::system::error_code make(basic e=ok)
    {
      return boost::system::error_code(
          static_cast<int>(e), get_basic_category());
    }

    inline boost::system::error_code make_error_code(basic e)
    {
      return make(e);
    }

}}

namespace boost { namespace system
{
	template<> struct is_error_code_enum<http::error::basic>
	{
	  static const bool value = true;
	};
}}
#endif
