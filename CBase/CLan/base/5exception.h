#ifndef __clan_base_exception__
#define __clan_base_exception__

#include <exception>
#include "4concept.h"

//异常和错误代码是C/C++中最常见的错误处理方式
//由于C++使用了构造函数和赋值函数等无返回值函数, 所以异常处理是C++代码比较好的错误处理方式
namespace cl
{  
    //由于跨语言需要, 基本大部分的编程环境都已经转换为wchar编码, window的底层驱动强制所有字符串必须是wchar
    class Exception : public std::exception
    {
        s32 line_ = 0;
        s32 code_ = 0; //高16位为主代码, 低16位为次代码
        wchar msg_[256];//存放原始信息
        wchar file_[256]; 
    public: 
        Exception() { msg_[0] = 0; file_[0] = 0; } 
        Exception(const Exception& e); 
        Exception(const wchar* file, s32 line, s32 main_code, s32 sub_code, const wchar* msg);

        Exception& operator=(const Exception& e) { new(this)Exception(e); return *this; }

        const wchar* msg() const { return msg_; }
        const wchar* file() const { return file_; }
        s32 line() const { return line_; }
        s32 main_code() const { return (code_ & 0xFFFF0000) >> 16; }
        s32 sub_code() const { return code_ & 0xFFFF; }

        //std::exception 定义的接口
        virtual char const* what() const override { return "using msg, file instace!"; }
    };
       
#define	cl_throw(main_code, sub_code, msg) do{throw cl::Exception(_cl_W(__FILE__), __LINE__, main_code, sub_code, msg);}while(0)

#if CL_Version == CL_Version_Debug 
#   define cl_assert(x) do{if(!(x)) {char* _XXXXX = nullptr; *_XXXXX = 0;} } while(0)
#else
#   define cl_assert(x) do{if(!(x)) cl_throw(0, 0, _cl_W(#x)); } while(0)
#endif

    //使用示例
    // try
    //{
    //    clan_throw(0, 0, L"123");
    //}catch(std::exception& e) //注意最好使用引用, 否则Exception会发生拷贝, 导致性能损失
    //{ 
    //    int i = 0;
    //}
}
 

#endif//__clan_base_exception__ 