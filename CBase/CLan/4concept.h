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
}

#endif//__clan_concept__ 