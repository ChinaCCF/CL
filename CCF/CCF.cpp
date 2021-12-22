#include <Windows.h>
#include <iostream> 
#include <CL/base/2type.h>
#include <CL/base/8cstr.h>

//#include "test_cl/3type_traits.h"
//#include "test_cl/4val.h"
//#include "test_cl/5mem.h"
//#include "test_cl/6ptr.h"
//#include "test_cl/8cstr.h"

int main()
{     
    CL_Throw(0,0, "ccf");
    uc16 buf[32];
    cl::scopy(buf, 32, "123");
    const char8_t* str = u8"utf8 陈成发";
    const char* str2 = "utf8 陈成发";

    const char16_t* str3 = u"utf8 陈成发";
    const wchar_t* str4 = L"utf8 陈成发";
    //Fun(1);
    //Fun(2, 3);
    //test::test_type_traits();
    //test::test_val();
    //test::test_mem();
    //test::test_ptr();
    //test::test_cstr();
    //test::test_str();
    std::cout << "Hello World!\n";
    return 0;
}