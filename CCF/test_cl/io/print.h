#pragma once
#include <CL/io/2path.h>
#include <CL/io/3print.h>

namespace test
{
	void test_print()
	{
		cl::Print print;
		print << 123;
		print << "����";
		print << L"�й�"; 
	}
}