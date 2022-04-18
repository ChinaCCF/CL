#include <libCL/base/3type_traits.h>

namespace test
{ 

	template<typename T, typename D>
	void _check_raw_type()
	{
		bool ret = cl::IsSame_v<cl::RawType_t<T>, D>;
		CL_DBG(ret);
	}
	template<typename T, typename D>
	void _check_raw_type_with_const()
	{
		using type = cl::RawType_t<const T&>;
		//type t;
		bool ret = cl::IsSame_v<type, D>;
		CL_DBG(ret);
	}

	void test_type_traits() 
	{
		{
			char arr[] = "123";
			_check_raw_type<decltype(arr), char*>();
		}
		{
			const char arr[] = "123";
			_check_raw_type<decltype(arr), char*>();
		}

		{
			const char arr[] = "abc";
			_check_raw_type_with_const<decltype(arr), char*>();
		}
		int i = 0;
	}
}