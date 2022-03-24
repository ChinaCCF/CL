#include <CL/set/2arr.h>

namespace test
{
	namespace ntest_list
	{

	}

	void test_arr()
	{
		{
			cl::Stack<int, 4> arr;
			arr.push(1);
			arr.push(2);

			arr.clear();

			arr.push(1);
			arr.clear();

			arr.clear();

			arr.push(1);
			arr.pop();

			arr.push(1);
			arr.push(2);

			for (auto p : arr)
				printf("%d", p);

			int i = 0;
		}
	
		{
			cl::Array<int> arr;

			arr.push(1);
			arr.push(3);
			arr.insert(1, 2);
			arr.push(4);

			arr.pop();

			for(auto p : arr)
				printf("%d", p);

			cl::Array<int> arr2 = arr;
			cl::Array<int> arr3 = std::move(arr);

			arr = arr2;
			arr3 = std::move(arr2);
			arr3.clear();
			arr2.clear();
			

			arr.remove(1);

			auto it = arr.find(2);
			arr.remove(it);

			arr.push(1);
			arr.push(4);
			arr.push(2);
			arr.sort([](int* p1, int* p2)->sv32 
			{
				if (*p1 < *p2) return -1;
				if (*p1 > *p2) return 1;
				return 0;
			});

			arr.remove_if([](int* p) ->bool
			{
				if (*p == 2) return true;
				return false;
			},
						  true);

			int i = 0;
		}
	}
}