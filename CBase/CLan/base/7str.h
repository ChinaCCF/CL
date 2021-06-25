#ifndef __clan_str__
#define __clan_str__
 
#include <new>
#include <string.h>
#include "6mem.h"

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
		inline s32 _fval2str(T* buf, s32 size, V val, s32 fraction)
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
			return detail::_fval2str(buf, size, val, fraction);
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
	class cstr
	{
	public:
		const T* str_ = nullptr;
		s32 len_ = 0;

		cstr() { clan_CheckClass(cstr); }
		cstr(const std::nullptr_t&) {}
		cstr(const T* str) { str_ = str; len_ = Str::len(str); }
		cstr(const cstr& str) { str_ = str.str_; len_ = str.len_; } 
		cstr(const T* str, s32 len) {  str_ = str; len_ = len; }
		/*#######################################################################################*/
		cstr& operator=(const T* str) { new(this)cstr(str); return *this; }
		cstr& operator=(const cstr& str) { str_ = str.str_; len_ = str.len_; return *this; }
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
			bool _need(s32 len) { if (len >= N) return false; return true; }
			void _move(_StrBase* str)
			{
				len_ = str->len_;
				Str::copy(str_, str->str_);
			}
			s32 _size() const { return N; }
		};

		template<CharType T, AllocMemType A> class _StrBase<T, A, 0>
		{ 
			s32 size_;
		public:
			T* str_;
			s32 len_;

			_StrBase()
			{
				str_ = nullptr;
				len_ = 0;
				size_ = 0;
			}
			~_StrBase() { if (str_) A().free(str_); }
		protected:
			void _need(s32 len)
			{
				if (size_ == 0)
				{ 
					auto le = align(len + 1, 16);
					size_ = sizeof(T) * le;
					str_ = (T*)A().alloc(size_);
					str_[0] = 0;
				}
				else
				{
					if (len_ + len + 1 >= size_)
					{
						auto le = align(len_ + len + 1, 16);
						size_ = sizeof(T) * le; 
						str_ = (T*)A().realloc(str_, size_);
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
	}

	template<CharType T, AllocMemType A, s32 N> class _String : public detail::_StrBase<T, A, N>
	{
		using Base = detail::_StrBase<T, A, N>; 
		//void _set(const T* str, s32 len)
		//{
		//	if (len == 0)
		//	{
		//		if (str_) str_[0] = 0;
		//		len_ = 0;
		//		return;
		//	}

		//	_reserve(len);
		//	len_ = len;
		//	strcpy(str_, str);
		//}
	public: 
		_String() {}
	//	_String(const char* str) { if (str == nullptr) _set(0, 0); else _set(str, strlen(str)); }
	//	_String(const cstr& str) { _set(str.data(), str.len()); }
	//	_String(const std::string& str) { _set(str.data(), (s32)str.length()); }
	//	_String(const _String<N>& str) { _set(str.data(), str.len()); } //无法取消,否则会提示调用已删除的函数 

	//	_String(_String<N>&& str) noexcept { _move(&str); }

	//	_String& operator=(const c8* str) { if (str == nullptr) _set(0, 0); else _set(str, strlen(str)); return *this; }
	//	_String& operator=(const cstr& str) { _set(str.data(), str.len()); return *this; }
	//	_String& operator=(const std::string& str) { _set(str.data(), (s32)str.length()); return *this; }
	//	_String& operator=(const _String<N>& str) noexcept { _set(str.data(), str.len()); return *this; } //无法取消,否则会提示调用已删除的函数 

	//	_String& operator=(_String<N>&& str) noexcept { _move(&str); return *this; }


	//	c8* data() const { return (c8*)str_; }
	//	s32 len() const { return len_; }
	//	s32 size() const { return _size(); }

	//	void reserve(s32 len) { _reserve(len); }

	//	void set_len(s32 len) { len_ = len; if (str_) str_[len] = 0; }
	//	void cal_len() { len_ = strlen(str_); }

	//	c8& back() { return str_[len_ - 1]; }
	//	void pop(s32 len = 1) { len_ -= len; str_[len_] = 0; }

	//	void trim() { len_ = cl::trim(str_, len_); }
	//	/*#######################################################################################*/
	//	//若查找失败返回-1
	//	s32 find(c8 c) const { auto p = strfind(str_, c); if (!p) return -1; return s32(p - str_); }
	//	//若查找失败返回-1
	//	s32 find(s32 index, c8 c) const { auto p = strfind(str_ + index, c); if (!p) return -1; return s32(p - str_); }
	//	//若查找失败返回-1
	//	s32 find(const c8* str) const { auto p = strfind(str_, str); if (!p) return -1; return s32(p - str_); }
	//	//若查找失败返回-1
	//	s32 find(s32 index, const c8* str) const { auto p = strfind(str_ + index, str); if (!p) return -1; return s32(p - str_); }
	//	//若查找失败返回-1
	//	s32 rfind(c8 c) const { char* p = str_ + len_ - 1; while (*p != c && p >= str_) --p; return s32(p - str_); }
	//	/*###############################################################################*/
	//	//移除指定索引开始的指定长度字符串
	//	void remove(s32 index, s32 len)
	//	{
	//		cl_assert(index + len <= len_);
	//		s32 tail = len_ - index - len;
	//		Byte::copy(str_ + index, str_ + index + len, tail);
	//		len_ -= len;
	//		str_[len_] = 0;
	//	}
	//	void remove(const char* str, s32 len)
	//	{
	//		auto p = strfind(str_, str);
	//		if (p) remove(s32(p - str_), len);
	//	}
	//	void remove(const char* str) { remove(str, strlen(str)); }
	//	void remove(const cstr& str) { remove(str.data(), str.len()); }
	//	void remove(const std::string& str) { remove(str.data(), (s32)str.length()); }
	//	/*###############################################################################*/
	//	void replace(c8 src, c8 dst) { strrpl(str_, src, dst); }
	//	void replace(const char* src, s32 src_len, const char* dst, s32 dst_len)
	//	{
	//		s32 dif_len = dst_len - src_len;
	//		if (dif_len > 0) _reserve(dif_len);
	//		len_ = strrpl(str_, size(), len_, src, src_len, dst, dst_len);
	//	}
	//	void replace(const char* src, const char* dst) { replace(src, strlen(src), dst, strlen(dst)); }
	//	void replace(const cstr& src, const cstr& dst) { replace(src.data(), src.len(), dst.data(), dst.len()); }
	//	void replace(const std::string& src, const std::string& dst) { replace(src.data(), (s32)src.length(), dst.data(), (s32)dst.length()); }
	//	/*###############################################################################*/
	//	template<s32 M> void sub(s32 start, s32 len, _String<M>* ret) const
	//	{
	//		cl_assert(len_ >= start);
	//		s32 tail = len_ - start;
	//		ret->len_ = 0;
	//		if (tail > 0)
	//		{
	//			len = (len > tail) ? tail : len;
	//			ret->append((c8*)str_ + start, len);
	//		}
	//	}
	//	_String<0> sub(s32 start, s32 len) const
	//	{
	//		_String<0> str;
	//		sub(start, len, &str);
	//		return str;
	//	}
	//	_String<0> sub(s32 start) const { return sub(start, len_); }
	//	/*###############################################################################*/
	//	_String& append(const c8 c)
	//	{
	//		_reserve(1);
	//		str_[len_++] = c;
	//		str_[len_] = 0;
	//		return *this;
	//	}
	//	_String& append(const c8* str, s32 len)
	//	{
	//		_reserve(len);
	//		Byte::copy(str_ + len_, str, len);
	//		len_ += len;
	//		str_[len_] = 0;
	//		return *this;
	//	}
	//	/*###############################################################################*/
	//private:
	//	template<typename T> //可以接受的类型为数值
	//	struct _AddVal
	//	{
	//		enum { value = IsVal<T>::value };
	//		inline void operator()(_String* s, const T& val)
	//		{
	//			char buf[64];
	//			s32 len = val2str(buf, 64, val);
	//			s->append(buf, len);
	//		}
	//	};
	//	template<typename T>
	//	struct _AddStr
	//	{
	//		enum { value = IsStr<T>::value };
	//		inline void operator()(_String* s, const T& val)
	//		{
	//			cstr cs(val);
	//			s->append(cs.data(), cs.len());
	//		}
	//	};
	//public:
	//	template<typename T>
	//	_String& operator<<(const T& val)
	//	{
	//		using Type = typename SelectSupportT<_AddVal<T>, _AddStr<T>>::type;
	//		Type()(this, val);
	//		return *this;
	//	}
	//	_String& operator<<(const Float& val)
	//	{
	//		char buf[64];
	//		s32 len = val2str(buf, 64, val);
	//		append(buf, len);
	//		return *this;
	//	}
	//	_String& operator<<(const std::nullptr_t&) { return append("null"); }
	//	_String& operator<<(char c) { return append(c); }

	//	template<typename T> _String& operator=(T&& val) { len_ = 0; return this->operator<<(std::forward<T>(val)); }
	//	template<typename T> _String& operator+=(T&& val) { return this->operator<<(std::forward<T>(val)); }
	//	/*###############################################################################*/

	//	cstr cs() const { return cstr(str_, len_); }
	//	operator cstr () const { return cstr(str_, len_); }
	//	operator c8* () const { auto ret = str_ == nullptr ? "" : str_; return (c8*)ret; }
	//	c8& operator[](s32 index) { return str_[index]; }
	//private:
	//	template<typename T> T _2val() const { cl_assert(len_ > 0); T val; str2val(str_, val); return val; }
	//public:
	//	operator bool() const { return _2val<bool>(); }
	//	operator s8() const { return _2val<s8>(); }
	//	operator s16() const { return _2val<s16>(); }
	//	operator s32() const { return _2val<s32>(); }
	//	operator s64() const { return _2val<s64>(); }
	//	operator u8() const { return _2val<u8>(); }
	//	operator u16() const { return _2val<u16>(); }
	//	operator u32() const { return _2val<u32>(); }
	//	operator u64() const { return _2val<u64>(); }
	//	operator f32() const { return _2val<f32>(); }
	//	operator f64() const { return _2val<f64>(); }

	//	template<typename T> bool operator==(const T& t) const
	//	{
	//		const char* src = str_;
	//		if (str_ == nullptr || len_ == 0) src = "";

	//		return strcmp(src, (const char*)t) == 0;
	//	}
	//	template<typename T> bool operator!=(const T& t) const { return !this->operator==(t); }
	//	template<> bool operator==(const std::nullptr_t&) const { return len_ == 0; }
	//	template<> bool operator!=(const std::nullptr_t&) const { return len_ != 0; }
	};
}

#endif//__clan_str__ 