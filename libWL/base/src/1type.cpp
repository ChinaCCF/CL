#include <Windows.h>
#include "../1type.h"

namespace wl
{
	void _CloseHandle(HANDLE ptr)
	{
		CloseHandle(ptr);
	}
}