#ifndef __clan_base_str__
#define __clan_base_str__

#include <new>
#include <string.h>
#include "5exception.h"
#include "7mem.h"

namespace clan
{
	enum class CharProperty
	{
		None = 0, //不可见的字符
		Num = 0x1, //数字
		Letter = 0x2, //Bin : 10 字母
		Letter_Low = 0x6, //Bin : 110 小写字母
		Letter_Up = 0xA, //Dec : 10, Bin : 1010 大写字母
		Symbol = 0x10, //Dec 16, Bin : 1 0000 符号
	};

	CharProperty char_property(u8 c);

	template<CharType T>
	inline bool is_num(T c) { return EnumType<CharProperty>(char_property((u8)c)) == CharProperty::Num; }

	template<CharType T>
	inline bool is_letter(T c) { return EnumType<CharProperty>(char_property((u8)c)) & CharProperty::Letter; }

	u8 char_lower(u8 c);
	u8 char_upper(u8 c);

	namespace detail
	{
		//返回字符串长度, 调用默认的strcpy是会报错的(从这点来说, 这个手动实现的函数也不安全)
		template<CharType T>
		inline s32 _strcpy(T* t1, const T* t2)
		{
			auto org = t1;
			while (*t2) { *t1++ = *t2++; }
			*t1 = 0;
			return s32(t1 - org);
		}

		template<CharType T>
		inline s32 _strcpy_s(T* t1, s32 size, const T* t2)
		{
			auto org = t1;
			while (--size && *t2) { *t1++ = *t2++; }
			*t1 = 0;
			return s32(t1 - org);
		}

		template<CharType T>
		inline s32 _strcpy_ns(T* t1, s32 size, const T* t2, s32 len)
		{
			auto org = t1;
			while (--size && len-- && *t2) { *t1++ = *t2++; }
			*t1 = 0;
			return s32(t1 - org);
		}
		template<CharType T>
		inline void _flip(T* str, s32 len)
		{
			T* end = str + len - 1;
			while (str < end) swap(*str++, *end--);
		}
		/**************************************************************************************************************/
		//返回字符串长度 
		template<CharType T, IntType V>
		inline s32 _uval2str(T* buf, s32 size, V val)
		{
			[[unlikely]]
			if (size <= 1) { *buf = 0; return 0; }

			auto p = buf;
			do
			{
				*p++ = val % 10 + '0';
				val /= 10;
			} while (val != 0 && --size > 1);

			[[unlikely]]
			if (size <= 1) { *buf = 0; return 0; }//缓冲区不足 

			auto len = s32(p - buf);
			_flip(buf, len);
			*p = 0;
			return len;
		}

		//返回字符串长度, fraction表示小数部分 
		s32 _ufval2str(char* buf, s32 size, f32 fval, s32 dst_fraction);
		s32 _ufval2str(char* buf, s32 size, f64 fval, s32 dst_fraction);
		s32 _ufval2str(wchar* buf, s32 size, f32 fval, s32 dst_fraction);
		s32 _ufval2str(wchar* buf, s32 size, f64 fval, s32 dst_fraction);

		template<CharType T, UintType V>
		inline s32 _val2str(T* buf, s32 size, V val)
		{
			return _uval2str(buf, size, val);
		}

		template<CharType T, SintType V>
		inline s32 _val2str(T* buf, s32 size, V val)
		{
			s32 len = 0;
			if (val < 0)
			{
				val = -val;
				*buf++ = '-';
				size--;
				len++;
			}
			return _uval2str(buf, size, val) + len;
		}

		template<CharType T, FloatType V>
		inline s32 _val2str(T* buf, s32 size, V val, s32 fraction)
		{
			s32 len = 0;
			if (val < 0)
			{
				val = -val;
				*buf++ = '-';
				size--;
				len++;
			}
			return _ufval2str(buf, size, val, fraction) + len;
		}
		/**************************************************************************************************************/
		//失败返回nullptr, 否则返回第一个无法解析的字符位置
		char* _str2val(const char* str, s64& val);
		char* _str2val(const char* str, f64& val);
		char* _str2val(const char* str, bool& val);

