#ifndef __cl_exception_code__
#define __cl_exception_code__
 
namespace cl
{
	enum class ExceptionCode
	{
		Debug = 0,

		Memory_Alloc_Fail =  10,
		Buffer_OverRead = 11,
		Buffer_OverWrite = 12,

		Float_Format_Error = 15,
		Convert_Error = 16,

		IO_Open_Fail = 20, 
		IO_Write_Fail = 21,
		IO_Read_Fail = 22,
	};
}

#endif//__cl_exception_code__ 