#include <libCL/base/5mem.h>

namespace test
{
    namespace ntest_mem
    {
        class A
        {
        public:
            int i;
            A()
            {
                int j = 0;
            }
            ~A()
            {
                int j = 0;
            }
        };

        template<class Allocator = cl::MemAllocator>
            requires cl::MemAllocType<Allocator>
        class B
        {
        public:
            uv8* a;
            B()
            {
                a = Allocator().alloc(4);
            }
            //~B() { Pool<A>::free(a); }
        };
    }
    void test_mem()
    {
        ntest_mem::B<> b;
    }
}