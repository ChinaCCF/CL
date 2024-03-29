#ifndef __wl_base_type__
#define __wl_base_type__

#include <libCL/base/2type.h>
 
#if CL_Compiler == CL_Compiler_VC  
#pragma warning(disable:6011) //取消引用 NULL 指针
#	if CL_Version == CL_Version_Debug
#	pragma comment(lib, "libWLD.lib")
#	else
#	pragma comment(lib, "libWL.lib")
#	endif
#endif


typedef int BOOL;
typedef unsigned int UINT;
typedef unsigned long DWORD;

#if CL_Bits == CL_Bits_64 
typedef __int64 INT_PTR, * PINT_PTR;
typedef unsigned __int64 UINT_PTR, * PUINT_PTR; 
typedef __int64 LONG_PTR, * PLONG_PTR;
typedef unsigned __int64 ULONG_PTR, * PULONG_PTR; 
#define __int3264   __int64 
#else 
typedef _W64 int INT_PTR, * PINT_PTR;
typedef _W64 unsigned int UINT_PTR, * PUINT_PTR; 
typedef _W64 long LONG_PTR, * PLONG_PTR;
typedef _W64 unsigned long ULONG_PTR, * PULONG_PTR;  
#define __int3264   __int32 
#endif
  
typedef wchar WCHAR;
typedef const WCHAR* PCWSTR;

typedef UINT_PTR            WPARAM;
typedef LONG_PTR            LPARAM;
typedef LONG_PTR            LRESULT;

typedef void* HANDLE;
 
struct HINSTANCE__;
typedef HINSTANCE__* HINSTANCE;

struct HWND__;
typedef HWND__* HWND;

struct HDC__;
typedef HDC__* HDC;

struct HFONT__;
typedef HFONT__* HFONT;

struct HBITMAP__;
typedef HBITMAP__* HBITMAP;

struct HMONITOR__;
typedef HMONITOR__* HMONITOR;
 
typedef void* HGDIOBJ; 

struct HGLRC__;
typedef HGLRC__* HGLRC;

namespace wl
{
	void _CloseHandle(HANDLE ptr);
	template<typename> struct Fun_CloseHanle { void operator()(HANDLE ptr) { _CloseHandle(ptr); } };
}
#endif//__wl_base_type__ 