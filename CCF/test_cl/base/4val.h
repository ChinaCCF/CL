#include <libCL/base/4val.h>

namespace test
{  
	namespace ntest_val
	{
		enum class MyCode
		{
			Code1,
			Code2
		};
	}
	void test_val()
	{
		cl::EnumType<ntest_val::MyCode> e = ntest_val::MyCode::Code1;
		//bool val2 = e == 1.0;
		bool val = e == ntest_val::MyCode::Code1;
		uv32 v = e;
	}
}