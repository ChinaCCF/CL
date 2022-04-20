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

		~Print() { std::cout << std::endl; }

		template<typename T>
			requires (IsBool_v<T> || IsInt_v<T> || IsFloat_v<T> || IsChar_v<T>)
		Print& operator<<(const T& val) { std::cout << val; return *this; }

		template<uv32 N>
		Print& operator<<(const tc8 val[N]) { return operator<<((const tc8*)val); }
		template<uv32 N>
		Print& operator<<(const tc16 val[N]) { return operator<<((const tc16*)val); }
		 
		Print& operator<<(const tc8* val);
		Print& operator<<(const tc16* val); 

		Print& operator<<(const StrView& val) { return operator<<(val.data()); }
		Print& operator<<(const StrViewW& val) { return operator<<(val.data()); }
		Print& operator<<(const String& val) { return operator<<(val.data()); }
		Print& operator<<(const StringW& val) { return operator<<(val.data()); }
	};

#define CPrint cl::Print() << __FILE__ << '(' << __LINE__ << ") :\n"
}
#endif//__cl_io_print__