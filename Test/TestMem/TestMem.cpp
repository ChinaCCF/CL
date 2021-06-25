// TestMem.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <stdio.h>   

#include <CLan/base/10call.h>
#include <CLan/base/11time.h>
#include <CLan/base/7str.h>

	class AllocMem
	{
	public:
		char* alloc(s64 size) { return (char*)malloc(size); }
		char* realloc(void* p, s64 size) { return (char*)p; }
		void free(void* p) { ::free(p); }
	};

int main()
{         
	//clan::_String<char, AllocMem, 0> str;
	//auto s = str.str();
	int val = 10;
	clan::Call<void(int)> call = [&val](int a) 
	{
		printf("%d\n", val + a);
	};
 
	clan::Call<void(int)> call2 = call;
	call2(3);
	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
