#include <stdio.h> 
#include <CLan/base/8str.h>
#include <CLan/base/10ptr.h>

#include <CLan/set/1base.h>
#include <CLan/set/2arr.h>
#include <CLan/set/3list.h>
#include <CLan/set/4heap.h>
#include <CLan/set/5map.h>
#include <CLan/set/6mix_obj.h>

#include <CLan/tool/json.h>


#include <locale.h>
namespace test_mixobj
{

	class AllocMem
	{
	public:
		char* alloc(s64 size) { return (char*)::malloc(size); }
		char* realloc(void* p, s64 size) { return (char*)::realloc(p, size); }
		void free(void* p) { ::free(p); }
	};

	class A
	{
	public:
		A() { printf("##A\n"); }
		~A() { printf("--A\n"); }
	};

	void fun()
	{
		//{
		//	cl::_Array<int, AllocMem> arr;
		//	arr.push(1);
		//	arr.push(2);
		//	arr.push(3);

		//	auto it = arr.begin();
		//	++it;
		//	int i = 0;
		//}

		//{
		//	cl::_List<int, AllocMem> list;
		//	list.push_back(1);
		//	list.push_back(2);
		//	list.push_back(3);

		//	auto it = list.begin();
		//	++it;
		//	int i = 0;
		//}

		//{
		//	cl::_SerialHashMap<cl::_String<char, AllocMem, 0>, int, AllocMem> map;
		//	map["a"] = 1;
		//	map["b"] = 2;

		//	auto it = map.begin();
		//	++it;
		//	int i = 0;
		//}

		//{
		//	cl::_HashMap<cl::_String<char, AllocMem, 0>, int, AllocMem> map;
		//	map["a"] = 1;
		//	map["b"] = 2;

		//	auto it = map.begin();
		//	++it;
		//	int i = 0;
		//}
		//{
		//	cl::_String<char, AllocMem, 0> key = "c";
		//	A a;
		//	auto p = &a;
		//	{
		//		cl::_SerialHashMap<cl::_String<char, AllocMem, 0>, A, AllocMem> map;
		//		{
		//			map["a"] = A();
		//		}

		//		int j = 0;
		//		{
		//			map["b"] = A();
		//		}
		//		int j2 = 0;

		//		map.push_order(key, a);
		//		for (auto& p : map)
		//		{
		//			int i = 0;
		//		}
		//	}
		//}
		
		{
			cl::_MixObj<char, AllocMem> mo;
			//auto it = mo.end();

			mo["a"] = 1;
			mo["b"] = 2;

			//bool is = mo.contain("b");

			//auto it = mo.begin();
			//while (it != mo.end())
			//{
			//	printf("%s : %d\n", it->first.data(), (int)it->second);
			//	++it;
			//	int i = 0;
			//}
			for (auto& p : mo)
			{
				printf("%s : %d\n", p.first.data(), (int)p.second);
			}
			int i = 0;
		}
		 
		int i = 0;
	}

}