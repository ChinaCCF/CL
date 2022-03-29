#if CL_Sys == CL_Sys_Win 
#include <Windows.h>
#endif

#include "../lock.h"
#include "../thread.h"
namespace cl
{
#if CL_Sys != CL_Sys_Win
	void SpinLock::lock()
	{ 
		while (flag_.test_and_set(std::memory_order_acquire)) Thread::yield();
	}
	void SpinLock::unlock() { flag_.clear(std::memory_order_release); }
#else
	void SpinLock::lock() 
	{ 
		while (1 == InterlockedCompareExchange(&flag_, 1, 0)) Thread::yield();
	}
	void SpinLock::unlock() { flag_ = 0; }
#endif
}