		wchar* _str2val(const wchar* str, s64& val);
		wchar* _str2val(const wchar* str, f64& val);
		wchar* _str2val(const wchar* str, bool& val);
	}
	enum class CharCode
	{
		UTF8,
		GBK
	};
	class Str
	{
	public:
		//这样写非常慢!!!!
		//for(s32 i = 0; i < len; i++) str[i] = g_char_lower[str[i]];
		template<CharType T> static inline void lower(T* str) { while (*str) { *str = char_lower((u8)*str); ++str; } }
		template<CharType T> static inline void upper(T* str) { while (*str) { *str = char_upper((u8)*str); ++str; } }
		/**************************************************************************************************************/
		static inline s32 len(const char* str) { return (s32)::strlen(str); }
		static inline s32 cmp(const char* t1, const char* t2) { return ::strcmp(t1, t2); }
		static inline s32 cmp(const char* t1, const char* t2, s32 len) { return ::strncmp(t1, t2, len); }
		static inline bool equ(const char* t1, const char* t2) { return strcmp(t1, t2) == 0; }
		static inline bool equ(const char* t1, const char* t2, s32 len) { return ::strncmp(t1, t2, len) == 0; }
		static inline bool iequ(const char* t1, const char* t2) { return ::_stricmp(t1, t2) == 0; }//忽略大小写进行比较 
		static inline bool iequ(const char* t1, const char* t2, s32 len) { return ::_strnicmp(t1, t2, len) == 0; }
		/**************************************************************************************************************/
		static inline s32 len(const wchar* str) { return (s32)::wcslen(str); }
		static inline s32 cmp(const wchar* t1, const wchar* t2) { return ::wcscmp(t1, t2); }
		static inline s32 cmp(const wchar* t1, const wchar* t2, s32 len) { return ::wcsncmp(t1, t2, len); }
		static inline bool equ(const wchar* t1, const wchar* t2) { return wcscmp(t1, t2) == 0; }
		static inline bool equ(const wchar* t1, const wchar* t2, s32 len) { return ::wcsncmp(t1, t2, len) == 0; }
		static inline bool iequ(const wchar* t1, const wchar* t2) { return ::_wcsicmp(t1, t2) == 0; }
		static inline bool iequ(const wchar* t1, const wchar* t2, s32 len) { return ::_wcsnicmp(t1, t2, len) == 0; }
		/**************************************************************************************************************/
		static inline s32 copy(char* t1, const char* t2) { return detail::_strcpy(t1, t2); }
		static inline s32 copy(char* t1, s32 size, const char* t2) { return detail::_strcpy_s(t1, size, t2); }
		static inline s32 copy(char* t1, s32 size, const char* t2, s32 len) { return detail::_strcpy_ns(t1, size, t2, len); }

		static inline s32 copy(wchar* t1, const wchar* t2) { return detail::_strcpy(t1, t2); }
		static inline s32 copy(wchar* t1, s32 size, const wchar* t2) { return detail::_strcpy_s(t1, size, t2); }
		static inline s32 copy(wchar* t1, s32 size, const wchar* t2, s32 len) { return detail::_strcpy_ns(t1, size, t2, len); }
		/**************************************************************************************************************/
		static inline char* find(const char* str, char c) { return (char*)::strchr(str, c); }
		static inline char* rfind(const char* str, char c) { return (char*)::strrchr(str, c); }
		static inline char* find(const char* str, const char* dst) { return (char*)::strstr(str, dst); }

