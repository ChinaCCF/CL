#ifndef __cl_base_type__
#define __cl_base_type__

#include <stdint.h> 
#include <float.h>   
#include <exception>
#include "1cfg.h"
#include "_exception_code.h"

typedef uint8_t  uv8;
typedef uint16_t uv16;
typedef uint32_t uv32;
typedef uint64_t uv64;

typedef int8_t   sv8;
typedef int16_t  sv16;
typedef int32_t  sv32;
typedef int64_t  sv64;

typedef float    fv32;
typedef double   fv64;

typedef char      ac8;  //ascii
typedef char8_t   uc8;  //utf8 
typedef char16_t  uc16; //在Windows上char16_t == wchar_t 

#define CL_TXT8(x)  u8##x
#define CL_TXT16(x) u##x

static constexpr sv8   max_sv8 = (sv8)0x7F;
static constexpr sv16 max_sv16 = (sv16)0x7FFF;
static constexpr sv32 max_sv32 = (sv32)0x7FFFFFFF;
static constexpr sv64 max_sv64 = (sv64)0x7FFFFFFFFFFFFFFF;

static constexpr sv8   min_sv8 = (sv8)0x80;
static constexpr sv16 min_sv16 = (sv16)0x8000;
static constexpr sv32 min_sv32 = (sv32)0x80000000;
static constexpr sv64 min_sv64 = (sv64)0x8000000000000000;

static constexpr uv8   max_uv8 = (uv8)0xFF;
static constexpr uv16 max_uv16 = (uv16)0xFFFF;
static constexpr uv32 max_uv32 = (uv32)0xFFFFFFFF;
static constexpr uv64 max_uv64 = (uv64)0xFFFFFFFFFFFFFFFF;

#if CL_Bits == CL_Bits_64 
typedef sv64 svt;
typedef uv64 uvt;
static constexpr uvt max_uvt = max_uv64;
static constexpr svt max_svt = max_sv64;
#else  
typedef sv32 svt;
typedef uv32 uvt;
static constexpr uvt max_uvt = max_uv32;
static constexpr svt max_svt = max_sv32;
#endif


namespace cl
{
	//用于自定义序列化和反序列化
	enum class TypeId
	{
		None = 0,//没有任何值, 通常用来表示序列化结束
		Null,//表有这个变量, 但是是一个空值
		Bool,
		Uv8,
		Uv16,
		Uv32,
		Uv64,
		Uv128,
		Sv8,
		Sv16,
		Sv32,
		Sv64,
		Sv128,
		C8,//字符
		C16,
		C32,
		ACs8,//ac8字符串, typeid + 16位长度 + "String", 16位长度不包括字符串空结尾
		UCs8,//同上
		Cs16,//同上
		Cs32,//同上
		Fv32,
		Fv64,
		Fv128, 
		Arr,//数组  typeid + 元素个数 + [(Type), (Type), ...]
		Map,//映射  typeid + 元素个数 + { (UCs8) : (Type), (UCs8) : (Type), ...}
		Bin,//表示二进制数据  typeid + 32位长度 + bin_data
		TypeIdEnd,
	};
	/*#####################################################################################*/
	//定义 不能拷贝构造, 不能拷贝赋值的对象 
	/*#####################################################################################*/
	class NoCopyObj
	{
	public:
		NoCopyObj() {}

		NoCopyObj(NoCopyObj&) = delete;
		NoCopyObj(const NoCopyObj&) = delete;

		NoCopyObj& operator=(NoCopyObj&) = delete;
		NoCopyObj& operator=(const NoCopyObj&) = delete;
	};

	//由于C++使用了构造函数和赋值函数等无返回值函数, 所以异常处理是C++代码比较好的错误处理方式 
	class Exception : public std::exception
	{
	public:
		uv16 main_code_ = 0;
		uv16 sub_code_ = 0;
		uv32 line_ = 0;
		uc16* msg_ = nullptr;
		uc16* file_ = nullptr;
	private:
		uc16 buf_[512];
	public:
		Exception() { buf_[0] = 0; }
		Exception(_in const uc16* file, _in uv32 line, _in uv16 main_code, _in uv16 sub_code, _in const uc16* msg);

		Exception(const Exception& e) = delete;
		Exception& operator=(const Exception& e) = delete;

		const uc16* msg() const { return msg_; }
		const uc16* file() const { return file_; }
		sv32 line() const { return line_; }
		sv32 main_code() const { return main_code_; }
		sv32 sub_code() const { return sub_code_; }

		virtual char const* what() const override { return "cl_exception_2D5A1B2F"; }
	};

#define _CL_Throw(file, MainCode, SubCode, Msg) do{throw cl::Exception(CL_TXT16(file), __LINE__, MainCode, SubCode, CL_TXT16(Msg));}while(0)
#define CL_Throw(MainCode, SubCode, Msg) _CL_Throw(__FILE__, (uv16)MainCode, (uv16)SubCode, Msg)

#if CL_Version == CL_Version_Debug 
#   define CL_Assert(x) do{if(!(x)) {uc8* _XXXXX = nullptr; *_XXXXX = 0;} } while(0)
#else
#   define CL_Assert(x) do{if(!(x)) CL_Throw(cl::ExceptionCode::Debug, 0, #x); } while(0)
#endif
}
#endif//__cl_base_type__ 