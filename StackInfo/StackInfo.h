#ifndef __dll_StackInfo__
#define __dll_StackInfo__

#include <libCL/base/1cfg.h>

#if CL_Compiler == CL_Compiler_VC

#	ifndef StackInfo_DLL
#		if CL_Version == CL_Version_Debug  
#			pragma comment(lib, "StackInfoD.lib")
#		else
#			pragma comment(lib, "StackInfo.lib")
#		endif 
#	endif

#	ifdef StackInfo_DLL
#		define stack_api __declspec(dllexport)
#	else
#		define stack_api __declspec(dllimport)
#	endif  

#endif

#include <libCL/base/2type.h>
#include <libCL/base/9str.h>

//DLL不要暴露对象的构造和析构函数, 否则可能会导致
//_ASSERTE(__acrt_first_block == header);
namespace cl
{
	struct FrameInfo
	{
		uv32 line_ = 0;
		wchar* file_ = nullptr;
		wchar* fun_ = nullptr;
	};

	struct StackInfo
	{
		uv32 size_ = 0;
		uv8 buf_[1];
	};

	stack_api void stack_init(const wchar* pdb_dir = nullptr);
	stack_api void stack_uninit();

	stack_api StackInfo* stack_alloc();
	stack_api void stack_free(StackInfo* nodes);

	stack_api FrameInfo* frame_alloc(StackInfo* stack, uv32 index);
	stack_api void frame_free(FrameInfo* frame);
}

#endif//__dll_StackInfo__