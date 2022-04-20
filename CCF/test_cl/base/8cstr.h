#include <libCL/base/8cstr.h> 
#include <chrono>

namespace test
{    
	namespace cs = cl::cstr;
	void test_cstr()
	{
		{
			tc16 buf[32];
			cs::copy(buf, 32, CL_TC16("123"));
		}
		{//tos
			char buf[256];

			{
				auto len = cs::from_val (buf, 256, 123);
				CL_DBG(cs::equ(buf, "123"));
				CL_DBG(len == 3);
			}

			{
				auto len = cs::from_val(buf, 256, 0);
				CL_DBG(cs::equ(buf, "0"));
				CL_DBG(len == 1);
			}

			{
				auto len = cs::from_val(buf, 256, 100);
				CL_DBG(cs::equ(buf, "100"));
				CL_DBG(len == 3);
			}
			{
				auto len = cs::from_val(buf, 256, -100);
				CL_DBG(cs::equ(buf, "-100"));
				CL_DBG(len == 4);
			}

			{
				auto len = cs::from_val(buf, 256, 0.0, 1);
				CL_DBG(cs::equ(buf, "0.0"));
				CL_DBG(len == 3);
			}
			{
				auto len = cs::from_val(buf, 256, 12.34, 2);
				CL_DBG(cs::equ(buf, "12.34"));
				CL_DBG(len == 5);
			}
			{
				auto len = cs::from_val(buf, 256, 0.003, 3);
				CL_DBG(cs::equ(buf, "0.003"));
				CL_DBG(len == 5);
			}
			{
				auto len = cs::from_val(buf, 256, -1.0, 4);
				CL_DBG(cs::equ(buf, "-1.0000"));
				CL_DBG(len == 7);
			}
			{
				auto len = cs::from_val(buf, 256, false);
				CL_DBG(cs::equ(buf, "false"));
				CL_DBG(len == 5);
			}
			{
				auto len = cs::from_val(buf, 256, true);
				CL_DBG(cs::equ(buf, "true"));
				CL_DBG(len == 4);
			}
		}
		/*########################################################################*/
		/*########################################################################*/
		{ //str_2_val
			{
				bool val;
				auto p = cs::to_val("true", val);
				CL_DBG(val == true);
				CL_DBG(*p == 0);
			}
			{
				bool val;
				auto p = cs::to_val("false", val);
				CL_DBG(val == false);
				CL_DBG(*p == 0);
			}
			{
				int val;
				auto p = cs::to_val("0", val);
				CL_DBG(val == 0);
				CL_DBG(*p == 0);
			}
			{
				int val;
				auto p = cs::to_val("12300", val);
				CL_DBG(val == 12300);
				CL_DBG(*p == 0);
			}
			{
				int val;
				auto p = cs::to_val("-12300", val);
				CL_DBG(val == -12300);
				CL_DBG(*p == 0);
			}
			{
				fv64 val;
				auto p = cs::to_val("0", val);
				CL_DBG(val == 0);
				CL_DBG(*p == 0);
			} 
			{
				fv64 val;
				auto p = cs::to_val("0.01", val);
				CL_DBG(cl::abs(val - 0.01) < 0.000001);
				CL_DBG(*p == 0);
			}
			{
				fv64 val;
				auto p = cs::to_val("-0.01", val);
				CL_DBG(cl::abs(val + 0.01) < 0.000001);
				CL_DBG(*p == 0);
			} 
			{
				fv64 val;
				auto p = cs::to_val("0.100", val);
				CL_DBG(cl::abs(val - 0.1) < 0.000001);
				CL_DBG(*p == 0);
			}
			 
			{
				fv64 val;
				auto p = cs::to_val("-1.20001e2", val);
				CL_DBG(cl::abs(val + 120.001) < 0.000001);
				CL_DBG(*p == 0);
			}
			{
				fv64 val;
				auto p = cs::to_val("-1.20001e-2", val);
				CL_DBG(cl::abs(val + 0.0120001) < 0.000001);
				CL_DBG(*p == 0);
			}
			{
				fv64 val;
				auto p = cs::to_val("-1.20001eabc", val);
				CL_DBG(cl::abs(val + 1.20001) < 0.000001);
				CL_DBG(*p == 'e');
			}
		}

		//{
		//	static const int cnt = 99999;
		//	char buf[128];

		//	auto start = time_point();
		//	for (sv32 i = 0; i < cnt; i++)
		//		snprintf(buf, 128, "%f", -123.456);
		//	auto end = time_point();

		//	auto def_dif = end - start;

		//	start = time_point();
		//	for (sv32 i = 0; i < cnt; i++)
		//		cs::val_2_str(buf, 128, -123.456);
		//	end = time_point();
		//	auto self_dif = end - start;
		//	//def_dif  = 8038726700
		//	//self_dif =  489581100
		//	int j = 0;
		//}

		//{
		//	static const int cnt = 99999;
		//	const char* buf = "-123.456";
		//	fv64 val;

		//	auto start = time_point();
		//	for (sv32 i = 0; i < cnt; i++)
		//		val = atof(buf);
		//	auto end = time_point();

		//	auto def_dif = end - start;

		//	start = time_point();
		//	for (sv32 i = 0; i < cnt; i++)
		//		cs::str_2_val(buf, val);
		//	end = time_point();

		//	auto self_dif = end - start;
		//	//def_dif  = 3836281200
		//	//self_dif =  619881800
		//	int j = 0; 
		//int i = 0;
	}
}