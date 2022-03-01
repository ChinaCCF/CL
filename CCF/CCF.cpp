#include <Windows.h>
#include <iostream>

#pragma warning(disable:4996)
#include <CL/base/2type.h>
#include <CL/base/8cstr.h>

#include "test_cl/3type_traits.h"
#include "test_cl/4val.h"
#include "test_cl/5mem.h"
#include "test_cl/6ptr.h"
#include "test_cl/7call.h"
#include "test_cl/8cstr.h"
#include "test_cl/9str.h"

int main()
{     
    uv8 _dst[4096];
    uv8 _src[4096];

    int kkk = 0;
    { 
        for (int i = 0; i < 99999; i++)
        {
            uv8* dst = _dst;
            uv8* src = _src;
            int c = 4096;
            while (c--) *dst++ = *src++;
        } 
    }
    int lll = 0;
    { 
        for (int i = 0; i < 99999; i++)
        {
            uv8* dst = _dst;
            uv8* src = _src;
            for (int c = 0; c < 4096; c++)
            {
                dst[c] = src[c];
            }
        }
    }
    int ddd = 0;

    test::test_call();
       
    test::test_type_traits();
    test::test_val();
    test::test_mem();
    test::test_ptr();
    test::test_cstr();
    test::test_str();
    std::cout << "Hello World!\n";
    return 0;
}