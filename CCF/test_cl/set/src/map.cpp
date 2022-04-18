#include <libCL/set/5hash_map.h>

namespace test
{
	namespace ntest_map
	{

	}

	void test_map()
	{
		cl::HashMap<int, int> map;
		map[1] = 1;
		map[2] = 2;

		map.clear();

		map[2] = 3;
		map[3] = 4;

		cl::HashMap<int, int> map2 = map;
		cl::HashMap<int, int> map3; 
		map3 = map;

		
		cl::HashMap<int, int> map4 = std::move(map3);
		cl::HashMap<int, int> map5;
		map5 = std::move(map);

		map << map4;
		map4[4] = 5;
		map << map4;

		map5.clear();
		map5[2] = 4;
		map5[5] = 7;
		map << std::move(map5);

		for (auto& p : map)
		{
			printf("%d : %d\n", p.first, p.second);
		}
		map.remove(5);
		auto it = map.begin();
		map.remove(it);

		int i = 0;
	}
}