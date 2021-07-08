//本项目是视线一些c++语言层次的代码
//io, 内存, 线程等均不包含

#ifndef __clan_base_config__
#define __clan_base_config__

//系统   window还是Linux
//编译器 vc还是gcc
//硬件   x86还是arm
//位数   32位还是64位
//版本   开发还是发布

#define CL_Sys_Win    1
#define CL_Sys_Linux  2

#define CL_Compiler_VC  1
#define CL_Compiler_GCC 2

#define CL_Hardware_x86  1
#define CL_Hardware_arm  2

#define CL_Bits_32   1
#define CL_Bits_64   2

#define CL_Version_Debug   1
#define CL_Version_Release 2

/*#####################################################################################*/
//定义默认值
/*#####################################################################################*/
#define CL_Sys        CL_Sys_Win
#define CL_Compiler   CL_Compiler_VC
#define CL_Hardware   CL_Hardware_x86
#define CL_Bits       CL_Bits_32
#define CL_Version    CL_Version_Release

/*#####################################################################################*/
//更新默认值
/*#####################################################################################*/
#if defined(__LLP64__) || defined(__LP64__) || defined(_M_X64) || defined(_WIN64)
#undef CL_Bits
#define CL_Bits       CL_Bits_64
#endif

#ifdef XDEBUG
#undef CL_Version
#define CL_Version    CL_Version_Debug
#endif

/*#####################################################################################*/
//VC设置lib
/*#####################################################################################*/
#if CL_Compiler == CL_Compiler_VC

#if !defined(CLan_LIB) && !defined(CLan_DLL)
#   ifdef XDEBUG
#       pragma comment(lib, "CLanD.lib")
#   else
#       pragma comment(lib, "CLan.lib")
#   endif 
#endif

#pragma warning(disable:6011) //取消引用 NULL 指针
 
#endif

#endif//__clan_base_config__