#include <Windows.h>
#include "../2ptr.h"

namespace wl
{
	void Fun_CloseHanle::operator()(HANDLE ptr)
	{
		CloseHandle(ptr);
	}
}