#ifndef _HTTP_ERROR_HPP_
#define _HTTP_ERROR_HPP_

#include <boost/system/error_code.hpp>

namespace http { namespace error
{
    enum basic
    {
    	ok = 0,
    	not_implemented,
    	wrong_value,
    	unexpected,
    	invalid_message,
    	need_more_data,
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
    	  case wrong_value:
    		  return "wrong value";
    	  case unexpected:
    		  return "unexpected";
    	  case invalid_message:
    		  return "invalid message";
    	  case need_more_data:
    		  return "need more data";
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
