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
	template<typename T> struct Fun_Free { void operator()(T* ptr) { MemAllocator().free(ptr); } }; 
	template<typename T> struct Fun_Del { void operator()(T* ptr) { free_obj<MemAllocator, T>(ptr); } };

	template<typename T, template<typename> class Free = Fun_Free>
	class Ptr : public cl::NoCopyObj
	{
		void _free() { if (ptr_ != T(0)) Free<T>()(ptr_); }
	public:
		T ptr_ = T(0);

		Ptr() {}
		Ptr(const std::nullptr_t&) {}
		Ptr(T p) : ptr_(p) {}
		Ptr(Ptr&& h) noexcept : ptr_(h.ptr_) { h.ptr_ = T(0); }

		~Ptr() { _free(); }

		bool valid() { return ptr_ != T(0); }

		operator bool() const { return ptr_ != T(0); }
		operator T () const { return ptr_; }
		operator T* () const { return &ptr_; }

		T operator->() const { return ptr_; }
		T* operator&() { return &ptr_; }

		void operator=(T p) { _free(); ptr_ = p; }
		void operator=(Ptr&& h) noexcept { _free(); ptr_ = h.ptr_; h.ptr_ = T(0); }

		bool operator==(const std::nullptr_t&) { return ptr_ == nullptr; }
		bool operator!=(const std::nullptr_t&) { return ptr_ != nullptr; }

		template<typename R> bool operator==(R p) { return ptr_ == (T)p; }
		template<typename R> bool operator!=(R p) { return ptr_ != (T)p; }
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

		template<typename T, template<typename> class Free>  
		class HoldPtr : public iHolder<T>
		{
		public:
			HoldPtr() {}
			virtual ~HoldPtr() { Free<T>()(iHolder<T>::ptr_); }
		};
	}

	template<typename T, template<typename> class Free = Fun_Free, class MA = MemAllocator>
		requires MemAllocType<MA>
	class PtrCnt
	{
	public:
		using iHolder = lib::iHolder<T>;
		using HoldPtr = lib::HoldPtr<T, Free>;
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
		//当赋值指针时候, 默认指针的 Allocator 和 PtrCnt 的 Allocator 是一致的
		PtrCnt(T* p)
		{
			iholder_ = alloc_obj<MA, HoldPtr>();
			iholder_->ptr_ = p;
			iholder_->cnt_ = 1;
		}
		PtrCnt(const PtrCnt& p) { retain(p.iholder_); }
		PtrCnt& operator=(T* p) { release(); new(this)PtrCnt(p); return *this; }


		template<typename T2>
		PtrCnt(const PtrCnt<T2, Free, MA>& p) { retain(p.iholder_); }

		template<typename T2>
		PtrCnt& operator=(const PtrCnt<T2, Free, MA>& p) { release(); retain(p.iholder_); return *this; }

		~PtrCnt() { release(); }

		void release()
		{
			if(iholder_)
			{
				--iholder_->cnt_;
				if (iholder_->cnt_ == 0) free_obj<MA>(iholder_);
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

		cl_si auto make() { return PtrCnt(alloc_obj<MA, lib::HoldObj<T>>()); }
	};
}

#endif//__cl_base_ptr__ 