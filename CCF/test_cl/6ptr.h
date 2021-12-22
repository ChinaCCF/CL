#include <CL/base/6ptr.h>

namespace test
{
    namespace ptr
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
            //cl::Ptr<ptr::A> ptr = cl::ObjAllocator<ptr::A>().alloc();
        }
        {
            //cl::PtrCnt<ptr::A> ptr = cl::ObjAllocator<ptr::A>().alloc();
        }
        {
            //cl::PtrCnt<ptr::A> ptr = cl::PtrCnt<ptr::A>::make();
        }
        {
            cl::PtrCnt<ptr::A> ptr = cl::PtrCnt<ptr::A>::make();
            auto ptr2 = ptr;
        }
    }
}