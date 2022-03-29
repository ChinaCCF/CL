#ifndef __wl_ui_window__
#define __wl_ui_window__
 
#include "../base/1type.h"

namespace wl
{
	typedef LRESULT(_stdcall* CB_WindowProc)(HWND, UINT, WPARAM, LPARAM);

	enum class DPI
	{
		_100 = 96,
		_125 = 120,
		_150 = 144,
		_200 = 192
	};

	class Window
	{
	public:
		//×¢²á´°¿ÚÀà
		static bool reg(CB_WindowProc cb, const wchar* class_name);
		static HWND create(uv32 x, uv32 y, uv32 w, uv32 h, const wchar* title,
						   uv32 style, uv32 exstyle,
						   void* extra_data,
						   const wchar* class_name);

		static fv64 get_dpi(HWND hwnd);  
	 };
}

#endif//__wl_ui_window__