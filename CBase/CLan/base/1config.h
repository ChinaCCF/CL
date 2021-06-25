//本项目是视线一些c++语言层次的代码
//io, 内存, 线程等均不包含

#ifndef __clan_config__
#define __clan_config__

#ifdef _MSC_VER

#if !defined(CLan_LIB) && !defined(CLan_DLL)
#   ifdef XDEBUG
#       pragma comment(lib, "CLanD.lib")
#   else
#       pragma comment(lib, "CLan.lib")
#   endif 
#endif

#define Compiler_VC

#endif//_MSC_VER

#endif//__clan_config__