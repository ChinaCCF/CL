#ifndef __cl_base_str__
#define __cl_base_str__

#include "8cstr.h"
#include <string>

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
			template<CharType T2> requires (sizeof(T) == sizeof(T2))
				_StrView(const T2* str, sv32 len) { str_ = (T*)str; len_ = len; }

			template<CharType T2>
			_StrView(const _StrView<T2>& str) : _StrView(str.data(), str.length()) { }

			template<CharType T2> requires (sizeof(T) == sizeof(T2))
				_StrView(const T2* str) : _StrView(str, cstr::length(str)) {}

			template<typename R>
				requires IsNull_v<R> || IsCharPtr_v<R> || IsSame_v<R, _StrView<T>>
			_StrView & operator=(const R & str) { new(this)_StrView(str); return *this; }
			/*#######################################################################################*/

			uv32 length() const { return len_; }
			const T* data() const { return str_; }
			T back() const { return str_[len_ - 1]; }
			void clear() { len_ = 0; str_ = nullptr; }
			T operator[](uv32 index) const { return str_[index]; }

			operator T* () const { return str_; }

			template<typename V> requires IsInt_v<V> || IsFloat_v<V> || IsBool_v<V>
			operator V() const
			{
				V val;
				auto end = cstr::to_val(str_, val);
				if (end == nullptr) CL_Throw(ExceptionCode::Convert_Error, 0, "convert type error");
				return val;
			}

			template<CharType T2>
				requires (sizeof(T) == sizeof(T2))
			bool operator==(const T2* t) const { return cstr::equ(str_, t); }

			template<CharType T2>
				requires (sizeof(T) == sizeof(T2))
			bool operator!=(const T2* t) const { return !cstr::equ(str_, t); }

			bool operator==(const std::nullptr_t&) const { return len_ == 0; }
			bool operator!=(const std::nullptr_t&) const { return len_ != 0; }
			/*#######################################################################################*/
		private:
			sv32 _find_ret(const T* p) const { if (!p) return -1; return sv32(p - str_); }
		public:
			//若查找失败返回-1
			template<CharType C> sv32 find(C c) const { return _find_ret(cstr::find(str_, c)); }
			template<CharType C> sv32 findr(C c) const { return _find_ret(cstr::findr(str_, c)); }
			template<CharType C> sv32 find(uv32 index, C c) const { return _find_ret(cstr::find(str_ + index, c)); }
			template<CharType C> sv32 find(const C* sub) const { return _find_ret(cstr::find(str_, sub)); }
			template<CharType C> sv32 find(uv32 index, const C* sub) const { return _find_ret(cstr::find(str_ + index, sub)); }

			/*#######################################################################################*/
			//std::string和std::wstring的支持
			/*#######################################################################################*/
			_StrView(const std::string& str)
			{
				static_assert(sizeof(T) == 1, "can't convert multibyte to wchar!");
				str_ = (T*)str.data();
				len_ = str.length();
			}
			_StrView& operator=(const std::string& std_str)
			{
				static_assert(sizeof(T) == 1, "can't convert multibyte to wchar!");
				new(this)_StrView(std_str.data(), (uv32)std_str.length()); return *this;
			}

			_StrView(const std::wstring& str)
			{
				static_assert(sizeof(T) == 2, "can't convert wchar to multibyte!");
				str_ = (T*)str.data();
				len_ = str.length();
			}
			_StrView& operator=(const std::wstring& std_str)
			{
				static_assert(sizeof(T) == 2, "can't convert wchar to multibyte!");
				new(this)_StrView(std_str.data(), (uv32)std_str.length()); return *this;
			}
		};

	}
	using StrView = lib::_StrView<uc8>;
	using StrViewW = lib::_StrView<uc16>;

	//用于格式化时, 指示浮点数保留多少位小数
	template<FloatType F>
	struct Fraction
	{
		uv32 len_;//小数部分的长度
		F val_;
		Fraction(F val, uv32 len = 2) : val_(val), len_(len) {}
	};

	template<FloatType F> Fraction<F> fraction(F val, uv32 len = 2) { return Fraction<F>(val, len); }

	template<CharType T>
	struct EmptyStr { T* operator()() const { return (T*)""; } };
	template<>
	struct EmptyStr<uc16> { uc16* operator()() const { return (uc16*)u""; } };
	template<>
	struct EmptyStr<wchar> { wchar* operator()() const { return (wchar*)L""; } };
	template<>
	struct EmptyStr<uc32> { uc32* operator()() const { return (uc32*)U""; } };

	namespace lib
	{
		//一个指定大小的字符串缓冲区
		template<CharType T, MemAllocType MA, uv32 N>
		class _StrBuf
		{
			uv32 len_ = 0;
			T str_[N]; 
		public: 
			_StrBuf() { str_[0] = 0; }

			void reserve(uv32 len) { } 

			template<uv32 M>
			void move(_StrBuf<T, MA, M>&& buf)
			{ 
				len_ = buf.len_;
				cstr::copy(str_, buf.str_, len_);
			} 
			template<typename T2>
			void append(const T2* str, uv32 len)
			{
				cstr::copy(str_ + len_, str, len);
				len_ += len;
				str_[len_] = 0;
			}

			uv32 size() const { return N; } 
			uv32 length() const { return len_; }
			void length(uv32 len) { len_ = len; str_[len] = 0; }
			T* data() const { return (T*)str_; }  
			
			 
			void pop(uv32 cnt) { len_ -= cnt;  str_[len_] = 0; }
		};

		//一个不是固定大小的字符串缓冲区
		template<CharType T, MemAllocType MA>
		class _StrBuf<T, MA, 0>
		{ 
			using MBT = MemBuf<T, MA, 1>;
			uv32 len_ = 0;
			MBT buf_; 
		public: 
			_StrBuf() 
			{
			}

			void reserve(uv32 len) 
			{
				len += len_;
				if (len <= buf_.size()) return;

				MBT tmp;
				tmp.alloc(len); 
				tmp.copy(0, buf_.data(), 0, len_);
				buf_ = std::move(tmp); 
			}
			void move(_StrBuf&& buf) 
			{
				buf_ = std::move(buf.buf_); 
				len_ = buf.len_;
			}

			template<typename T2>
			void append(const T2* str, uv32 len)
			{ 
				cstr::copy(buf_.data() + len_, str, len);
				len_ += len;
				buf_.data()[len_] = 0;
			}

			uv32 size() const { return buf_.size(); }
			uv32 length() const { return len_; }
			void length(uv32 len) { len_ = len; if(buf_.size()) buf_.data()[len] = 0; }
			T* data() const { return buf_.size() == 0 ? EmptyStr<T>()() : buf_.data(); }
			   
			void pop(uv32 cnt) { len_ -= cnt;  buf_.data()[len_] = 0; }
		};
		  
		template<CharType T, MemAllocType MA, uv32 N>
		class _NString : protected _StrBuf<T, MA, N>
		{
			using Base = _StrBuf<T, MA, N>;
			using ThisType = _NString<T, MA, N>;
			  
			uv32 _size() const { return Base::size(); }    
			  
		public:
			_NString() {} 
			_NString(const std::nullptr_t&) {}
			  
			uv32 length() const { return Base::length(); }
			void length(uv32 len) { Base::length(len); }
			void clear() { length(0); }

			T* data() const { return Base::data(); }
			void reserve(uv32 len) { Base::reserve(len); } 

			T& operator[](uv32 index) const { return data()[index]; }

			_StrView<T> view() const { return _StrView<T>(data(), length()); }
			operator _StrView<T>() const { return view(); }

			operator T* () const { return data(); }

			T& back() const { return data()[length() - 1]; }
			void pop(uv32 len) { Base::pop(len); }
			T pop() { T c = back(); pop(1); return c; }
			 
			template<typename T2>
			_NString& append(const T2* str, uv32 len)
			{
				if (str == nullptr || len == 0) return *this;
				reserve(len);
				Base::append(str, len);
				return *this;
			}
			template<typename T2>
			_NString& append(const T2* str)
			{  
				return append(str, cstr::length(str));
			}
			/*#################################################################################*/
			//构造函数和赋值
			/*#################################################################################*/
			template<CharType T2> _NString(const T2* str) { append(str, cstr::length(str)); }
			template<CharType T2> _NString(const _StrView<T2>& str) { append(str.data(), str.length()); }
			_NString(const ThisType& str) { append(str.data(), str.length()); } //无法取消,否则会提示调用已删除的函数   
			template<CharType T2, MemAllocType MA2, uv32 M>
			_NString(const _NString<T2, MA2, M>& str) { append(str.data(), str.length()); }
			_NString(ThisType&& str) noexcept { Base::move(std::move(str)); }

			template<CharType T2> _NString& operator=(const T2* str) { clear(); return append(str, cstr::length(str)); }
			template<CharType T2> _NString& operator=(const _StrView<T2>& str) { clear(); return append(str.data(), str.length()); }
			_NString& operator=(const ThisType& str) { clear(); return append(str.data(), str.length()); }
			template<CharType T2, MemAllocType MA2, uv32 M>
			_NString& operator=(const _NString<T2, MA2, M>& str) { clear(); return append(str.data(), str.length()); }
			_NString& operator=(ThisType&& str) noexcept { Base::move(std::move(str)); return *this; }

			/*#################################################################################*/
			//类型转换
			/*#################################################################################*/
			template<typename V> requires IsBool_v<V> || IsInt_v<V> || IsFloat_v<V>
			operator V() const
			{
				V val;
				auto end = cstr::to_val(data(), val);
				CL_Assert(end != nullptr && *end == 0);
				return val;
			}
			template<typename V> requires IsBool_v<V> || IsInt_v<V> || IsFloat_v<V>
			_NString & operator=(const V & val) { clear(); return operator<<(val); }
			/*#################################################################################*/
			//追加 << 
			/*#################################################################################*/ 
			_NString& operator<<(const std::nullptr_t&) { return append("null", 4); }

			template<typename V> requires IsBool_v<V> || IsInt_v<V> || IsFloat_v<V>
			_NString & operator<<(const V & val)
			{
				T buf[64];
				uv32 len = cstr::from_val(buf, 64, val);
				return append(buf, len);
			}

			template<FloatType F>
			_NString& operator<<(const Fraction<F>& f) 
			{
				T buf[64];
				uv32 len = cstr::from_val(buf, 64, f.val_, f.len_);
				return append(buf, len); 
			}

			template<CharType T2>
			_NString& operator<<(T2 c) 
			{
				reserve(1);
				data()[length()] = c;
				length(length() + 1);
				data()[length()] = 0; 
				return *this;
			}
			template<CharType T2>
			_NString& operator<<(const T2* str) { return append(str, cstr::length(str)); }

			template<CharType T2>
			_NString& operator<<(const _StrView<T2>& str) { return append(str.data(), str.length()); }

			template<CharType T2, MemAllocType MA2, uv32 M>
			_NString& operator<<(const _NString<T2, MA2, M>& str) { return append(str.data(), str.length()); }
			/*#######################################################################################*/
			//比较
			/*#######################################################################################*/
		private:
			template<CharType C> bool _equ(const C* str) const
			{
				if (str == nullptr) str = EmptyStr<C>()();
				return cstr::equ(str, data());
			}
		public:
			bool operator==(const std::nullptr_t&) const { return length() == 0; }
			bool operator!=(const std::nullptr_t&) const { return length() != 0; }

			template<CharType T2> bool operator==(const T2* str) const { return _equ(str); }
			template<CharType T2> bool operator==(const _StrView<T2>& str) const { return _equ(str.data()); }
			template<CharType T2, MemAllocType MA2, uv32 M>
			bool operator==(const _NString<T2, MA2, M>& str) const { return _equ(str.data()); }
			template<typename U> bool operator!=(const U& v) const { return !operator==(v); }

			/*#######################################################################################*/
			//其他一些常规函数
			/*#######################################################################################*/
			void trim() { length(cstr::trim(data(), length())); }
			template<CharType C> void replace(C src, C dst) { cstr::replace(data(), src, dst); }

			template<CharType C> uv32 copy_to(C* buf, uv32 size, uv32 start, uv32 len) const
			{
				uv32 tail = length() - start;
				if (len > tail) len = tail;
				cstr::copy(buf, size, data() + start, len);
				return len;
			}

			template<CharType C> uv32 copy_to(C* buf, uv32 size, uv32 start) const { return copy_to(buf, size, start, length()); }
		private:
			sv32 _find_ret(T* p) { if (p == nullptr) return -1; return sv32(p - data()); }
		public:
			template<CharType C> sv32 find(C c) const { return _find_ret(cstr::find(data(), c)); }
			template<CharType C> sv32 findr(C c) const { return _find_ret(cstr::findr(data(), c)); }
			template<CharType C> sv32 find(uv32 index, C c) const { return _find_ret(cstr::find(data() + index, c)); }
			template<CharType C> sv32 find(const C* sub) const { return _find_ret(cstr::find(data(), sub)); }
			template<CharType C> sv32 find(uv32 index, const C* sub) const { return _find_ret(cstr::find(data() + index, sub)); }

			template<CharType C> bool start_with(const C* sub) const { return _find_ret(cstr::find(data(), sub)) == 0; }
		};
	}
	 
	template<uv32 N>
	using StrBuf = lib::_NString<uc8, MemAllocator, N>;
	template<uv32 N>
	using StrBufW = lib::_NString<uc16, MemAllocator, N>;

	using String = lib::_NString<uc8, MemAllocator, 0>;
	using StringW = lib::_NString<uc16, MemAllocator, 0>;
}

#endif//__cl_base_str__ 