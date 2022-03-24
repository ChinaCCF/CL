#ifndef __cl_exception_code__
#define __cl_exception_code__
 
namespace cl
{
	enum class ExceptionCode
	{
		Debug,
		Memory_Alloc_Fail,
		Buffer_OverRead,
		Buffer_OverWrite,

		Float_Format_Error,
		Convert_Error,
	};
}

#endif//__cl_exception_code__ 