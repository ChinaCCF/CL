#ifndef __cl_base_str_view__
#define __cl_base_str_view__
 
#include "8cstr.h"

namespace cl
{
	namespace lib
	{
		template<CharType T>
		class _StrView
		{
		public:
			uv32 len_ = 0;
			T* str_ = nullptr;

			_StrView() { }
			_StrView(const std::nullptr_t&) {}
			_StrView(const T* str) { str_ = (T*)str; len_ = slen(str); }
			_StrView(const _StrView& str) { str_ = (T*)str.data(); len_ = str.length(); }
			_StrView(const T* str, uv32 len) { str_ = (T*)str; len_ = len; }

			template<typename R>
			requires IsNull_v<R> || IsCharPtr_v<R> || IsSame_v<R, _StrView<T>>
			_StrView& operator=(const R& str) { new(this)_StrView(str); return *this; } 
			/*#######################################################################################*/

			uv32 length() const { return len_; }
			const T* data() const { return str_; }
			T back() const { return str_[len_ - 1]; }
			T operator[](uv32 index) const { return str_[index]; }

			operator T* () const { return str_; }
			operator const T* () const { return str_; }

			template<typename V> requires IsInt_v<V> || IsFloat_v<V> || IsBool_v<V>
			operator V() const
			{
				V val;
				auto end = tov(str_, val);
				if(end == nullptr ) CL_Throw(ExceptionCode::Convert_Error, 0, "convert type error");
				return val;
			}

			template<CharType C> bool operator==(const C* t) const { return sequ(str_, t); }
			template<CharType C> bool operator!=(const C* t) const { return !sequ(str_, t); }
			bool operator==(const std::nullptr_t&) const { return len_ == 0; }
			bool operator!=(const std::nullptr_t&) const { return len_ != 0; }
			/*#######################################################################################*/
		private:
			sv32 _find_ret(const T* p) { if (!p) return -1; return sv32(p - str_); }
		public:
			//若查找失败返回-1
			template<CharType C> sv32 find(C c) const { return _find_ret(sfind(str_, c)); }
			template<CharType C> sv32 findr(C c) const { return _find_ret(sfindr(str_, c)); } 
			template<CharType C> sv32 find(uv32 index, C c) const { return _find_ret(sfind(str_ + index, c)); } 
			template<CharType C> sv32 find(const C* sub) const { return _find_ret(sfind(str_, sub));   } 
			template<CharType C> sv32 find(uv32 index, const C* sub) const { return _find_ret(sfind(str_ + index, sub)); }  
		};
	} 
	using StrView = lib::_StrView<ch8>;
	using StrViewW = lib::_StrView<uc16>;

	template<CharType T, MemAllocType MA = DefaultMemAlloc>
	class _NString : public lib::_StrView<T>
	{
		using Base = lib::_StrView<T>;
		using Base::str_;
		using Base::len_;

		uv32 size_ = 0;
		template<typename R> _NString(const R&) {} //禁止一切带参数初始化
	public:
		_NString() {}

		bool need(uv32 size)
		{
			if (str_ == nullptr)
			{
				size_ = align<uv32>(size + 1, 16);
				str_ = (T*)MA::alloc(size_ * sizeof(T)); 
				return str_ != nullptr;
			}
			else
			{
				if (len_ + size + 1 >= size_)
				{
					size_ = align<uv32>(len_ + size + 1, 16);
					auto p = (T*)MA::alloc(size_ * sizeof(T));
					if (p == nullptr) return false;
					if(len_) scopy(p, str_); 
					MA::free(str_);
					str_ = p; 
				}
				return true;
			}
		}

		template<CharType R>
		bool set(const R* str, uv32 len = 0) 
		{
			if(len == 0) len = slen(str);
			if (!need(len - len_)) return false;
			scopy(str_, str);
			len_ = len;
			return true;
		}

		bool set(_NString&& str) noexcept
		{
			str_ = str.str_;
			len_ = str.len_;
			size_ = str.size_;

			str.str_ = nullptr;
			str.size_ = str.len_ = 0;
			return true;
		}

	};

	template<MemAllocType MA = DefaultMemAlloc>
	using _String = _NString<ch8, MA>;

	//using String = _String<>;
}

#endif//__cl_base_str_view__ 