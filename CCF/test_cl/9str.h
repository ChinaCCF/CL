#include <CL/base/9str.h>  
namespace test
{ 
	void test_str()
	{
		{//strview
			{
				cl::StrView sv;
				cl_dbg(sv == nullptr);
			}
			{
				cl::StrView sv = "123";
				cl_dbg(sv.length() == 3);
			}
			{
				cl::StrView sv = "123";
				cl_dbg(sv == "123");
			}
			{
				cl::StrView sv = "123";
				cl_dbg(sv != nullptr);
			}
			{
				cl::StrView sv = "123";
				sv32 i = sv;
				cl_dbg(i == 123);
			}
			{
				cl::StrView sv = "123.456";
				fv64 i = sv;
				cl_dbg(cl::abs(i - 123.456) < 0.000001);
			}
		}
		
		{//string
			{
				cl::_String<> str; 
				bool ret = str.set("123");
				cl_dbg(ret == true);
				cl_dbg(str == "123");
				ret = str.set("12345678901234567");
				cl_dbg(ret == true);
				cl_dbg(str == "12345678901234567");
				int i = 0;
			}
		
		}
	}
}