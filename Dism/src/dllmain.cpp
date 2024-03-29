﻿// dllmain.cpp : 定义 DLL 应用程序的入口点。
#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
// Windows 头文件
#include <windows.h>
#include <wimgapi.h>
#include <dismapi.h>
#include "../dism.h"

#pragma comment(lib, "wimgapi.lib")
#pragma comment(lib, "dismapi.lib")
namespace cl
{
    dism_api void _WIMCloseHandle(HANDLE ptr) { WIMCloseHandle(ptr); }
    dism_api void _DismDelete(HANDLE ptr) { DismDelete(ptr); }
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

