#ifndef __cl_task_lock__
#define __cl_task_lock__

#include "../base/1cfg.h"

#if CL_Sys != CL_Sys_Win
#include <atomic> 
#endif

namespace cl
{  
	//原子锁,不休眠当前线程
	class SpinLock
	{
#if CL_Sys != CL_Sys_Win
		std::atomic_flag flag_;
#else
		long flag_ = 0;
#endif
	public:
		SpinLock() {}
		void lock();
		void unlock();
	}; 

	template<typename T>
	struct LockGuardImplement
	{
		T& lock_;
		LockGuardImplement(T& lock) : lock_(lock) { lock_.lock(); }
		~LockGuardImplement() { lock_.unlock(); }
	};

#define __LockGuard(x, line)  cl::LockGuardImplement<decltype(x)> _##line(x)  //LockGuard _123(x)
#define _LockGuard(x, line)   __LockGuard(x, line)                            //__LockGuard(x, 123)
#define LockGuard(x)          _LockGuard(x, __LINE__)                         //_LockGuard(x, __LINE__)
}

#endif//__cl_set_base__