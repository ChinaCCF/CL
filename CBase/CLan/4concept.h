#ifndef __clan_concept__
#define __clan_concept__

#include "3type_traits.h"

namespace clan
{ 
    template<typename T>
    concept CharsType = IsChars<T>::value;

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
    concept MoveType = MoveCheck<T>::value;

    //定义内存分配器
    template<typename A>
    concept AllocMemType = requires(A a, s64 size, void* p)
    {
        a.alloc(size);//具备alloc 指定大小函数
        requires IsSameType<char*, decltype(a.alloc(1))>::value;//确保申请函数返回char* 指针
        a.free(p);//释放任意大小指针空间
    };

    //定义对象分配器
    template<typename A, typename T>
    concept AllocObjType = requires(A a, T * p)
    {
        a.alloc(); 
        requires IsSameType<T*, decltype(a.alloc())>::value; 
        a.free(p);
    };
}

#endif//__clan_concept__ 