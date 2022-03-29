#include <Windows.h>
#include "../1app.h"

namespace wl
{
	extern "C" IMAGE_DOS_HEADER __ImageBase; 

	HINSTANCE APP::instance() 
	{
		static HINSTANCE ins = ((HINSTANCE)&__ImageBase);
		return ins;
	}
}