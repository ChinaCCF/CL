#ifndef __dll_StackInfo__
#define __dll_StackInfo__

#include <CL/base/1cfg.h>

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
  
#include <CL/base/2type.h>
#include <CL/base/9str.h>

namespace cl
{ 
    class FrameInfo
    { 
    public:
        uv32 line_ = 0; 
        StringW fun_;
        StringW file_;
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

    stack_api bool frame_info(StackInfo* stack, uv32 index, FrameInfo* frame);

}

#endif//__dll_StackInfo__