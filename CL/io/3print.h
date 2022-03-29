#ifndef __cl_io_print__
#define __cl_io_print__

#include "../base/9str.h"
#include "../base/3type_traits.h"
#include <iostream>

namespace cl
{
	//当前类非线程安全
	class Print
	{
	public:
		static void init();

		template<typename T>
		requires (IsBool_v<T> || IsInt_v<T> || IsFloat_v<T>)
		Print& operator<<(const T& val) { std::cout << val; return *this; }

		template<uv32 N>
		Print& operator<<(const char val[N]) { return operator<<((const char*)val); }
		template<uv32 N>
		Print& operator<<(const wchar val[N]) { return operator<<((const wchar*)val); }

		Print& operator<<(const char* val) { std::cout << val; return *this; }
		Print& operator<<(const uc8* val);
		Print& operator<<(const wchar* val);
		Print& operator<<(const uc16* val) { return operator<<((const wchar*)val); }

		Print& operator<<(const StrView& val) { return operator<<(val.data()); }
		Print& operator<<(const StrViewW& val) { return operator<<(val.data()); }
		Print& operator<<(const String& val) { return operator<<(val.data()); }
		Print& operator<<(const StringW& val) { return operator<<(val.data()); }
	 };
}
#endif//__cl_io_print__