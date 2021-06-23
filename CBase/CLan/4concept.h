#ifndef __clan_concept__
#define __clan_concept__

#include "3type_traits.h"

namespace clan
{ 
    template<typename T>
    concept IntType = IsInt<T>::value;

    template<typename T>
    concept FloatType = IsFloat<T>::value;

    template<typename T>
    concept CharsType = IsChars<T>::value;

    template<typename T>
    concept MoveType = MoveCheck<T>::value;
}

#endif//__clan_concept__ 