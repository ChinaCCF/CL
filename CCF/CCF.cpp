#include <Windows.h>
#include <iostream>

#pragma warning(disable:4996) 

#include "test_cl/base/3type_traits.h"
#include "test_cl/base/4val.h"
#include "test_cl/base/5mem.h"
#include "test_cl/base/6ptr.h"
#include "test_cl/base/7call.h"
#include "test_cl/base/8cstr.h"
#include "test_cl/base/9str.h"
#include "test_cl/base/10time.h"
 
#include "test_cl/set/set.h"

#include "test_cl/io/path.h"
#include "test_cl/io/print.h"

#include <StackInfo/StackInfo.h>

void fun2()
{
    auto stack = cl::stack_alloc();

	cl::stack_init(); 
    for (uv32 i = 0; i < stack->size_; i++)
    {
        cl::FrameInfo* info = cl::frame_alloc(stack, i);
        CPrint << info->file_ << '[' << info->line_ << ']' << info->fun_ << '\n';
        cl::frame_free(info);
    }
	cl::stack_uninit();

    cl::stack_free(stack);
}
void fun1()
{
    fun2();
}


int main()
{ 
	for(int j = 0; j < 100; j++)
	{
		int v = j;
		for(int i = 0; i < 100; i++)
		{
			printf("%d\n", v & 0xFF);
			v += 0xa7;
			v *= 5;
		}
		printf("\n\n\n");
	}

	test::test_type_traits();
	test::test_val();
	test::test_mem();
	test::test_ptr();
	test::test_cstr();
	test::test_call();
	test::test_str();
	test::test_time();

	test::test_arr();
	test::test_list();
	test::test_heap();
	test::test_map();
	test::test_rbtree();

	test::test_path();
	test::test_print();

	fun1();



	std::cout << "Hello World!\n";
	return 0;
}
 



