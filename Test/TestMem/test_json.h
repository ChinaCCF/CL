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
namespace test_json
{

	class AllocMem
	{
	public:
		char* alloc(s64 size) { return (char*)::malloc(size); }
		char* realloc(void* p, s64 size) { return (char*)::realloc(p, size); }
		void free(void* p) { ::free(p); }
	};

	void fun()
	{
		setlocale(LC_ALL, "chs");

		//{
		//	cl::_MixObj<char, AllocMem> mo;
		//	mo["c"] = 1;
		//	mo["b"] = 2;

		//	cl::_MixObj<char, AllocMem> mo2;
		//	mo2 = 8;

		//	mo["2"] = std::move(mo2);

		//	cl::_MixObj<char, AllocMem> mo3;
		//	mo3["1"] = 1;
		//	mo3["2"] = 2;

		//	mo["3"] = std::move(mo3);

		//	cl::_MixObj<char, AllocMem> mo4;
		//	mo4.push(1);
		//	mo4.push(2);

		//	mo["4"] = std::move(mo4);


		//	cl::_Json<char, AllocMem> json;
		//	json.root_ = std::move(mo);
		//	auto str = json.dump();
		//	printf("%s", str.data());
		//}

		//{
		//	cl::_MixObj<wchar, AllocMem> mo;
		//	mo[L"c"] = 1;
		//	mo[L"b"] = 2;

		//	cl::_MixObj<wchar, AllocMem> mo2;
		//	mo2 = 8;

		//	mo[L"2"] = std::move(mo2);

		//	cl::_MixObj<wchar, AllocMem> mo3;
		//	mo3[L"1"] = 1;
		//	mo3[L"2"] = 2;

		//	mo[L"3"] = std::move(mo3);

		//	cl::_MixObj<wchar, AllocMem> mo4;
		//	mo4.push(1);
		//	mo4.push(2);

		//	mo[L"4"] = std::move(mo4);

		//	cl::_Json<wchar, AllocMem> json;
		//	json.root_ = std::move(mo);
		//	auto str = json.dump(false);
		//	printf("%ws", str.data());
		//}

		//{
		//	const char* str = "{\"c\":1,\"b\":true,\"2\":null,\"3\":{\"1\":false,\"2\":2},\"4\":[1,2.3456, \"жа2\\n3\"]}";
		//	cl::_Json<char, AllocMem> json;
		//	json.parse(str);

		//	json << cl::Fraction(4);
		//	auto ret = json.dump(false, false);
		//	printf("%s", ret.data());
		//	int i = 0;
		//}

		{
			const wchar* str = L"{\"c\":1,\"b\":true,\"2\":null,\"3\":{\"1\":false,\"2\":2},\"4\":[1,2.3456, \"жа2\\n3\"]}";
			cl::_Json<wchar, AllocMem> json;
			json.parse(str);

			json << cl::Fraction(4);
			auto ret = json.dump(false, false);
			//wprintf(L"%ws", str);
			printf("%ws", ret.data());
		}
	}

}