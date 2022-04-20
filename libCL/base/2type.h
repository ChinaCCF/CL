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
typedef unsigned long uv32_2;

typedef int8_t   sv8;
typedef int16_t  sv16;
typedef int32_t  sv32;
typedef int64_t  sv64;

typedef float    fv32;
typedef double   fv64;

typedef wchar_t   wchar;
typedef char      ac8;  //ascii
typedef char8_t   uc8;  //utf8 prefix : u8
typedef char16_t  uc16; //��Windows��char16_t == wchar_t  prefix : u
typedef char32_t  uc32; //prefix : U

typedef uc8       tc8;
typedef wchar     tc16;
typedef uc32      tc32;

#define CL_TC8(x)   u8##x
#define CL_TC16(x)  L##x

constexpr sv8   max_sv8 = (sv8)0x7F;
constexpr sv16 max_sv16 = (sv16)0x7FFF;
constexpr sv32 max_sv32 = (sv32)0x7FFFFFFF;
constexpr sv64 max_sv64 = (sv64)0x7FFFFFFFFFFFFFFF;

constexpr sv8   min_sv8 = (sv8)0x80;
constexpr sv16 min_sv16 = (sv16)0x8000;
constexpr sv32 min_sv32 = (sv32)0x80000000;
constexpr sv64 min_sv64 = (sv64)0x8000000000000000;

constexpr uv8   max_uv8 = (uv8)0xFF;
constexpr uv16 max_uv16 = (uv16)0xFFFF;
constexpr uv32 max_uv32 = (uv32)0xFFFFFFFF;
constexpr uv64 max_uv64 = (uv64)0xFFFFFFFFFFFFFFFF;

#if CL_Bits == CL_Bits_64 
typedef sv64 svt;
typedef uv64 uvt;
constexpr uvt max_uvt = max_uv64;
constexpr svt max_svt = max_sv64;
#else  
typedef sv32 svt;
typedef uv32 uvt;
constexpr uvt max_uvt = max_uv32;
constexpr svt max_svt = max_sv32;
#endif


namespace cl
{
	//�����Զ������л��ͷ����л�
	enum class TypeId
	{
		None = 0,//û���κ�ֵ, ͨ��������ʾ���л�����
		Null,//�����������, ������һ����ֵ
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
		C8,//�ַ�
		C16,
		C32,
		AC8,//ac8�ַ���, typeid + 16λ���� + "String", 16λ���Ȳ������ַ����ս�β
		UC8,//ͬ��
		UC16,//ͬ��
		UC32,//ͬ��
		Fv32,
		Fv64,
		Fv128, 
		Arr,//����  typeid + Ԫ�ظ��� + [(Type), (Type), ...]
		Map,//ӳ��  typeid + Ԫ�ظ��� + { (UCs8) : (Type), (UCs8) : (Type), ...}
		Bin,//��ʾ����������  typeid + 32λ���� + bin_data
		TypeIdEnd,
	};
	/*#####################################################################################*/
	//���� ���ܿ�������, ���ܿ�����ֵ�Ķ��� 
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

	//����C++ʹ���˹��캯���͸�ֵ�������޷���ֵ����, �����쳣������C++����ȽϺõĴ�����ʽ 
	class Exception : public std::exception
	{
	public:
		uv16 main_code_ = 0;
		uv16 sub_code_ = 0;
		uv32 line_ = 0;
		tc16* msg_ = nullptr;
		tc16* file_ = nullptr;
	private:
		tc16 buf_[512];
	public:
		Exception() { buf_[0] = 0; }
		Exception( const tc16* file,  uv32 line,  uv16 main_code,  uv16 sub_code,  const tc16* msg);

		Exception(const Exception& e) = delete;
		Exception& operator=(const Exception& e) = delete;

		const tc16* msg() const { return msg_; }
		const tc16* file() const { return file_; }
		sv32 line() const { return line_; }
		sv32 main_code() const { return main_code_; }
		sv32 sub_code() const { return sub_code_; }

		virtual char const* what() const override { return "cl_exception_2D5A1B2F"; }
	};

#define _CL_Throw(file, MainCode, SubCode, Msg) do{throw cl::Exception(CL_TC16(file), __LINE__, MainCode, SubCode, CL_TC16(Msg));}while(0)
#define CL_Throw(MainCode, SubCode, Msg) _CL_Throw(__FILE__, (uv16)MainCode, (uv16)SubCode, Msg)

#if CL_Version == CL_Version_Debug 
#   define CL_Assert(x) do{if(!(x)) {tc8* _XXXXX = nullptr; *_XXXXX = 0;} } while(0)
#else
#   define CL_Assert(x) do{if(!(x)) CL_Throw(cl::ExceptionCode::Debug, 0, #x); } while(0)
#endif
}
#endif//__cl_base_type__ 