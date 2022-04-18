#include <Windows.h>
#include "../thread.h"

namespace cl
{
	void Thread::yield()
	{
		//通过 pause 指令告诉线程, 正在自旋循环等待, 
		//避免内存顺序违规和大幅耗电
#if CL_Sys == CL_Sys_Win
		_mm_pause(); 
#else

#	if CL_Sys == CL_Sys_Linux
		__asm__ volatile("pause" ::: "memory");
#	else
		struct timespec ts = { 0 };
		nanosleep(&ts, 0);
#	endif 

#endif
	}

	void Thread::resume(uvt thread_handle)
	{
		ResumeThread((HANDLE)thread_handle);
	}
}