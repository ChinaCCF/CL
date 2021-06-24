#ifndef __clan_str__
#define __clan_str__

#include "6mem.h"
#include <string.h>

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

	template<CharsType T>
	inline bool is_num(T c) { return EnumType<CharProperty>(char_property((u8)c)) == CharProperty::Num; }

	template<CharsType T>
	inline bool is_letter(T c) { return EnumType<CharProperty>(char_property((u8)c)) & CharProperty::Letter; }

	u8 char_lower(u8 c);
	u8 char_upper(u8 c);

	namespace detail
	{
		//返回字符串长度, 调用默认的strcpy是会报错的(从这点来说, 这个手动实现的函数也不安全)
		template<CharsType T>
		inline s32 _strcpy(T* t1, const T* t2)
		{
			auto org = t1;
			while (*t2) { *t1++ = *t2++; }
			*t1 = 0;
			return s32(t1 - org);
		}

		template<CharsType T>
		inline s32 _strcpy_s(T* t1, s32 size, const T* t2)
		{
			auto org = t1;
			while (--size && *t2) { *t1++ = *t2++; }
			*t1 = 0;
			return s32(t1 - org);
		}

		template<CharsType T>
		inline s32 _strcpy_ns(T* t1, s32 size, const T* t2, s32 len)
		{
			auto org = t1;
			while (--size && len-- && *t2) { *t1++ = *t2++; }
			*t1 = 0;
			return s32(t1 - org);
		}
		template<CharsType T>
		inline void _flip(T* str, s32 len)
		{
			T* end = str + len - 1;
			while (str < end) swap(*str++, *end--);
		}
		/**************************************************************************************************************/
		//返回字符串长度 
		template<CharsType T, IntType V>
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

		template<CharsType T, UintType V>
		inline s32 _val2str(T* buf, s32 size, V val)
		{
			return _uval2str(buf, size, val);
		}

		template<CharsType T, SintType V>
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

		template<CharsType T, FloatType V>
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
		template<CharsType T> static inline void lower(T* str) { while (*str) { *str = char_lower((u8)*str); ++str; } }
		template<CharsType T> static inline void upper(T* str) { while (*str) { *str = char_upper((u8)*str); ++str; } }
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
		template<CharsType T> static inline void flip(T* str, s32 len)
		{
			detail::_flip(str, len);
		}
		/**************************************************************************************************************/
		//去掉字符串末端无法显示的字符,返回裁剪后的长度
		template<CharsType T> static inline s32 trim_back(T* str, s32 len)
		{
			auto end = str + len - 1;
			while (*end > 0 && *end <= 32 && end >= str) --end;
			end[1] = 0;
			return s32(end - str) + 1;
		} 
		//去掉字符串前端和末端无法显示的字符,返回裁剪后的长度
		template<CharsType T> static inline s32 trim(T* str, s32 len)
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
		template<CharsType T> static inline void replace(T* str, T src, T dst)
		{
			while (*str)
			{
				if (*str == src) *src = dst;
				++str;
			}
		}
		/**************************************************************************************************************/
		template<CharsType T>
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
		template<CharsType T> static inline s32 convert(T* buf, s32 size, T val)
		{
			buf[0] = val;
			buf[1] = 0;
			return 1;
		} 
		template<CharsType T, IntType V> static inline s32 convert(T* buf, s32 size, V val)
		{
			return detail::_val2str(buf, size, val);
		} 
		template<CharsType T, FloatType V> static inline s32 convert(T* buf, s32 size, V val, s32 fraction = 0)
		{
			return detail::_fval2str(buf, size, val, fraction);
		}
		/**************************************************************************************************************/ 
		template<CharsType T, BoolType V> static inline T* to(const T* buf, V& val)
		{
			return detail::_str2val(buf, val);
		}

		template<CharsType T, FloatType V> static inline T* to(const T* buf, V& val)
		{
			f64 tmp;
			auto ret = detail::_str2val(buf, tmp);
			val = (V)tmp;
			return ret;
		}

		template<CharsType T, IntType V> static inline T* to(const T* buf, V& val)
		{
			s64 tmp;
			auto ret = detail::_str2val(buf, tmp);
			val = (V)tmp;
			return ret;
		}
		template<CharsType T> static inline T* to(const T* buf, T& val)
		{
			s64 tmp;
			auto ret = detail::_str2val(buf, tmp);
			val = (T)tmp;
			return ret;
		}
		/**************************************************************************************************************/
		static inline CharCode judge_char_code(const char* str);
	};


}

#endif//__clan_str__ 