		static inline wchar* find(const wchar* str, wchar c) { return (wchar*)::wcschr(str, c); }
		static inline wchar* rfind(const wchar* str, wchar c) { return (wchar*)::wcsrchr(str, c); }
		static inline wchar* find(const wchar* str, const wchar* dst) { return (wchar*)::wcsstr(str, dst); }
		/**************************************************************************************************************/
		//反转字符串
		template<CharType T> static inline void flip(T* str, s32 len)
		{
			detail::_flip(str, len);
		}
		/**************************************************************************************************************/
		//去掉字符串末端无法显示的字符,返回裁剪后的长度
		template<CharType T> static inline s32 trim_back(T* str, s32 len)
		{
			auto end = str + len - 1;
			while (*end > 0 && *end <= 32 && end >= str) --end;
			end[1] = 0;
			return s32(end - str) + 1;
		}
		//去掉字符串前端和末端无法显示的字符,返回裁剪后的长度
		template<CharType T> static inline s32 trim(T* str, s32 len)
		{
			len = trim_back(str, len);
			if (len)
			{
				T* begin = str;
				while (len-- && *begin <= 32) ++begin;
				len = s32(begin - str);
				if (len <= 0)
				{
					str[0] = 0;
					return 0;
				}
				for (s32 i = 0; i < len; i++)
					str[i] = begin[i];
				str[len] = 0;
			}
			return len;
		}
		/**************************************************************************************************************/
		//替换所有指定字符为目标字符
		template<CharType T> static inline void replace(T* str, T src, T dst)
		{
			while (*str)
			{
				if (*str == src) *src = dst;
				++str;
			}
		}
		/**************************************************************************************************************/
		template<CharType T>
		static inline s32 convert(T* buf, s32 size, bool val)
		{
			if (val)
			{
				if (size < 4) { *buf = 0; return 0; }
				buf[0] = 't';
				buf[1] = 'r';
				buf[2] = 'u';
				buf[3] = 'e';
				buf[4] = 0;
				return 4;
			}
			if (size < 5) { *buf = 0; return 0; }
			buf[0] = 'f';
			buf[1] = 'a';
			buf[2] = 'l';
			buf[3] = 's';
			buf[4] = 'e';
			buf[5] = 0;
			return 5;
		}
		template<CharType T> static inline s32 convert(T* buf, s32 size, T val)
		{
			buf[0] = val;
			buf[1] = 0;
			return 1;
		}
		template<CharType T, IntType V> static inline s32 convert(T* buf, s32 size, V val)
		{
			return detail::_val2str(buf, size, val);
		}
		template<CharType T, FloatType V> static inline s32 convert(T* buf, s32 size, V val, s32 fraction = 0)
		{
			return detail::_val2str(buf, size, val, fraction);
		}
		/**************************************************************************************************************/
		template<CharType T, BoolType V> static inline T* to(const T* buf, V& val)
		{
			return detail::_str2val(buf, val);
		}

		template<CharType T, FloatType V> static inline T* to(const T* buf, V& val)
		{
			f64 tmp;
			auto ret = detail::_str2val(buf, tmp);
			val = (V)tmp;
			return ret;
		}

		template<CharType T, IntType V> static inline T* to(const T* buf, V& val)
		{
			s64 tmp;
			auto ret = detail::_str2val(buf, tmp);
			val = (V)tmp;
			return ret;
		}
		template<CharType T> static inline T* to(const T* buf, T& val)
		{
			s64 tmp;
			auto ret = detail::_str2val(buf, tmp);
			val = (T)tmp;
			return ret;
		}
		/**************************************************************************************************************/
		static inline CharCode judge_char_code(const char* str);
	};

	template<CharType T>
	class StrView
	{
	public:
		const T* str_ = nullptr;
		s32 len_ = 0;

		StrView() { clan_CheckClass(StrView); }
		StrView(const std::nullptr_t&) {}
		StrView(const T* str) { str_ = str; len_ = Str::len(str); }
		StrView(const StrView& str) { str_ = str.str_; len_ = str.len_; }
		StrView(const T* str, s32 len) { str_ = str; len_ = len; }
		/*#######################################################################################*/
		StrView& operator=(const T* str) { new(this)StrView(str); return *this; }
		StrView& operator=(const StrView& str) { str_ = str.str_; len_ = str.len_; return *this; }
		/*#######################################################################################*/

		s32 len() const { return len_; }
		const T* data() const { return str_; }

		operator const T* () const { return str_; }
		char operator[](s32 index) const { return str_[index]; }

		template<ValType R>
		operator R() const
		{
			R val;
			Str::to(str_, val);
			return val;
		}

