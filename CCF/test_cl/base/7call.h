#include <stdio.h>
#include <libCL/base/7call.h>

namespace test
{
    namespace ntest_call
    {
        void normal_fun()
        {
            printf("normal fun call\n");
        }

        class A
        { 
        public:
            int i_ = 0;
            void fun(int i) 
            {
                printf("class fun self i : %d, args i : %d\n", i_, i);
            }
        };
    }
    void test_call()
    {
        {
            cl::Call<void(void)> call = ntest_call::normal_fun;
            call();
        }

        {
            cl::Call<void(void)> call = []() 
            {
                printf("lambda call\n");
            };
            call();
        }
        {
            int i = 2;
            cl::Call<void(void)> call = [&i]()
            {
                printf("lambda call %d\n", i);
            };
            call();
        }

        {
            ntest_call::A a;
            a.i_ = 8;
            cl::Call<void(int)> call;
            call.bind(a, &ntest_call::A::fun);
            call(2);
        }

        int i = 0;
    }
}