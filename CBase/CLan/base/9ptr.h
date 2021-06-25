#ifndef __clan_ptr__
#define __clan_ptr__

#include "8obj.h"
#include "4concept.h"

namespace clan
{
	//内存指针 #include<memory>
	//share_ptr 是一个多线程共享安全的指针, 这就带来了性能的损失
	//然后auto_ptr是使用默认的new 和 delete内存管理的, 对于一些特许要求可能会不合适,例如不想使用默认内存操作
	//下面这些对象都是非线程安全的

	//常规的内存申请器 
	//class AllocMem
	//{
	//public:
	//	char* alloc(s64 size) { return malloc(size); }
	//	void free(void* p) { ::free(p); }
	//};

	//常规的对象申请器
	//template<typename T>
	//class AllocObj
	//{
	//public:
	//	static T* alloc() { auto p = (T*)malloc(sizeof(T)); new(p)T(); return p; }
	//	static void free(T* p) { p->~T(); ::free(p); }
	//};

	/*############################################################################################*/
	//指针对象, 用来保存指针
	/*############################################################################################*/
	template<typename T, typename A>
	requires AllocObjType<A, T> || AllocMemType<A>
		class _Ptr : public NoCopyObj
	{
	public:
		T* obj_ = nullptr;//对象指针

		_Ptr() { clan_CheckClass(_Ptr); }
		_Ptr(T* p) : obj_(p) {}
		_Ptr(_Ptr&& pt) : obj_(pt.obj_) { pt.obj_ = nullptr; }
		~_Ptr() { if (obj_) A().free(obj_); }

		void operator=(T* p) { if (obj_) A().free(obj_); obj_ = p; }
		operator T* () const { return obj_; }
		T* operator->() const { return obj_; }
		T& operator*() const { return *obj_; }
	};

	/*############################################################################################*/
	//计数指针对象
	/*############################################################################################*/
	namespace detail
	{
		template<typename T>
		class iHolder : NoCopyObj
		{
		public:
			s32 cnt_ = 0;
			T* ptr_ = nullptr;//对象指针

			iHolder() {}
			virtual ~iHolder() {}
		};
		//优化内存和速度, 把实际的对象和计数对象绑定在一起
		template<typename T>
		class HoldObj : public iHolder<T>
		{
			T obj_;
		public:
			HoldObj()
			{
				new(&obj_)T();
				iHolder<T>::ptr_ = &obj_;
			}
			virtual ~HoldObj() {}//无需手动调用obj_的析构函数
		};

		template<typename T, typename A>
		requires AllocObjType<A, T> || AllocMemType<A>
			class HoldPtr : public iHolder<T>
		{
		public:
			HoldPtr(T* p) { iHolder<T>::ptr_ = p; }
			virtual ~HoldPtr() { if (iHolder<T>::ptr_) A().free(iHolder<T>::ptr_); }
		};
	}

	//和_Ptr一样需要自己重新定义
	template<typename T, typename AllocObj, typename AllocHolder>
	requires (AllocObjType<AllocObj, T> || AllocMemType<AllocObj>) && AllocMemType<AllocHolder>
		class _PtrCnt
	{
	public:
		using Holder = detail::iHolder<T>;
		using HoldPtr = detail::HoldPtr<T, AllocObj>;
	private:
		//强制转换容器指针, 例如父类A, 子类B, A转换到B, iHolder<A> => iHolder<B>
		template<typename R>
		void _retain(detail::iHolder<R>* holder)
		{
			holder_ = (Holder*)holder;
			++holder_->cnt_;
		}
		void _release()
		{
			if (holder_)
			{
				--holder_->cnt_;
				if (holder_->cnt_ <= 0)
				{
					holder_->~Holder();
					AllocHolder().free(holder_);
				}
				holder_ = nullptr;
			}
		}
	public:
		Holder* holder_ = nullptr;

	public:
		_PtrCnt() { clan_CheckClass(_PtrCnt); }
		_PtrCnt(T* p)
		{
			holder_ = (Holder*)AllocHolder().alloc(sizeof(HoldPtr));
			new(holder_)HoldPtr(p);
			holder_->cnt_ = 1;
		}
		_PtrCnt(const _PtrCnt& p) { _retain(p.holder_); }

		~_PtrCnt() { _release(); }

		_PtrCnt& operator=(T* p) { _release(); new(this)_PtrCnt(p); return *this; }
		_PtrCnt& operator=(const _PtrCnt& p) { _release(); new(this)_PtrCnt(p); return *this; }

		void release() { _release(); }

		s32 cnt() const { return holder_->cnt_; }
		operator bool() const { return holder_ != nullptr; }

		bool operator==(std::nullptr_t) const { return holder_ == nullptr; }
		bool operator!=(std::nullptr_t) const { return holder_ != nullptr; }
		bool operator==(T* p) const { if (!holder_) return false; return holder_->ptr_ == p; }
		bool operator!=(T* p) const { if (!holder_) return false; return holder_->ptr_ != p; }

		operator T* () const { return holder_->ptr_; }
		T* operator->() const { return holder_->ptr_; }
		T& operator*() const { return *holder_->ptr_; }
	};

	template<typename T, typename AllocHolder>
	auto make_ptr()
	{
		using HoldObj = detail::HoldObj<T>;
		_PtrCnt<T, AllocHolder, AllocHolder> ptr;
		ptr.holder_ = (HoldObj*)AllocHolder().alloc(sizeof(HoldObj));
		new(ptr.holder_)HoldObj();
		ptr.holder_->cnt_++;
		return ptr;
	}
}

#endif//__clan_ptr__ 