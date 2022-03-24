#ifndef __cl_base_cfg__
#define __cl_base_cfg__

// ����ļ����е�ͷ�ļ�, ��������Ӧ�ó���, Windows��Linux
// ���������Ҫ���Ǳ���֧��C++20, �������ڴ�

//ϵͳ   window����Linux
//������ vc����gcc
//Ӳ��   x86����arm
//λ��   32λ����64λ
//�汾   �������Ƿ���

#define CL_Sys_Win       1
#define CL_Sys_Linux     2
#define CL_Sys_Mac       3
#define CL_Sys_Android   4 
#define CL_Sys_IOS       5

#define CL_Compiler_VC  1
#define CL_Compiler_GCC 2

#define CL_Hardware_x86  1   //x64Ҳ����x86
#define CL_Hardware_arm  2

#define CL_Bits_32   1
#define CL_Bits_64   2

#define CL_Version_Debug   1
#define CL_Version_Release 2

/*#####################################################################################*/
//����Ĭ��ֵ
/*#####################################################################################*/
#define CL_Sys        CL_Sys_Linux
#define CL_Compiler   CL_Compiler_GCC 
#define CL_Bits       CL_Bits_32
#define CL_Version    CL_Version_Release
#define CL_Hardware   CL_Hardware_x86

/*#####################################################################################*/
//����Ĭ��ֵ
/*#####################################################################################*/

//ϵͳ
#if defined(_WIN64) || defined(_WIN32)
#   undef CL_Sys
#	define CL_Sys  CL_Sys_Win
#endif

#if defined(ANDROID) || defined(__ANDROID__)
#   undef CL_Sys
#	define CL_Sys  CL_Sys_Android
#endif

#if defined(__APPLE__)  
#	if defined(TARGET_OS_MAC)  
#       undef CL_Sys
#		define CL_Sys  CL_Sys_Mac
#	else //TARGET_OS_IPHONE
#       undef CL_Sys
#		define CL_Sys  CL_Sys_IOS
#	endif
#endif

//������
#if defined(_MSC_VER)   
#	if _MSC_VER < 1929 //vs2019
#		error "Not Support Previous VS!" 
#	endif 
#   undef CL_Compiler
#	define CL_Compiler   CL_Compiler_VC   
#endif

//ƽ̨λ��
#if defined(__LLP64__) || defined(__LP64__) || defined(_M_X64) || defined(_WIN64) || defined(__amd64) || defined(__x86_64) || defined(__x86_64__) 
#   undef CL_Bits
#   define CL_Bits       CL_Bits_64
#endif

//������Ի��Ƿ����汾
#if defined(XDEBUG) || defined(DEBUG) || defined(_DEBUG) 
#   undef CL_Version
#   define CL_Version    CL_Version_Debug
#endif

/*#####################################################################################*/
//cl_dbg �������Դ���
/*#####################################################################################*/
#if CL_Version == CL_Version_Debug 
#define CL_DBG(x) do{if(!(x)) {char* _XXXXX = nullptr; *_XXXXX = 0;} } while(false)
#else
#define CL_DBG(x) 
#endif
 
#define _out  
/*#####################################################################################*/
//����
/*#####################################################################################*/
#if CL_Compiler == CL_Compiler_VC  
#pragma warning(disable:6011) //ȡ������ NULL ָ��
#	if CL_Version == CL_Version_Debug
#	pragma comment(lib, "CLD.lib")
#	else
#	pragma comment(lib, "CL.lib")
#	endif
#endif

#endif//__cl_base_cfg__