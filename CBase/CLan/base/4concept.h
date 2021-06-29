#ifndef __clan_base_concept__
#define __clan_base_concept__

#include <concepts>
#include "3type_traits.h"

namespace clan
{ 
    template<typename T>
    concept CharType = IsChar<T>::value;

    template<typename T>
    concept BoolType = IsBool<T>::value;

    template<typename T>
    concept FloatType = IsFloat<T>::value;

    template<typename T>
    concept UintType = IsUnsignedInt<T>::value;

    template<typename T>
    concept SintType = IsSignedInt<T>::value;
      
    template<typename T>
    concept IntType = IsInt<T>::value;
     
    template<typename T>
    concept ValType = IsVal<T>::value;

    template<typename T>
    concept ValExFloatType = IsChar<T>::value || IsBool<T>::value || IsInt<T>::value;

    template<typename T>
    concept CharsType = 
        IsSameType<typename RawType<T>::type, char*>::value ||
        IsSameType<typename RawType<T>::type, wchar*>::value;

    template<typename T>
    concept NotCharsType = !CharsType<T>;

    template<typename T>
    concept StrType = 
        std::is_convertible<T, char*>::value || 
        std::is_convertible<T, const char*>::value ||
        std::is_convertible<T, wchar*>::value ||
        std::is_convertible<T, const wchar*>::value;
     
    template<typename T>
    concept ClassType = IsClass<T>::value;

    template<typename T>
    concept ClassFunType = IsClassFun<T>::value;

    template<typename T>
    concept MoveType = MoveCheck<T>::value;

    //定义内存分配器
    template<typename A>
    concept AllocMemType = requires(A a, s64 size, void* p)
    {
        a.alloc(size);//具备alloc 指定大小函数
        a.realloc(p, size);//具备realloc函数
        requires IsSameType<char*, decltype(a.alloc(1))>::value;//确保申请函数返回char* 指针
        requires IsSameType<char*, decltype(a.realloc(p,1))>::value;//确保申请函数返回char* 指针
        a.free(p);//释放任意大小指针空间
    };

    //常规的内存申请器 
    //class AllocMem
    //{
    //public:
    //	char* alloc(s64 size) { return (char*)malloc(size); }
    //	char* realloc(void* p, s64 size) { return (char*)realloc(p, size); }
    //	void free(void* p) { ::free(p); }
    //};
}

#endif//__clan_base_concept__ 