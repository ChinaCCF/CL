#ifndef __cl_task_thread__
#define __cl_task_thread__

#include "../base/2type.h"

namespace cl
{
	class Thread
	{
	public:
		//����ֹͣ, ��ŵ�λ��Ϊns
		static void yield();
		static void resume(uvt thread_handle);
	}; 
}

#endif//__cl_task_thread__