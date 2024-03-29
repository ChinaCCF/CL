#include "../3print.h"
#include <locale.h>
namespace cl
{
	void Print::init()
	{
#if CL_Version == CL_Version_Debug 
		static bool has_init = false;
		if (!has_init)
		{
			has_init = true;
			setlocale(LC_ALL, "chs");
		}
#endif
	}
	Print& Print::operator<<(const tc8* val)
	{
#if CL_Version == CL_Version_Debug 
		std::cout << (const char*)val; 
#endif
		return *this; 
	}
	Print& Print::operator<<(const tc16* val)
	{
#if CL_Version == CL_Version_Debug 
		init();
		wprintf(L"%s", val);
#endif
		return *this;
	} 
}