		bool operator==(const T* t) const { return Str::equ(str_, t) == 0; }
		bool operator!=(const T* t) const { return !this->operator==(t); }
		bool operator==(const std::nullptr_t&) const { return len_ == 0; }
		bool operator!=(const std::nullptr_t&) const { return len_ != 0; }
		/*#######################################################################################*/
		//若查找失败返回-1
		s32 find(T c) const { auto p = Str::find(str_, c); if (!p) return -1; return s32(p - str_); }
		//若查找失败返回-1
		s32 find(s32 index, T c) const { auto p = Str::find(str_ + index, c); if (!p) return -1; return s32(p - str_); }
		//若查找失败返回-1
		s32 find(const T* str) const { auto p = Str::find(str_, str); if (!p) return -1; return s32(p - str_); }
		//若查找失败返回-1
		s32 find(s32 index, const T* str) const { auto p = Str::find(str_ + index, str); if (!p) return -1; return s32(p - str_); }
		//若查找失败返回-1
		s32 rfind(T c) const { auto p = Str::rfind(str_, c); if (!p) return -1; return s32(p - str_); }
	};

	namespace detail
	{
		template<CharType T, AllocMemType A, s32 N> class _StrBase
		{
		public:
			T str_[N];
			s32 len_ = 0;

			_StrBase() { str_[0] = 0; }
			~_StrBase() {}
		protected:
			void _need(s32 len) { clan_assert(len + len_ < N); }
			void _move(_StrBase* str)
			{
				len_ = str->len_;
				Str::copy(str_, str->str_);
			}
			s32 _size() const { return N; }
		};

		template<CharType T, AllocMemType A> class _StrBase<T, A, 0>
		{
			s32 size_ = 0; //可以存储的字符数目, 非内存大小
		public:
			T* str_ = nullptr;
			s32 len_ = 0;

			_StrBase() {}
			~_StrBase() { if (str_) A().free(str_); }
		protected:
			void _need(s32 len)
			{
				if (str_ == nullptr)
				{
					size_ = align(len + 1, 16);
					str_ = (T*)A().alloc(size_ * sizeof(T));
					str_[0] = 0;
				}
				else
				{
					if (len_ + len + 1 >= size_)
					{
						size_ = align(len_ + len + 1, 16);
						str_ = (T*)A().realloc(str_, size_ * sizeof(T));
					}
				}
			}
			void _move(_StrBase* str)
			{
				if (str_) A().free(str_);

				len_ = str->len_; str->len_ = 0;
				size_ = str->size_; str->size_ = 0;
				str_ = str->str_; str->str_ = nullptr;
			}
			s32 _size() const { return size_; }
		};

		char* _empty(char*);
		wchar* _empty(wchar*);
	}

	struct Fraction
	{
		s32 len_;//小数部分的长度
		Fraction(s32 len = 2) : len_(len) {}
	};
	 
