#ifndef __clan_base_concept__
#define __clan_base_concept__

#include <concepts>
#include "3type_traits.h"

namespace cl
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
    concept IntAndFloatType = IsInt<T>::value || IsFloat<T>::value;
     
    template<typename T>
    concept NotFloatType = IsChar<T>::value || IsBool<T>::value || IsInt<T>::value;

    template<typename T>
    concept NotCharType = IsFloat<T>::value || IsBool<T>::value || IsInt<T>::value;

    template<typename T>
    concept ValType = IsVal<T>::value;
     
    //字符串数据
    template<typename T>
    concept CharsType = IsChars<T>::value;

    //字符串数据
    template<typename T>
    concept ToCharsType = ToChars<T, char>::value || ToChars<T, wchar>::value;

    //非字符串数据, concept NotCharsType = !CharsType<T>; 注意这样的写法编译没问题, 但是实际中不起作用
    template<typename T>
    concept NotCharsType = IsNotChars<T>::value;
      
    //是否是指针, 但不包含字符串
    template<typename T>
    concept PtrType = std::is_pointer<T>::value && IsNotChars<T>::value;
      
    template<typename T>
    concept ClassType = IsClass<T>::value;

    //成员函数类型
    template<typename T>
    concept ClassFunType = IsClassFun<T>::value;

    template<typename T>
    concept MoveType = MoveCheck<T>::value;

    //定义内存分配器
    //这样很多对象操作可以在本库进行定义,但是涉及内存操作的地方可以在后面再定义
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
    //    char* alloc(s64 size) { return (char*)::malloc(size); }
    //    char* realloc(void* p, s64 size) { return (char*)::realloc(p, size); }
    //    void free(void* p) { ::free(p); }
    //};

}

#endif//__clan_base_concept__ 