//
//#include <windows.h>   
//#define WND_CLASSNAME	L"MainWnd"
//#define WND_TITLENAME	L"Test"
//
//class MemHDC
//{
//    HWND hwnd_;
//    HDC hdc_;
//    HDC mem_hdc_;
//    HBITMAP mem_bmp_;
//    HBITMAP old_mem_bmp_;
//    uv32* buf_;
//
//    uv32 w_;
//    uv32 h_;
//
//    BITMAPINFO bmp_info_;
//    RGBQUAD _pad_[2];//为BITMAPINFO的m_bmiColors补充两个元素空间
//public:
//    MemHDC();
//    ~MemHDC() { free_mem_hdc(); }
//    uv32* buf() { return buf_; }
//    HDC hdc() { return mem_hdc_; }
//    void free_mem_hdc();
//    void alloc_mem_hdc(HWND hwnd, uv32 w, uv32 h);
//    void fill();
//};
//
//MemHDC::MemHDC()
//{
//    hwnd_ = nullptr;
//    hdc_ = nullptr;
//    mem_hdc_ = nullptr;
//    mem_bmp_ = nullptr;
//    old_mem_bmp_ = nullptr;
//    buf_ = nullptr;
//    w_ = h_ = 0;
//
//    bmp_info_ = { 0 };
//    BITMAPINFO* info = &bmp_info_;
//    info->bmiHeader.biSize = sizeof(info->bmiHeader);
//    info->bmiHeader.biPlanes = 1;
//    info->bmiHeader.biBitCount = 32;
//    info->bmiHeader.biCompression = BI_BITFIELDS;
//
//    *(UINT*)(info->bmiColors + 0) = 0xFF0000;//red分量
//    *(UINT*)(info->bmiColors + 1) = 0x00FF00;//green分量
//    *(UINT*)(info->bmiColors + 2) = 0x0000FF;//blue分量 
//}
//
//void MemHDC::free_mem_hdc()
//{
//    if (buf_) free(buf_);
//    if (old_mem_bmp_) SelectObject(mem_hdc_, old_mem_bmp_);
//    if (mem_bmp_) DeleteObject(mem_bmp_);
//    if (mem_hdc_) DeleteDC(mem_hdc_);
//    if (hdc_) ReleaseDC(hwnd_, hdc_);
//}
//
//void MemHDC::alloc_mem_hdc(HWND hwnd, uv32 w, uv32 h)
//{
//    free_mem_hdc();
//    w_ = w;
//    h_ = h;
//
//    BITMAPINFO* info = &bmp_info_;
//    info->bmiHeader.biWidth = w;
//    info->bmiHeader.biHeight = - sv32(h);
//
//    hwnd_ = hwnd;
//    hdc_ = GetDC(hwnd_);
//    mem_hdc_ = CreateCompatibleDC(NULL);
//    mem_bmp_ = CreateCompatibleBitmap(hdc_, w, h);
//    old_mem_bmp_ = (HBITMAP)SelectObject(mem_hdc_, mem_bmp_);
//    buf_ = (uv32*)malloc(sizeof(uv32)* w * h);
//}
//void MemHDC::fill()
//{
//    bool ret = SetDIBitsToDevice(mem_hdc_,
//                                 0, 0, w_, h_,
//                                 0, 0, 0, h_,
//                                 buf_, &bmp_info_, DIB_RGB_COLORS) == h_; 
//}
//
//
//LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
//{
//    switch (uMsg)
//    {
//    case WM_LBUTTONDOWN:
//        // 使鼠标单击窗口任何一个地方都能移动整个窗口
//
//        SendMessageA(hWnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0);
//        //SendMessageA(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
//        break;
//    }
//
//    return DefWindowProc(hWnd, uMsg, wParam, lParam);
//}
//
//#define Win_W 914 
//
////int main()
//#pragma comment(linker, "/subsystem:\"windows\" ") 
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int nCmdShow)
//{
//    WNDCLASSEXW wndClass;
//    wndClass.cbSize = sizeof(wndClass);
//    wndClass.style = CS_HREDRAW | CS_VREDRAW;
//    wndClass.lpfnWndProc = WndProc;
//    wndClass.cbClsExtra = 0;
//    wndClass.cbWndExtra = 0;
//    wndClass.hInstance = nullptr;
//    wndClass.hIcon = NULL;//LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NORMAL));
//    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
//    wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
//    wndClass.lpszMenuName = NULL;//MAKEINTRESOURCE(IDC_DAHUAXY2);
//    wndClass.lpszClassName = WND_CLASSNAME;
//    wndClass.hIconSm = NULL;//LoadIcon(wndClass.hInstance, MAKEINTRESOURCE(IDI_SMALL));
//    RegisterClassExW(&wndClass);
//
//    DWORD wndStyle = WS_POPUP;	// 无标题栏:WS_POPUP | WS_MINIMIZEBOX | WS_SYSMENU;
//
//    RECT rcWnd = { 0, 0, Win_W, Win_W };
//    AdjustWindowRect(&rcWnd, wndStyle, FALSE);
//
//    HWND hWnd = CreateWindowExW(0,
//                                WND_CLASSNAME,
//                                WND_TITLENAME,
//                                wndStyle,
//                                (GetSystemMetrics(SM_CXSCREEN) - Win_W) / 2,	// SM_CXSCREEN：以像素为单位的屏幕的宽度
//                                (GetSystemMetrics(SM_CYSCREEN) - Win_W) / 2,	// SM_CYSCREEN：以像素为单位的屏幕的高度
//                                (rcWnd.right - rcWnd.left), (rcWnd.bottom - rcWnd.top),
//                                NULL,
//                                NULL,
//                                nullptr,
//                                NULL);
//
//    ShowWindow(hWnd, SW_SHOWNORMAL);
//
//    DWORD dwExStyle = ::GetWindowLong(hWnd, GWL_EXSTYLE);
//    if (!(dwExStyle & WS_EX_LAYERED))
//        ::SetWindowLong(hWnd, GWL_EXSTYLE, dwExStyle | WS_EX_LAYERED);
//
//    MemHDC mem_hdc;
//    mem_hdc.alloc_mem_hdc(hWnd, Win_W, Win_W);
//    uv32* buf = mem_hdc.buf();
//
//    for (int r = 0; r < Win_W; r++)
//        for (int c = 0; c < Win_W; c++)
//        {
//            char* p = (char*)(buf + r * Win_W + c);
//
//            if (c < 150 && r < 150)
//                p[0] = p[1] = p[2] = p[3] = 0;
//            else
//            {
//                
//
//                p[0] = char(c * 1.0 / Win_W * 255);//blue
//                p[1] = 0;//green
//                p[2] = 0;//red
//                //p[3] = char((c + r) / 600.0 * 255);
//                p[3] = 1;
//            } 
//        }
//
//    MSG msg;
//    memset(&msg, 0, sizeof(msg));
//
//    //glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
//
//    while (TRUE)
//    {
//        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
//        {
//            if (msg.message == WM_QUIT)
//                break;
//
//            TranslateMessage(&msg);
//            DispatchMessage(&msg);
//        }
//
//        {
//            mem_hdc.fill();
//
//            BLENDFUNCTION blend;
//            blend.BlendOp = AC_SRC_OVER;//当前微软仅定义了这个值
//            blend.BlendFlags = 0;//必须为0 
//            blend.SourceConstantAlpha = 0xFF;
//            blend.AlphaFormat = AC_SRC_ALPHA;//当RGBA中alpha通道为0时候,鼠标会穿透
//
//            RECT rcWnd;
//            GetWindowRect(hWnd, &rcWnd);
//            POINT dst_pos = { rcWnd.left, rcWnd.top };
//            SIZE dst_size = { rcWnd.right - rcWnd.left, rcWnd.bottom - rcWnd.top };
//            POINT src_pos = { 0, 0 };
//
//            UPDATELAYEREDWINDOWINFO info = { 0 };
//            info.cbSize = sizeof(info);
//            //info.hdcDst = g_GDI.m_hMainDC;
//            info.pptDst = &dst_pos;
//            info.psize = &dst_size;
//            info.hdcSrc = mem_hdc.hdc();
//            info.pptSrc = &src_pos;
//            info.pblend = &blend;
//            info.dwFlags = ULW_ALPHA;
//
//            //RECT rect = {0, 0, 100, 100};
//            //info.prcDirty = &rect;
//
//            UpdateLayeredWindowIndirect(hWnd, &info);
//        }
//    }
//
//
//    return 0;
//}



 