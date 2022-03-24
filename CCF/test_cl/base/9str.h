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
				//sv = stds; //报错
			}
		}
		
		{//string
			{//定义
				cl::StrBuf<4> str1;
				cl::StrBufW<4> str2;
				cl::String str3;
				cl::StringW str4; 
			}
		
			{//初始化
				//cl::StrBuf<4> sb = "1234";

				cl::StrBuf<4> str1 = "123";
				cl::String str2 = "123";

				cl::StrView sv = "123"; 
				cl::StrBuf<4> str3 = sv;
				cl::String str4 = sv;

				cl::StrBuf<4> str5 = str1;
				cl::StrBuf<4> str6 = str2;

				cl::String str7 = str1;
				cl::String str8 = str2;

				cl::StrBuf<4> str9 = std::move(str5);
				cl::String str10 = std::move(str7);
				int i = 0;
			}

			{//转换
				cl::String str = "123";
				int i = str;
				CL_Assert(i == 123); 
			}
			{// =
				cl::String str;

				str = "123";
				CL_Assert(str == "123");
				str = 123;
				CL_Assert(str == "123");
				str = 123.4;
				CL_Assert(str == "123.400000");

				cl::StrView sv = "123";
				str = sv;
				CL_Assert(str == "123");

				cl::StrBuf<4> sb = "abc";
				str = sb;
				CL_Assert(str == "abc");

				int i = 0;
			}

			{//<<
				cl::String str;
				str << "abc";
				CL_Assert(str == "abc");
				str << 123;
				CL_Assert(str == "abc123");
				str << false;
				CL_Assert(str == "abc123false");
				str << cl::Fraction(123.4, 1); 
				CL_Assert(str == "abc123false123.4");
				str << nullptr;
				CL_Assert(str == "abc123false123.4null");

				str = "";
				cl::StrView sv = "123";
				str << sv;
				CL_Assert(str == "123");
				cl::StrBuf<4> sb = "abc";
				str << sb;
				CL_Assert(str == "123abc");

				int i = 0;
			}

			{// ==
				cl::String str = "123";
				bool ret = str == "123";
				CL_Assert(ret);

				cl::StrBuf<4> sb = "123";
				ret = str == sb;
				CL_Assert(ret);

				cl::StrView sv = "123";
				ret = str == sv;
				CL_Assert(ret);

				cl::String str2 = "123";
				ret = str == str2;
				CL_Assert(ret);

				int i = 0;
			}
		}
	}
}