	template<CharType T, AllocMemType A, s32 N> class _String : public detail::_StrBase<T, A, N>
	{
		using Base = detail::_StrBase<T, A, N>;
		using SelfType = _String<T, A, N>;
		using Base::_need;
		using Base::_move;
		using Base::_size;//可以存储的字符数目, 非内存大小
		  
		void _set(const T* str, s32 len)
		{
			_need(len);
			len_ = len;
			Str::copy(str_, str);
		}
	public:
		s32 fraction_ = 2;//默认小数位2位
		using Base::str_;
		using Base::len_;

		_String() {}
		_String(const std::nullptr_t&) {}
		_String(const T* str) { _set(str, Str::len(str)); }
		_String(const StrView<T>& str) { if (str.str_) _set(str.data(), str.len()); }
		template<s32 M>
		_String(const _String<T, A, M>& str) { _set(str.str_, str.len_); } 
		_String(const SelfType& str) { _set(str.str_, str.len_); } //无法取消,否则会提示调用已删除的函数  
		_String(SelfType&& str) noexcept { _move(&str); }
		 
		_String& operator=(const T* str) { if (str == nullptr) _set("", 0); else _set(str, Str::len(str)); return *this; }
		_String& operator=(const StrView<T>& str) { _set(str.data(), str.len()); return *this; }
		template<s32 M>
		_String& operator=(const _String<T, A, M>& str) noexcept { _set(str.str_, str.len_); return *this; }
		_String& operator=(SelfType&& str) noexcept { _move(&str); return *this; }
		template<ValType R> _String& operator=(const R& val) { len_ = 0; return operator<<(val); }

		T* data() const { return (T*)str_; }
		s32 len() const { return len_; }
		s32 size() const { return _size(); }
		void need(s32 len) { _need(len); }

		StrView<T> view() const { return StrView<T>(str_, len_); }
		T& operator[](s32 index) const { return str_[index]; }

		operator StrView<T>() const { return view(); }
		operator T* () const { return str_ ? str_ : detail::_empty(str_); } 
		template<ValType R>
		operator R() const { R val = R(); if(len_ != 0) Str::to(str_, val); return val; }
		/*#######################################################################################*/
		T& back() { return str_[len_ - 1]; }
		T pop_back() { T c = str_[--len_]; str_[len_] = 0; return c; }
		_String& push_back(T c) { str_[len_++] = c; return *this; }

		_String& push_back(const T* str, s32 len)
		{
			_need(len);
			Byte::copy(str_ + len_, str, len);
			len_ += len;
			str_[len_] = 0;
			return *this;
		}
		/*#######################################################################################*/
		void trim() { len_ = Str::trim(str_, len_); }
		/*#######################################################################################*/
		//若查找失败返回-1
		s32 find(T c) const { auto p = Str::find(str_, c); if (!p) return -1; return s32(p - str_); }
		//若查找失败返回-1
		s32 find(s32 index, T c) const { auto p = Str::find(str_ + index, c); if (!p) return -1; return s32(p - str_); }
		//若查找失败返回-1
		s32 find(const T* str) const { auto p = Str::find(str_, str); if (!p) return -1; return s32(p - str_); }
		//若查找失败返回-1
		s32 find(s32 index, const T* str) const { auto p = Str::find(str_ + index, str); if (!p) return -1; return s32(p - str_); }
		//若查找失败返回-1
		s32 rfind(T c) const { auto p = Str::rfind(str_, c); if (!p) return -1; return s32(p - str_); }
		/*###############################################################################*/
		//移除指定索引开始的指定长度字符串
		void remove(s32 index, s32 len)
		{
			clan_assert(index + len <= len_);

			s32 tail = len_ - index - len;
			Byte::copy(str_ + index, str_ + index + len, tail);
			len_ -= len;
			str_[len_] = 0;
		}
		void remove(const T* str, s32 len)
		{
			auto p = Str::find(str_, str);
			if (p) remove(s32(p - str_), len);
		}
		void remove(const T* str) { remove(str, Str::len(str)); }
		void remove(const StrView<T>& str) { remove(str.data(), str.len()); }
		/*###############################################################################*/
		void replace(T src, T dst) { Str::replace(str_, src, dst); }
		/*###############################################################################*/
		s32 sub(T* buf, s32 start, s32 len) const
		{
			clan_assert(len_ >= start);
			s32 tail = len_ - start;
			if (len > tail) len = tail;
			Str::copy(buf, 9999, str_ + start, len);
			return len;
		}

		s32 sub(T* buf, s32 start) const { return sub(buf, start, len_); }
		/*###############################################################################*/
	public:
		_String& operator=(const Fraction& f) { fraction_ = f.len_; return *this; }
		_String& operator<<(const Fraction& f) { fraction_ = f.len_; return *this; }
		_String& operator<<(const std::nullptr_t&) { return push_back("null", 4); }

		template<ValExFloatType R>
		_String& operator<<(const R& val)
		{
			T buf[64];
			s32 len = Str::convert(buf, 64, val);
			return push_back((const T*)buf, len);
		}
		template<FloatType R>
		_String& operator<<(const R& val)
		{
			T buf[64];
			s32 len = Str::convert(buf, 64, val, fraction_);
			return push_back((const T*)buf, len);
		}
		_String& operator<<(const T* str) { return push_back(str, Str::len(str)); }
		_String& operator<<(const StrView<T>& str) { return push_back(str.data(), str.len()); }
		template<s32 M> _String& operator<<(const _String<T, A, M>& str) { return push_back(str.data(), str.len()); }

		/*###############################################################################*/
		bool operator==(const std::nullptr_t&) const { return len_ == 0; }
		bool operator!=(const std::nullptr_t&) const { return len_ != 0; }

		bool operator==(const T* t) const
		{
			auto src = str_ ? str_ : detail::_empty();
			return Str::equ(src, t);
		}
		bool operator==(const StrView<T>& t) const { return operator==(t.data()); }
		template<s32 M>
		bool operator==(const _String<T, A, M>& t) const { return operator==(t.data()); }
		template<typename T> bool operator!=(const T& t) const { return !operator==(t); }
	};
}

#endif//__clan_base_str__ 