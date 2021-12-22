#include <CL/base/4val.h>

namespace test
{  
	namespace eval
	{
		enum class MyCode
		{
			Code1,
			Code2
		};
	}
	void test_val()
	{
		cl::EnumType<eval::MyCode> e = eval::MyCode::Code1;
		//bool val2 = e == 1.0;
		bool val = e == eval::MyCode::Code1;
		uv32 v = e;
	}
}