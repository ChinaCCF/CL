#include <Windows.h>
#include "../2msg.h"
#include <libCL/base/9str.h>

namespace wl
{
	void Message::loop()
	{
		MSG msg;
		while (GetMessageW(&msg, nullptr, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}
	 
	namespace lib
	{ 
		cl_si const wchar* _get_str(bool& status, const wchar* str, bool repeat = true)
		{
			if (status && !repeat) return L"";
			status = true;
			return str;
		}
	}
#define MsgToStr(x, ...)  case x:{ static bool _##x = false; return lib::_get_str(_##x, L#x L"\n", __VA_ARGS__); } break

	const wchar* Message::to_str(uv32 msg)
	{
		static cl::StrBufW<128> buf;
		switch (msg)
		{
			MsgToStr(WM_DEVICECHANGE);

			//鼠标
			MsgToStr(WM_MOUSEMOVE, false);
			MsgToStr(WM_MOUSEWHEEL);
			MsgToStr(WM_MOUSEHWHEEL);
			MsgToStr(WM_LBUTTONDOWN);
			MsgToStr(WM_LBUTTONUP);
			MsgToStr(WM_NCLBUTTONDOWN);

			//按键 
			MsgToStr(WM_KEYDOWN);
			MsgToStr(WM_KEYUP);
			MsgToStr(WM_CHAR);

			//输入法
			MsgToStr(WM_IME_SETCONTEXT);
			MsgToStr(WM_IME_NOTIFY);
			MsgToStr(WM_INPUTLANGCHANGE);
			MsgToStr(WM_IME_STARTCOMPOSITION);
			MsgToStr(WM_IME_ENDCOMPOSITION);
			MsgToStr(WM_IME_COMPOSITION);
			MsgToStr(WM_IME_COMPOSITIONFULL);
			MsgToStr(WM_IME_CHAR);
			MsgToStr(WM_IME_REQUEST);
			MsgToStr(WM_IME_CONTROL);

			MsgToStr(WM_NCCREATE);
			MsgToStr(WM_CREATE);
			MsgToStr(WM_GETICON);
			MsgToStr(WM_INITDIALOG);

			MsgToStr(WM_SHOWWINDOW);
			MsgToStr(WM_NCHITTEST, false);
			MsgToStr(WM_SETCURSOR, false);
			MsgToStr(WM_SETFOCUS);
			MsgToStr(WM_KILLFOCUS);
			MsgToStr(WM_NCCALCSIZE);
			MsgToStr(WM_SIZE);

			MsgToStr(WM_MOVE, false);
			MsgToStr(WM_MOVING, false);
			MsgToStr(WM_WINDOWPOSCHANGING);
			MsgToStr(WM_WINDOWPOSCHANGED);
			MsgToStr(WM_EXITSIZEMOVE);

			MsgToStr(WM_NCPAINT);
			MsgToStr(WM_PAINT);
			MsgToStr(WM_ERASEBKGND);
			MsgToStr(WM_CTLCOLOREDIT);

			MsgToStr(WM_NCACTIVATE);
			MsgToStr(WM_ACTIVATE);
			MsgToStr(WM_ACTIVATEAPP);

			MsgToStr(WM_COMMAND);
			MsgToStr(WM_SYSCOMMAND);

			MsgToStr(WM_TIMECHANGE);
			MsgToStr(WM_CAPTURECHANGED);
			MsgToStr(WM_GETMINMAXINFO);
			MsgToStr(WM_ENTERSIZEMOVE);
		default:
			buf = L"Unknow Window Message : ";
			buf << msg << '\n';
			return buf.data();
		}
	} 
}