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
			_StrView(const _StrView& str) { str_ = (T*)str.data(); len_ = str.length(); }
			template<CharType T2>
				requires (sizeof(T) == sizeof(T2))
			_StrView(const T2* str) { str_ = (T*)str; len_ = cstr::length(str); }
			template<CharType T2>
				requires (sizeof(T) == sizeof(T2))
			_StrView(const T2 * str, uv32 len) { str_ = (T*)str; len_ = len; }

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
				auto end = cstr::to_val(str_, val);
				if(end == nullptr ) CL_Throw(ExceptionCode::Convert_Error, 0, "convert type error");
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
			sv32 _find_ret(const T* p) { if (!p) return -1; return sv32(p - str_); }
		public:
			//若查找失败返回-1
			template<CharType C> sv32 find(C c) const { return _find_ret(cstr::find(str_, c)); }
			template<CharType C> sv32 findr(C c) const { return _find_ret(cstr::findr(str_, c)); } 
			template<CharType C> sv32 find(uv32 index, C c) const { return _find_ret(cstr::find(str_ + index, c)); } 
			template<CharType C> sv32 find(const C* sub) const { return _find_ret(cstr::find(str_, sub));   } 
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


	namespace lib
	{
		//一个指定大小的字符串缓冲区
		template<CharType T, MemAllocType MA, uv32 N>
		class _StrBuf
		{ 
		public:
			uv32 len_ = 0;
			T str_[N];

			_StrBuf() { str_[0] = 0; } 

			void _base_need(uv32 len) { CL_Assert(len + len_ < N); } 

			template<CharType T>
			void _base_move(T*& str, uv32 size, uv32 len)
			{
				CL_Assert(len < N);
				len_ = len;
				cstr::copy(str_, str, len);
			}
			T*& _base_data() { return str_; }
			uv32 _base_size() { return N; }
			uv32& _base_len() { return len_; }
		};

		//一个不是固定大小的字符串缓冲区
		template<CharType T, MemAllocType MA> 
		class _StrBuf<T, MA, 0>
		{ 
		public:
			MemBuf<T, MA> buf_; 
			  
			void _base_need(uv32 len) { buf_.need(len); }

			template<CharType T>
			void _base_move(T*& str, uv32 size, uv32 len)
			{
				buf_.clear();
				buf_.buf_ = str;
				buf_.size_ = size;
				buf_.len_ = len; 
				str = nullptr;
			}

			T*& _base_data() { return buf_.buf_; }
			uv32 _base_size() { return buf_.size_; }
			uv32& _base_len() { return buf_.len_; }
		};

		//用于格式化时, 指示浮点数保留多少位小数
		struct FloatFormat
		{
			uv32 len_;//小数部分的长度
			FloatFormat(uv32 len = 2) : len_(len) {}
		};

	//	template<CharType T>
	//	struct EmptyStr { const T* operator() { return ""; } };
	//	template<>
	//	struct EmptyStr<uc16> { const uc16* operator() { return U""; } };
	//	template<>
	//	struct EmptyStr<wchar_t> { const wchar_t* operator() { return L""; } };


	//	template<CharType T, MemAllocType MA, uv32 N>
	//	class _NString : protected _StrBuf<T, MA, N>
	//	{
	//		using Base =_StrBuf<T, MA, N>;
	//		using ThisType = _NString<T, MA, N>;

	//		uv32 fraction_ = 2;//默认小数位2位

	//		T*& _data() const { return Base::_base_data(); }
	//		uv32 _size() const { return Base::_base_size(); }
	//		uv32& _len() const { return Base::_base_len(); }
	//		void _need(uv32 len) { Base::_base_need(len); }
	//		template<CharType T2>
	//		void _move(T2*& str, uv32 size, uv32 len) { Base::_base_move(str, size, len); }



	//		template<CharType T2>
	//		_append(const T2* str, uv32 len)
	//		{
	//			_need(len);
	//			cstr::copy(_data() + _len(), str, len);
	//			_len() += len;
	//		}
	//	public: 
	//		_NString() {}
	//		_NString(const std::nullptr_t&) {}
	//		 
	//		T* data() const { return _data()(); }
	//		uv32 length() const { return _len(); }
	//		void need(uv32 len) { _need(len); }
	//		void clear() { _len() = 0; if (_data()) _data()[0] = 0; }

	//		T& operator[](uv32 index) const { return _data()[index]; }

	//		_StrView<T> view() const { return _StrView<T>(_data(), _len()); }
	//		operator _StrView<T>() const { return view(); }

	//		operator T* () const { return _data() ? _data() : EmptyStr<T>()(); } 

	//		T& back() { return _data()[_len() - 1]; }
	//		T pop() { _len() -= 1; T c = _data()[_len()]; _data()[_len()] = 0; return c; }
	//		void pop(uv32 len) { _len() -= len; _data()[_len()] = 0; }

	//		/*#################################################################################*/
	//		//构造函数和赋值
	//		/*#################################################################################*/
	//		template<CharType T2> _NString(const T2* str) { _append(str, cstr::length(str)); }
	//		template<CharType T2> _NString(const _StrView<T2>& str) { _append(str.data(), str.length()); }
	//		_NString(const ThisType& str) { _append(str.data(), str.length()); } //无法取消,否则会提示调用已删除的函数  
	//		_NString(ThisType&& str) noexcept { _move(str._data(), str._size(), str._len()); }

	//		template<CharType T2> _NString& operator=(const T2* str) { clear(); new(this)_NString(str); return *this; }
	//		template<CharType T2> _NString& operator=(const _StrView<T2>& str) { clear(); new(this)_NString(str); return *this; }
	//		_NString& operator=(const ThisType& str) { clear(); new(this)_NString(str); return *this; }  
	//		_NString& operator=(ThisType&& str) noexcept { clear(); new(this)_NString(std::move(str)); return *this; }

	//		template<CharType T2, MA, uv32 M> 
	//		_NString(const _NString<T2, MA, M>& str) { _append(str.data(), str.length()); }
	//		template<CharType T2, MA, uv32 M> 
	//		_NString& operator=(const _NString<T2, MA, M>& str) { clear(); new(this)_NString(str); return *this; }

	//		/*#################################################################################*/
	//		//类型转换
	//		/*#################################################################################*/
	//					template<typename V>
	//					requires IsBool_v<V> || IsInt_v<V> || IsFloat_v<V>
	//		operator V() const
	//		{
	//			V val;
	//			auto end = cstr::str_2_val(str_, val);
	//			cl_assert(end != nullptr && *end == 0);
	//			return val;
	//		}
	//		template<typename V> 
	//		requires IsBool_v<V> || IsInt_v<V> || IsFloat_v<V>
	//		_NString& operator=(const V& val) { _len() = 0; return operator<<(val); }
	//		/*#################################################################################*/
	//		//追加 << 
	//		/*#################################################################################*/

	//	






	//	//	/*#######################################################################################*/


	//	//	/*#######################################################################################*/
	//	//	void trim() { len_ = str_trim(str_, len_); }
	//	//	/*#######################################################################################*/
	//	//	//若查找失败返回-1
	//	//	template<CharType C> uv32 find(C c) const { auto p = str_find(str_, c); if (!p) return -1; return uv32(p - str_); }
	//	//	//若查找失败返回-1
	//	//	template<CharType C> uv32 find(uv32 index, C c) const { auto p = str_find(str_ + index, c); if (!p) return -1; return uv32(p - str_); }
	//	//	//若查找失败返回-1
	//	//	template<CharType C> uv32 find(const C* str) const { auto p = str_find(str_, str); if (!p) return -1; return uv32(p - str_); }
	//	//	//若查找失败返回-1
	//	//	template<CharType C> uv32 find(uv32 index, const C* str) const { auto p = str_find(str_ + index, str); if (!p) return -1; return uv32(p - str_); }
	//	//	//若查找失败返回-1
	//	//	template<CharType C> uv32 findr(C c) const { auto p = str_findr(str_, c); if (!p) return -1; return uv32(p - str_); }
	//	//	/*###############################################################################*/
	//	//	//移除指定索引开始的指定长度字符串
	//	//	void remove(uv32 index, uv32 len)
	//	//	{
	//	//		cl_assert(index + len <= len_);

	//	//		uv32 tail = len_ - index - len;
	//	//		str_copy(str_ + index, str_ + index + len, tail);
	//	//		len_ -= len;
	//	//		str_[len_] = 0;
	//	//	}
	//	//	template<CharType C> void remove(const C* str, uv32 len)
	//	//	{
	//	//		auto p = str_find(str_, str);
	//	//		if (p) remove(uv32(p - str_), len);
	//	//	}
	//	//	template<CharType C> void remove(const C* str) { remove(str, str_len(str)); }
	//	//	template<CharType C> void remove(const _StrView<C>& str) { remove(str.data(), str.len()); }
	//	//	/*###############################################################################*/
	//	//	template<CharType C> void replace(C src, C dst) { str_replace(str_, src, dst); }
	//	//	/*###############################################################################*/
	//	//	template<CharType C> uv32 sub(C* buf, uv32 size, uv32 start, uv32 len) const
	//	//	{
	//	//		cl_assert(len_ >= start);
	//	//		uv32 tail = len_ - start;
	//	//		if (len > tail) len = tail;
	//	//		str_copy(buf, size, str_ + start, len);
	//	//		return len;
	//	//	}

	//	//	template<CharType C> uv32 sub(C* buf, uv32 size, uv32 start) const { return sub(buf, size, start, len_); }
	//	//	/*###############################################################################*/
	//	//public:
	//	//	_NString& operator=(const Fraction& f) { fraction_ = f.len_; return *this; }
	//	//	_NString& operator<<(const Fraction& f) { fraction_ = f.len_; return *this; }

	//	//	_NString& operator<<(const std::nullptr_t&) { return push("null", 4); }

	//	//	_NString& operator<<(const wchar& c) { return push(c); }
	//	//	_NString& operator<<(const char& c) { return push(c); }

	//	//	template<BaseValExFloatType E>
	//	//	_NString& operator<<(const E& val)
	//	//	{
	//	//		T buf[64];
	//	//		uv32 len = str_format(buf, 64, val);
	//	//		return push((const T*)buf, len);
	//	//	}
	//	//	template<FloatType E>
	//	//	_NString& operator<<(const E& val)
	//	//	{
	//	//		T buf[64];
	//	//		uv32 len = str_format(buf, 64, val, fraction_);
	//	//		return push((const T*)buf, len);
	//	//	}
	//	//	template<CharType C> _NString& operator<<(const C* str) { return push(str, str_len(str)); }
	//	//	template<StrViewType T> _NString& operator<<(const T& str) { return push(str.data(), str.len()); }

	//	//	/*###############################################################################*/
	//	//private:
	//	//	template<CharType C> bool _equ(const C* t) const
	//	//	{
	//	//		auto src = str_ ? str_ : detail::_str_empty(str_);
	//	//		return str_equ(src, t);
	//	//	}
	//	//public:
	//	//	bool operator==(const std::nullptr_t&) const { return len_ == 0; }
	//	//	bool operator!=(const std::nullptr_t&) const { return len_ != 0; }

	//	//	//不能写 const C* t
	//	//	template<CharType C> bool operator==(C* t) const { return _equ(t); }

	//	//	template<CharType C> bool operator==(const _StrView<C>& t) const { return _equ(t.data()); }
	//	//	template<CharType C, uv32 M> bool operator==(const _NString<C, A, M>& t) const { return _equ(t.data()); }

	//	//	template<typename T> bool operator!=(const T& t) const { return !operator==(t); }
	//	};
	}



	//template<CharType T, MemAllocType MA = MemAllocator>
	//class _NString 
	//{
	//	using Base = lib::_StrView<T>;
	//	using Base::str_;
	//	using Base::len_;

	//	uv32 size_ = 0;
	//	template<typename R> _NString(const R&) {} //禁止一切带参数初始化
	//public:
	//	_NString() {}

	//	bool need(uv32 size)
	//	{
	//		if (str_ == nullptr)
	//		{
	//			size_ = align<uv32>(size + 1, 16);
	//			str_ = (T*)MA::alloc(size_ * sizeof(T)); 
	//			return str_ != nullptr;
	//		}
	//		else
	//		{
	//			if (len_ + size + 1 >= size_)
	//			{
	//				size_ = align<uv32>(len_ + size + 1, 16);
	//				auto p = (T*)MA::alloc(size_ * sizeof(T));
	//				if (p == nullptr) return false;
	//				if(len_) scopy(p, str_); 
	//				MA::free(str_);
	//				str_ = p; 
	//			}
	//			return true;
	//		}
	//	}

	//	template<CharType R>
	//	bool set(const R* str, uv32 len = 0) 
	//	{
	//		if(len == 0) len = slen(str);
	//		if (!need(len - len_)) return false;
	//		scopy(str_, str);
	//		len_ = len;
	//		return true;
	//	}

	//	bool set(_NString&& str) noexcept
	//	{
	//		str_ = str.str_;
	//		len_ = str.len_;
	//		size_ = str.size_;

	//		str.str_ = nullptr;
	//		str.size_ = str.len_ = 0;
	//		return true;
	//	}

	//};

	//template<MemAllocType MA = DefaultMemAlloc>
	//using _String = _NString<ch8, MA>;

	//using String = _String<>;
}

#endif//__cl_base_str__ 