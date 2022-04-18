#include <libCL/set/4heap.h>

namespace test
{
	namespace ntest_heap
	{

	}

	void test_heap()
	{
		cl::HeapArr<int> heap;
		heap.push(1);
		heap.push(2);
		heap.push(3);
		heap.push(4);
		heap.push(5);
		heap.push(6);

		heap.pop();
		heap.pop();
		heap.pop();
		heap.pop();
		heap.pop();
		heap.pop();

		int i = 0;
	}
}