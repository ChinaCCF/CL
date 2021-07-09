#ifndef __clan_base_str__
#define __clan_base_str__

#include <new>
#include <string.h>
#include "5exception.h"
#include "7mem.h"

namespace cl
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
		inline s32 _str_len(const char* str) { return (s32)::strlen(str); }
		inline s32 _str_len(const wchar* str) { return (s32)::wcslen(str); }
		  
		/**************************************************************************************************************/
		//为啥要重复造这个轮子呢??? 
		/**************************************************************************************************************/
		//返回字符串长度, 调用默认的strcpy是会报错的(从这点来说, 这个手动实现的函数也不安全)
		template<CharType C1, CharType C2>
		inline s32 _str_copy(C1* dst, const C2* src)
		{
			auto org = dst;
			while (*src) { *dst++ = *src++; }
			*dst = 0;
			return s32(dst - org);
		}

		//返回拷贝的字节数
		template<CharType C1, CharType C2>
		inline s32 _str_copy(C1* dst, s32 size, const C2* src)
		{
			auto org = dst;
			while (--size && *src) { *dst++ = *src++; }
			*dst = 0;
			return s32(dst - org);
		}

		//返回拷贝的字节数
		template<CharType C1, CharType C2>
		inline s32 _str_copy(C1* dst, const C2* src, s32 len)
		{
			auto org = dst;
			while (len-- && *src) { *dst++ = *src++; }
			*dst = 0;
			return s32(dst - org);
		}

		//返回拷贝的字节数
		template<CharType C1, CharType C2>
		inline s32 _str_copy(C1* dst, s32 size, const C2* src, s32 len)
		{
			auto org = dst;
			while (--size && len-- && *src) { *dst++ = *src++; }
			*dst = 0;
			return s32(dst - org);
		}
		/**************************************************************************************************************/
		/**************************************************************************************************************/
		template<CharType C1, CharType C2>
		static inline s32 _str_cmp(const C1* t1, const C2* t2)
		{
			while (*t1 && *t1 == *t2) { ++t1; ++t2; }

			if (*t1 == *t2) return 0;
			if (*t1 < *t2) return -1; 
			return 1;
		}

		template<CharType C1, CharType C2>
		static inline s32 _str_cmp(const C1* t1, const C2* t2, s32 len)
		{
			while (len && *t1 && *t1 == *t2) { ++t1; ++t2; --len; }

			if (len == 0 || *t1 == *t2) return 0;
			if (*t1 < *t2) return -1;
			return 1;
		}

		template<CharType C1, CharType C2>
		static inline s32 _str_icmp(const C1* t1, const C2* t2)
		{
			while (*t1 && char_lower(*t1) == char_lower(*t2)) { ++t1; ++t2; }

			auto c1 = char_lower(*t1);
			auto c2 = char_lower(*t2);

			if (c1 == c2) return 0;
			if (c1 < c2) return -1; 
			return 1;
		}

		template<CharType C1, CharType C2>
		static inline s32 _str_icmp(const C1* t1, const C2* t2, s32 len)
		{
			while (len && *t1 && char_lower(*t1) == char_lower(*t2)) { ++t1; ++t2; --len; }

			auto c1 = char_lower(*t1);
			auto c2 = char_lower(*t2);

			if (len == 0 || c1 == c2) return 0;
			if (c1 < c2) return -1; 
			return 1;
		}

		/**************************************************************************************************************/
		template<CharType C1, CharType C2>
		static inline bool _str_equ(const C1* t1, const C2* t2)
		{
			return _str_cmp(t1, t2) == 0;
		}

		template<CharType C1, CharType C2>
		static inline bool _str_equ(const C1* t1, const C2* t2, s32 len)
		{
			return _str_cmp(t1, t2, len) == 0;
		}
		 
		/**************************************************************************************************************/
		template<CharType C1, typename C2>
		static inline C1* _str_find(const C1* str, C2 c)
		{
			while (*str && *str != c) str++;
			if (*str == 0) return nullptr;
			return (C1*)str;
		}

		template<CharType C1, CharType C2>
		static inline C1* _str_find(const C1* str, const C2* sub, s32 len)
		{
			while (true)
			{
				while (*str && *str != *sub) str++;
				if (*str == 0) return nullptr;
				if (_str_cmp(str, sub, len) == 0) return (C1*)str;
				++str;
			}
		}

		template<CharType C1, CharType C2>
		static inline C1* _str_find(const C1* str, const C2* sub)
		{ 
			return _str_find(str, sub, _str_len(sub));
		}
		 
		/**************************************************************************************************************/
		//反转指定长度字符串
		template<CharType T>
		inline void _str_flip(T* str, s32 len)
		{
			T* end = str + len - 1;
			while (str < end) swap(*str++, *end--);
		}
		/**************************************************************************************************************/
		//格式化无符号整型到字符串缓冲区中
		//返回字符串长度 
		template<CharType T, IntType V>
		inline s32 _str_format_uval(T* buf, s32 size, V val)
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
			_str_flip(buf, len);
			*p = 0;
			return len;
		}

		//返回字符串长度, fraction表示小数部分 
		s32 _str_format_ufval(char* buf, s32 size, f32 fval, s32 dst_fraction);
		//返回字符串长度
		s32 _str_format_ufval(char* buf, s32 size, f64 fval, s32 dst_fraction);
		//返回字符串长度,
		s32 _str_format_ufval(wchar* buf, s32 size, f32 fval, s32 dst_fraction);
		//返回字符串长度,
		s32 _str_format_ufval(wchar* buf, s32 size, f64 fval, s32 dst_fraction);

		//返回字符串长度,
		template<CharType T, UintType V>
		inline s32 _str_format(T* buf, s32 size, V val) { return _str_format_uval(buf, size, val); }

		//返回字符串长度,
		template<CharType T, SintType V>
		inline s32 _str_format(T* buf, s32 size, V val)
		{
			s32 len = 0;
			if (val < 0)
			{
				val = -val;
				*buf++ = '-';
				size--;
				len++;
			}
			return _str_format_uval(buf, size, val) + len;
		}

		//返回字符串长度,
		template<CharType T, FloatType V>
		inline s32 _str_format(T* buf, s32 size, V val, s32 fraction = 0)
		{
			s32 len = 0;
			if (val < 0)
			{
				val = -val;
				*buf++ = '-';
				size--;
				len++;
			}
			return _str_format_ufval(buf, size, val, fraction) + len;
		}
		/*###################################################################################*/
		//str 2 val
		/*###################################################################################*/
		struct Str2Val
		{
			bool is_float_;
			f64 val_;
		};
		//失败返回nullptr, 否则返回第一个无法解析的字符位置
		char* _str2val(const char* str, Str2Val& val);
		//失败返回nullptr, 否则返回第一个无法解析的字符位置
		char* _str2val(const char* str, bool& val);

		//失败返回nullptr, 否则返回第一个无法解析的字符位置
		wchar* _str2val(const wchar* str, Str2Val& val);
		//失败返回nullptr, 否则返回第一个无法解析的字符位置
		wchar* _str2val(const wchar* str, bool& val); 
	}

	class CStr
	{
	public:
		//这样写非常慢!!!!
		//for(s32 i = 0; i < len; i++) str[i] = g_char_lower[str[i]];
		template<CharType T> static inline void lower(T* str) { while (*str) { *str = char_lower((u8)*str); ++str; } }
		template<CharType T> static inline void upper(T* str) { while (*str) { *str = char_upper((u8)*str); ++str; } }
		/**************************************************************************************************************/
		template<CharType T> static inline s32 len(const T* str) { return (s32)detail::_str_len(str); }
		/**************************************************************************************************************/
		template<CharType T> static inline s32 copy(T* dst, const T* src) { return detail::_str_copy(dst, src); }
		template<CharType T> static inline s32 copy(T* dst, const T* src, s32 len) { return detail::_str_copy(dst, src, len); }
		template<CharType T> static inline s32 copy(T* dst, s32 size, const T* src) { return detail::_str_copy(dst, size, src); }
		template<CharType T> static inline s32 copy(T* dst, s32 size, const T* src, s32 len) { return detail::_str_copy(dst, size, src, len); } 
		/**************************************************************************************************************/
		template<CharType T> static inline s32 cmp(const T* t1, const T* t2) { return detail::_str_cmp(t1, t2); }
		template<CharType T> static inline s32 cmp(const T* t1, const T* t2, s32 len) { return detail::_str_cmp(t1, t2, len); }
		/**************************************************************************************************************/
		template<CharType T> static inline bool equ(const T* t1, const T* t2) { return cmp(t1, t2) == 0; }
		template<CharType T> static inline bool equ(const T* t1, const T* t2, s32 len) { return cmp(t1, t2, len) == 0; }
		/**************************************************************************************************************/
		template<CharType T> static inline bool iequ(const T* t1, const T* t2) { return detail::_str_icmp(t1, t2) == 0; }//忽略大小写进行比较 
		template<CharType T> static inline bool iequ(const T* t1, const T* t2, s32 len) { return detail::_str_icmp(t1, t2, len) == 0; }
		/**************************************************************************************************************/ 
		//返回第一个查找到指定字符位置, 否则返回nullptr
		template<CharType T>  static inline T* find(const T* str, T c) { return detail::_str_find(str, c); }
		//返回第一个查找到指定字符串的位置, 否则返回nullptr
		template<CharType T>  static inline T* find(const T* str, const T* sub) { return detail::_str_find(str, sub); }
		//从字符串右侧返回第一个查找到指定字符位置
		static inline char* rfind(const char* str, char c) { return (char*)::strrchr(str, c); } 
		//从字符串右侧返回第一个查找到指定字符位置
		static inline wchar* rfind(const wchar* str, wchar c) { return (wchar*)::wcsrchr(str, c); } 
		/**************************************************************************************************************/
		//反转字符串
		template<CharType T> static inline void flip(T* str, s32 len)
		{
			detail::_str_flip(str, len);
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
				while (len && *begin <= 32) { ++begin; --len; }
				auto dif = s32(begin - str);
				if (dif <= 0) return len;
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
		//格式化数值到字符串缓冲区中
		//返回格式化字符串长度
		template<CharType T>
		static inline s32 format(T* buf, s32 size, bool val)
		{
			if (val)
			{
				if (size < 4) { *buf = 0; return 0; }
				detail::_str_copy(buf, "true", 4); 
				return 4;
			}
			if (size < 5) { *buf = 0; return 0; }
			detail::_str_copy(buf, "false", 5);
			return 5;
		}
		//格式化数值到字符串缓冲区中
		//返回格式化字符串长度
		template<CharType T> static inline s32 format(T* buf, s32 size, T val)
		{
			buf[0] = val;
			buf[1] = 0;
			return 1;
		}
		//格式化数值到字符串缓冲区中
		//返回格式化字符串长度
		template<CharType T, IntType V> static inline s32 format(T* buf, s32 size, V val)
		{
			return detail::_str_format(buf, size, val);
		}
		//格式化数值到字符串缓冲区中
		//返回格式化字符串长度
		template<CharType T, FloatType V> static inline s32 format(T* buf, s32 size, V val, s32 fraction = 0)
		{
			return detail::_str_format(buf, size, val, fraction);
		}
		/**************************************************************************************************************/
		//字符串=>bool, 返回第一个无法解析的字符位置, 失败返回nullptr
		template<CharType T, BoolType V> static inline T* to_val(const T* buf, V& val)
		{
			return detail::_str2val(buf, val);
		}
		//字符串=>浮点数 或 整型, 返回第一个无法解析的字符位置, 失败返回nullptr
		template<CharType T, IntAndFloatType V> static inline T* to_val(const T* buf, V& val)
		{
			detail::Str2Val s2v;
			auto ret = detail::_str2val(buf, s2v);
			val = (V)s2v.val_;
			return ret;
		}

		/**************************************************************************************************************/
		//跳过utf8的头部tag
		static inline char* skip_utf8_bom(const char* buf)
		{
			u8* data = (u8*)buf;
			if (data[0] == 0xEF && data[1] == 0xBB && data[2] == 0xBF)
			{
				return (char*)(buf + 3);
			}
			return nullptr;
		}
		//判断多字节字符串是什么编码, utf8还是ASCII(GBK)
		static inline CharCode judge_char_code(const char* str);
	};

	//产量字符串视图, 类似标准库的std::string_view 和 win32驱动中的UnicodeString
	template<CharType T>
	class StrView
	{
	public:
		s32 len_ = 0;
		const T* str_ = nullptr;
		 
		StrView() { clan_CheckClass(StrView); }
		StrView(const std::nullptr_t&) {}
		StrView(const T* str) { str_ = str; len_ = CStr::len(str); }
		StrView(const StrView& str) { str_ = str.str_; len_ = str.len_; }
		StrView(const T* str, s32 len) { str_ = str; len_ = len; }
		/*#######################################################################################*/
		StrView& operator=(const T* str) { new(this)StrView(str); return *this; }
		StrView& operator=(const StrView& str) { str_ = str.str_; len_ = str.len_; return *this; }
		/*#######################################################################################*/

		s32 len() const { return len_; }
		const T* data() const { return str_; }
		char operator[](s32 index) const { return str_[index]; }

		operator T* () const { return str_; }
		template<NotCharType R>
		operator R() const
		{
			R val;
			auto end = CStr::to_val(str_, val);
			cl_assert(end != nullptr && *end == 0);
			return val;
		}

		bool operator==(const T* t) const { return CStr::equ(str_, t) == 0; }
		bool operator!=(const T* t) const { return !this->operator==(t); }
		bool operator==(const std::nullptr_t&) const { return len_ == 0; }
		bool operator!=(const std::nullptr_t&) const { return len_ != 0; }
		/*#######################################################################################*/
		//若查找失败返回-1
		s32 find(T c) const { auto p = CStr::find(str_, c); if (!p) return -1; return s32(p - str_); }
		//若查找失败返回-1
		s32 find(s32 index, T c) const { auto p = CStr::find(str_ + index, c); if (!p) return -1; return s32(p - str_); }
		//若查找失败返回-1
		s32 find(const T* str) const { auto p = CStr::find(str_, str); if (!p) return -1; return s32(p - str_); }
		//若查找失败返回-1
		s32 find(s32 index, const T* str) const { auto p = CStr::find(str_ + index, str); if (!p) return -1; return s32(p - str_); }
		//若查找失败返回-1
		s32 rfind(T c) const { auto p = CStr::rfind(str_, c); if (!p) return -1; return s32(p - str_); }
	};

	namespace detail
	{
		//一个指定大小的字符串缓冲区
		template<CharType T, AllocMemType A, s32 N> class _StrBuf
		{
			using ThisType = _StrBuf<T, A, N>;
		public:
			s32 len_ = 0;
			T str_[N];
			 
			_StrBuf() { str_[0] = 0; }
			~_StrBuf() {}
		protected:
			void _need(s32 len) { cl_assert(len + len_ < N); }
			void _move(ThisType* str)
			{
				len_ = str->len_;
				CStr::copy(str_, str->str_);
			}
			s32 _size() const { return N; }
		};

		//一个不是固定大小的字符串缓冲区
		template<CharType T, AllocMemType A> class _StrBuf<T, A, 0>
		{
			using ThisType = _StrBuf<T, A, 0>;
		public:
			s32 size_ = 0; //可以存储的字符数目, 非内存大小
			s32 len_ = 0; 
			T* str_ = nullptr;
		 
			_StrBuf() {}
			~_StrBuf() { if (str_) A().free(str_); }
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
			void _move(ThisType* str)
			{
				if (str_) A().free(str_);

				len_ = str->len_; str->len_ = 0;
				size_ = str->size_; str->size_ = 0;
				str_ = str->str_; str->str_ = nullptr;
			}
			s32 _size() const { return size_; }
		};

		char* _str_empty(const char*);
		wchar* _str_empty(const wchar*);

		char* _str_null(const char*);
		wchar* _str_null(const wchar*);
	}

	//用于格式化时, 指示浮点数保留多少位小数
	struct Fraction
	{
		s32 len_;//小数部分的长度
		Fraction(s32 len = 2) : len_(len) {}
	};

	//为啥要写这个类呢? 一个很简单的问题 std::string str; str.reserve(4096 * 4096); auto p = (char*)str.data(); do some thing with p, .... then crash
	//原因是, VS或者GCC实现时, 判断保留的大小超过一定数目就不会申请这么大的内存, 而是等到具体使用时再申请,
	//然后再一些常规操作, 例如 str = 123;//数值转化为字符串, 或者 int v = str; //字符串转换为数值
	//若使用标准库是非常麻烦
	//然后再就是, 在一个函数内部可以明确只需要使用某个数目的字符串缓冲区, 这时候定义一个std::string, 就会导致内存申请
	//若定义 char buf[32]; 这样..又导致必须使用C方式的函数处理..非常麻烦
	template<CharType T, AllocMemType A, s32 N> class _String : public detail::_StrBuf<T, A, N>
	{
		using Base = detail::_StrBuf<T, A, N>;
		using ThisType = _String<T, A, N>;
		using Base::_need;
		using Base::_move;
		using Base::_size;//可以存储的字符数目, 非内存大小

		void _set(const T* str, s32 len)
		{
			_need(len);
			len_ = len;
			CStr::copy(str_, str);
		}
	public:
		s32 fraction_ = 2;//默认小数位2位
		using Base::str_;
		using Base::len_;

		_String() {}
		_String(const std::nullptr_t&) {}
		_String(const T* str) { _set(str, CStr::len(str)); }
		_String(const StrView<T>& str) { if (str.str_) _set(str.data(), str.len()); }
		template<s32 M>
		_String(const _String<T, A, M>& str) { _set(str.str_, str.len_); }
		_String(const ThisType& str) { _set(str.str_, str.len_); } //无法取消,否则会提示调用已删除的函数  
		_String(ThisType&& str) noexcept { _move(&str); }

		_String& operator=(const T* str) { if (str == nullptr) _set(detail::_str_empty(str), 0); else _set(str, CStr::len(str)); return *this; }
		_String& operator=(const StrView<T>& str) { _set(str.data(), str.len()); return *this; }
		template<s32 M>
		_String& operator=(const _String<T, A, M>& str) noexcept { _set(str.str_, str.len_); return *this; }
		_String& operator=(ThisType&& str) noexcept { _move(&str); return *this; }
		template<ValType R> _String& operator=(const R& val) { len_ = 0; return operator<<(val); }

		T* data() const { return (T*)str_; }
		s32 len() const { return len_; }
		s32 size() const { return _size(); }
		void need(s32 len) { _need(len); }
		void copy(const T* str, s32 len) { _set(str, len); }

		StrView<T> view() const { return StrView<T>(str_, len_); }
		T& operator[](s32 index) const { return str_[index]; }

		operator StrView<T>() const { return view(); }

		operator T* () const { return str_ ? str_ : detail::_str_empty(str_); }
		template<NotCharType R>
		operator R() const
		{
			R val;
			auto end = CStr::to_val(str_, val);
			cl_assert(end != nullptr && *end == 0);
			return val;
		}
		/*#######################################################################################*/
		T& back() { return str_[len_ - 1]; }
		void pop(s32 len = 1) { len_ -= len; str_[len_] = 0; }
		_String& push(T c) { _need(1); str_[len_++] = c; str_[len_] = 0; return *this; }

		_String& push(const T* str, s32 len)
		{
			_need(len);
			CStr::copy(str_ + len_, str, len);
			len_ += len;
			str_[len_] = 0;
			return *this;
		}
		/*#######################################################################################*/
		void trim() { len_ = CStr::trim(str_, len_); }
		/*#######################################################################################*/
		//若查找失败返回-1
		s32 find(T c) const { auto p = CStr::find(str_, c); if (!p) return -1; return s32(p - str_); }
		//若查找失败返回-1
		s32 find(s32 index, T c) const { auto p = CStr::find(str_ + index, c); if (!p) return -1; return s32(p - str_); }
		//若查找失败返回-1
		s32 find(const T* str) const { auto p = CStr::find(str_, str); if (!p) return -1; return s32(p - str_); }
		//若查找失败返回-1
		s32 find(s32 index, const T* str) const { auto p = CStr::find(str_ + index, str); if (!p) return -1; return s32(p - str_); }
		//若查找失败返回-1
		s32 rfind(T c) const { auto p = CStr::rfind(str_, c); if (!p) return -1; return s32(p - str_); }
		/*###############################################################################*/
		//移除指定索引开始的指定长度字符串
		void remove(s32 index, s32 len)
		{
			cl_assert(index + len <= len_);

			s32 tail = len_ - index - len;
			CStr::copy(str_ + index, str_ + index + len, tail);
			len_ -= len;
			str_[len_] = 0;
		}
		void remove(const T* str, s32 len)
		{
			auto p = CStr::find(str_, str);
			if (p) remove(s32(p - str_), len);
		}
		void remove(const T* str) { remove(str, CStr::len(str)); }
		void remove(const StrView<T>& str) { remove(str.data(), str.len()); }
		/*###############################################################################*/
		void replace(T src, T dst) { CStr::replace(str_, src, dst); }
		/*###############################################################################*/
		s32 sub(T* buf, s32 start, s32 len) const
		{
			cl_assert(len_ >= start);
			s32 tail = len_ - start;
			if (len > tail) len = tail;
			CStr::copy(buf, 9999, str_ + start, len);
			return len;
		}

		s32 sub(T* buf, s32 start) const { return sub(buf, start, len_); }
		/*###############################################################################*/
	public:
		_String& operator=(const Fraction& f) { fraction_ = f.len_; return *this; }
		_String& operator<<(const Fraction& f) { fraction_ = f.len_; return *this; }
		_String& operator<<(const std::nullptr_t&) { return push(detail::_str_null((const T*)0), 4); }

		//char or wchar
		_String& operator<<(const char& val) { return push((T)val); }
		_String& operator<<(const wchar& val) { return push((T)val); }

		template<NotFloatType R>
		_String& operator<<(const R& val)
		{
			T buf[64];
			s32 len = CStr::format(buf, 64, val);
			return push((const T*)buf, len);
		}
		template<FloatType R>
		_String& operator<<(const R& val)
		{
			T buf[64];
			s32 len = CStr::format(buf, 64, val, fraction_);
			return push((const T*)buf, len);
		}
		_String& operator<<(const T* str) { return push(str, CStr::len(str)); }
		_String& operator<<(const StrView<T>& str) { return push(str.data(), str.len()); }
		template<s32 M> _String& operator<<(const _String<T, A, M>& str) { return push(str.data(), str.len()); }

		/*###############################################################################*/
	private:
		bool _equ(const T* t) const
		{
			auto src = str_ ? str_ : detail::_str_empty(str_);
			return CStr::equ(src, t);
		}
	public:
		bool operator==(const std::nullptr_t&) const { return len_ == 0; }
		bool operator!=(const std::nullptr_t&) const { return len_ != 0; }

		bool operator==(T* t) const { return _equ(t); }
		bool operator==(const T* t) const { return _equ(t); }
		bool operator==(const StrView<T>& t) const { return _equ(t.data()); }
		template<s32 M>
		bool operator==(const _String<T, A, M>& t) const { return _equ(t.data()); }
		template<typename T> bool operator!=(const T& t) const { return !operator==(t); }
	};

	namespace detail
	{
		//工具函数, 用于库开发
		//允许多字节字符串直接赋值到宽字节, 或反之
		template<typename T, typename A, s32 N, typename C>
		void _xString_push(_String<T, A, N>& str, const C* con)
		{
			while (*con) str.push(*con++);
		}
	}
}

#endif//__clan_base_str__ 