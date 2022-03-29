#include <Windows.h>
#include <shellscalingapi.h>
#include "../1window.h"
#include "../../app/1app.h"

namespace wl
{
	constexpr uv32 WindowClassExtraSize = sizeof(void*) * 32;
	constexpr uv32 WindowExtraSize = sizeof(void*) * 32;

	bool Window::reg(WNDPROC cb, const wchar* class_name)
	{
		//ʹ�� CreateDialog ���� CreateWindow ��������ʱ, cbWndExtra Ҫ���� DLGWINDOWEXTRA

		auto ins = APP::instance();

		WNDCLASSEXW wc;
		//�Ѿ�ע�����
		if (GetClassInfoExW(NULL, class_name, &wc)) return true;
		if (GetClassInfoExW(ins, class_name, &wc)) return true;

		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = cb;
		wc.cbClsExtra = WindowClassExtraSize;
		wc.cbWndExtra = WindowExtraSize;
		wc.hInstance = ins;
		wc.hIcon = NULL;
		wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName = NULL;
		wc.lpszClassName = class_name;
		wc.hIconSm = NULL;
		ATOM atom = RegisterClassExW(&wc);
		if (atom == NULL) return GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
		return true;
	}

	HWND Window::create(uv32 x, uv32 y, uv32 w, uv32 h, const wchar* title,
						uv32 style, uv32 exstyle,
						void* extra_data,
						const wchar* class_name)
	{
		RECT rect = { 0, 0, (LONG)w, (LONG)h };
		AdjustWindowRect(&rect, style, false); //���ݿͻ������С����ʽ, ��������ʵ�ʴ�С
		w = rect.right - rect.left;
		h = rect.bottom - rect.top;
		return CreateWindowExW(DWORD(exstyle),//��չ��ʾ���
							   class_name,//����������
							   title,//����
							   DWORD(style),//���
							   x, y, w, h,
							   nullptr,//parent
							   nullptr,//�˵���
							   APP::instance(),
							   extra_data);//WM_CREATE��Ϣ�����Ĳ��� 
	}

	fv64 Window::get_dpi(HWND hwnd)
	{
		HMONITOR monitor;
		if (hwnd)
			monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
		else
		{
			POINT pt;
			pt.x = pt.y = 10;
			monitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
		}

		UINT x, y;
		GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &x, &y);
		if (x == (UINT)DPI::_125) return 1.25;
		if (x == (UINT)DPI::_150) return 1.5;
		if (x == (UINT)DPI::_200) return 2.0;
		return 1.0;
	} 
}