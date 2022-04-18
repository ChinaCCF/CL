#include <Windows.h>
#include "../thread.h"

namespace cl
{
	void Thread::yield()
	{
		//ͨ�� pause ָ������߳�, ��������ѭ���ȴ�, 
		//�����ڴ�˳��Υ��ʹ���ĵ�
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