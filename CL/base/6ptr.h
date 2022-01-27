#ifndef __cl_base_ptr__
#define __cl_base_ptr__

#include "5mem.h"

namespace cl
{
	//内存指针 #include<memory>
	//share_ptr 是一个多线程共享安全的指针, 这带来了些性能的损失 
	//下面这些对象都是非线程安全的

	/*############################################################################################*/
	//指针对象, 用来保存指针
	/*############################################################################################*/
	template<typename T, template<typename R> class Allocator = ObjAllocator> requires ObjAllocType<Allocator, T>
	class Ptr : public NoCopyObj
	{
	public:
		T* ptr_ = nullptr;//对象指针

		Ptr() {}
		Ptr(const std::nullptr_t&) {}
		Ptr(T* p) : ptr_(p) {}
		Ptr(Ptr&& pt) noexcept : ptr_(pt.ptr_) { pt.ptr_ = nullptr; }

		~Ptr() { if(ptr_) Allocator<T>().free(ptr_); }

		bool valid() { return ptr_ != nullptr; }

		void operator=(T* p) { ~Ptr(); ptr_ = p; }

		operator bool() const { return ptr_ != nullptr; }
		operator T* () const { return ptr_; }
		operator T** () const { return &ptr_; }
		operator void* () const { return ptr_; }

		T* operator->() const { return ptr_; }
		T& operator*() const { return *ptr_; }
		T** operator&() { return &ptr_; }
		 
		bool operator==(const std::nullptr_t&) { return ptr_ == nullptr; }
		bool operator!=(const std::nullptr_t&) { return ptr_ != nullptr; }
		template<typename R> bool operator==(R* p) { return ptr_ == (T*)p; }
		template<typename R> bool operator!=(R* p) { return ptr_ != (T*)p; }
	};

	/*############################################################################################*/
	//计数指针对象
	/*############################################################################################*/
	namespace lib
	{
		template<typename T>
		class iHolder : NoCopyObj
		{
		public:
			uv32 cnt_ = 0;
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
			HoldObj() { iHolder<T>::ptr_ = &obj_; }
			virtual ~HoldObj() {}
		};

		template<typename T, template<typename> class Allocator = ObjAllocator> requires ObjAllocType<Allocator, T>
		class HoldPtr : public iHolder<T>
		{
		public:
			HoldPtr() {}
			virtual ~HoldPtr()
			{
				if(iHolder<T>::ptr_) Allocator<T>().free(iHolder<T>::ptr_);
			}
		};
	}

	//对象的申请器, 和容器的申请器可以不一致
	template<typename T,
		template<typename> class AllocatorHolder = ObjAllocator,
		template<typename> class AllocatorT = ObjAllocator>
		requires ObjAllocType<AllocatorHolder, lib::iHolder<T>>&& ObjAllocType<AllocatorT, T>
	class PtrCnt
	{
	public:
		using iHolder = lib::iHolder<T>;
		using HoldPtr = lib::HoldPtr<T, AllocatorT>;
	private:
		iHolder* iholder_ = nullptr;

		//强制转换容器指针, 例如父类A, 子类B, A转换到B, iHolder<A> => iHolder<B>
		template<typename R>
		void retain(lib::iHolder<R>* holder)
		{
			iholder_ = (iHolder*)holder;
			++iholder_->cnt_;
		}

		PtrCnt(iHolder* p) { if(p == nullptr) return; iholder_ = p; iholder_->cnt_ = 1; }
	public:
		PtrCnt() {}
		PtrCnt(const std::nullptr_t&) {}
		PtrCnt(T* p)
		{
			iholder_ = AllocatorHolder<HoldPtr>().alloc();
			iholder_->ptr_ = p;
			iholder_->cnt_ = 1;
		}
		PtrCnt& operator=(T* p) { release(); new(this)PtrCnt(p); return *this; }
		PtrCnt(const PtrCnt& p) { retain(p.iholder_); }

		template<typename R, template<typename> class AllocatorT2 = ObjAllocator> requires ObjAllocType<AllocatorT2, R>
		PtrCnt(const PtrCnt<R, AllocatorHolder, AllocatorT2>& p) { retain(p.iholder_); }

		template<typename R, template<typename> class AllocatorT2 = ObjAllocator> requires ObjAllocType<AllocatorT2, R>
		PtrCnt& operator=(const PtrCnt<R, AllocatorHolder, AllocatorT2>& p) { release(); new(this)PtrCnt(p); return *this; }

		~PtrCnt() { release(); }

		void release()
		{
			if(iholder_)
			{
				--iholder_->cnt_;
				if(iholder_->cnt_ == 0) AllocatorHolder<iHolder>().free(iholder_);
				iholder_ = nullptr;
			}
		}

		uv32 cnt() const { return iholder_->cnt_; }
		bool valid() { return iholder_ != nullptr; }

		operator bool() const { return iholder_ != nullptr; }
		operator T* () const { return iholder_->ptr_; }
		operator T** () { return &iholder_->ptr_; }
		operator void* () const { return iholder_->ptr_; }

		T* operator->() const { return iholder_->ptr_; }
		T& operator*() const { return *iholder_->ptr_; }
		T&& operator&() { return &iholder_->ptr_; }

		bool operator==(const std::nullptr_t&) const { return iholder_ == nullptr; }
		bool operator!=(const std::nullptr_t&) const { return iholder_ != nullptr; }

		bool operator==(T* p) const { return iholder_->ptr_ == p; }
		bool operator!=(T* p) const { return iholder_->ptr_ != p; }

		static inline auto make() { return PtrCnt(AllocatorHolder<lib::HoldObj<T>>().alloc()); }
	};
}

#endif//__cl_base_ptr__ 