#include <libCL/base/6ptr.h>

namespace test
{
    namespace ntest_ptr
    {
        class A
        {
        public:
            int i;
            A()
            {
                i = 3;
                int j = 0;
            }
            ~A()
            {
                int j = 0;
            }
        }; 
        
    }
    void test_ptr()
    {
        {
            //cl::Ptr<ntest_ptr::A> ptr = cl::ObjAllocator<ntest_ptr::A>().alloc();
        }
        {
            //cl::PtrCnt<ntest_ptr::A> ptr = cl::ObjAllocator<ntest_ptr::A>().alloc();
        }
        {
            //cl::PtrCnt<ntest_ptr::A> ptr = cl::PtrCnt<ntest_ptr::A>::make();
        }
        {
            cl::PtrCnt<ntest_ptr::A> ptr = cl::PtrCnt<ntest_ptr::A>::make();
            auto ptr2 = ptr;
        }
    }
}