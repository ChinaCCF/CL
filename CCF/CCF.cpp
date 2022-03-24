#include <Windows.h>
#include <iostream>

#pragma warning(disable:4996) 

#include "test_cl/base/3type_traits.h"
#include "test_cl/base/4val.h"
#include "test_cl/base/5mem.h"
#include "test_cl/base/6ptr.h"
#include "test_cl/base/7call.h"
#include "test_cl/base/8cstr.h"
#include "test_cl/base/9str.h"
#include "test_cl/set/set.h"


int main()
{     


    test::test_type_traits();
    test::test_val();
    test::test_mem();
    test::test_ptr();
    test::test_cstr();
    test::test_call();
    test::test_str();

    test::test_arr();
    test::test_list();
    test::test_heap();
    test::test_map();
    test::test_rbtree();
    std::cout << "Hello World!\n";
    return 0;
}
 