#ifndef __wl_base_ptr__
#define __wl_base_ptr__

#include "1type.h"
#include <CL/base/3type_traits.h>

namespace wl
{ 
	struct Fun_CloseHanle { void operator()(HANDLE ptr); };

	template<typename T, class Free> 
	requires cl::IsPtr_v<T>
	class Ptr : public cl::NoCopyObj
	{
	public:
		T ptr_ = nullptr; 

		Ptr() {}
		Ptr(const std::nullptr_t&) {}
		Ptr(T p) : ptr_(p) {}
		Ptr(Ptr&& pt) noexcept : ptr_(pt.ptr_) { pt.ptr_ = nullptr; }

		~Ptr() { Free()(ptr_); }

		bool valid() { return ptr_ != nullptr; }

		void operator=(T p) { ~Ptr(); ptr_ = p; }

		operator bool() const { return ptr_ != nullptr; }
		operator T () const { return ptr_; }
		operator T* () const { return &ptr_; } 

		T operator->() const { return ptr_; } 
		T* operator&() { return &ptr_; }

		bool operator==(const std::nullptr_t&) { return ptr_ == nullptr; }
		bool operator!=(const std::nullptr_t&) { return ptr_ != nullptr; }

		template<typename R> bool operator==(R p) { return ptr_ == (T)p; }
		template<typename R> bool operator!=(R p) { return ptr_ != (T)p; }
	};
}

#endif//__wl_base_ptr__