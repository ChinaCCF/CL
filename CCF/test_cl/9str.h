#include <CL/base/9str.h>  
namespace test
{ 
	void test_str()
	{
		{//strview
			{
				cl::StrView sv;
				CL_DBG(sv == nullptr);
			}
			{
				cl::StrView sv = "123";
				CL_DBG(sv.length() == 3);
			}
			{
				cl::StrView sv = "123";
				CL_DBG(sv == "123");
			}
			{
				cl::StrView sv = "123";
				CL_DBG(sv != nullptr);
			}
			{
				cl::StrView sv = "123";
				sv32 i = sv;
				CL_DBG(i == 123);
			}
			{
				cl::StrView sv = "123.456";
				fv64 i = sv;
				CL_DBG(cl::abs(i - 123.456) < 0.000001);
			}
			{//std::string convert
				std::string stds = "123";
				cl::StrView sv;
				sv = stds;
				CL_DBG(sv == "123");
			}
			{//std::string convert
				std::wstring stds = L"123";
				cl::StrView sv;
				//sv = stds; //±¨´í
			}
		}
		
		{//string
			{
				//cl::lib::_NString<uc8, cl::MemAllocator, 0> str; 
				/*bool ret = str.set("123");
				CL_DBG(ret == true);
				CL_DBG(str == "123");
				ret = str.set("12345678901234567");
				CL_DBG(ret == true);
				CL_DBG(str == "12345678901234567");
				int i = 0;*/
			}
		
		}
	}
}