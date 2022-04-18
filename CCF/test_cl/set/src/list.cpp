#include <libCL/set/3list.h>

namespace test
{
	namespace ntest_list
	{

	}

	void test_list()
	{
		cl::List<int> list;
		list.push_back(1);
		list.push_back(2);

		list.pop_back();

		cl::List<int> list2 = list;
		cl::List<int> list3 = std::move(list2);

		list2 = list;
		list3 = std::move(list2);

		list2 << list;
		list3 << std::move(list2);

		list.new_front() = 3;
		list.insert_at(1, 2);

		list.invert();

		for (auto p : list)
			printf("%d\n", p);

		auto it = list.find(2);
		list.remove(it);

		list.remove_if([](int* p)->bool 
		{
			if (*p == 3) return true;
			return false;
		});

		list.push_back(3);
		list.push_back(2);
		list.sort([](int* p1, int* p2)->sv32 
		{
			if (*p1 <= *p2) return 0;
			return 1;
		});
	}
}