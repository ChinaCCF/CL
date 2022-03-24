#ifndef __cl_base_cstr__
#define __cl_base_cstr__

#include <string.h>
#include "7call.h"

//重写基础的C字符串操作,兼容多字节和宽字节, 免去记忆多个函数名
namespace cl
{
	template<CharType T>
	static inline T clower(T c) { if (c >= 'A' && c <= 'Z') return c + 32; return c; }
	template<CharType T>
	static inline T cupper(T c) { if (c >= 'a' && c <= 'z') return c - 32; return c; }

	namespace cstr
	{ 
		template<CharType T>
		static inline uv32 length(const T* str) { return (uv32)::strlen(str); }
		static inline uv32 length(const uc16* str) { return (uv32)::wcslen((wchar*)str); }

		/*############################################################################################*/
		//rshift
		//指定长度len的字符串向右移动cnt个字符
		//"ABC" 右移 1 个字符,变为 " ABC"
		/*############################################################################################*/
		template<CharType C>
		static inline void rshift(C* buf, uv32 len, uv32 cnt)
		{
			mem::rshift(buf, len, cnt);
			buf[len + cnt] = 0; 
		}

		/*############################################################################################*/
		//copy
		/*############################################################################################*/
		template<CharType C1, CharType C2>
		static inline void copy(C1* dst, const C2* src, uv32 len)
		{
			while (len-- && *src) { *dst++ = *src++; }
			*dst = 0; 
		}

		template<CharType C1, CharType C2>
		static inline void copy(C1* dst, uv32 size, const C2* src)
		{
			while (--size && *src) { *dst++ = *src++; }
			*dst = 0;
			[[unlikely]]
			if (size == 0) CL_Throw(ExceptionCode::Buffer_OverWrite, 0, "");
		}

		template<CharType C1, CharType C2>
		static inline void copy(C1* dst, uv32 size, const C2* src, uv32 len)
		{
			if (len >= size) CL_Throw(ExceptionCode::Buffer_OverWrite, 0, "");
			for (uv32 i = 0; i < len; i++) dst[i] = src[i];
			dst[len] = 0; 
		}
		/*############################################################################################*/
		//compare
		/*############################################################################################*/
		template<CharType C1, CharType C2>
		static inline sv32 cmp(const C1* t1, const C2* t2)
		{
			while (*t1 && *t1 == *t2) { ++t1; ++t2; }
			if (*t1 == *t2) return 0;
			return *t1 < *t2 ? -1 : 1;
		}
		template<CharType C1, CharType C2>
		static inline sv32 cmp(const C1* t1, const C2* t2, uv32 len)
		{
			while (len-- && *t1 && *t1 == *t2) { ++t1; ++t2; }
			if (len == 0 || *t1 == *t2) return 0;
			return *t1 < *t2 ? -1 : 1;
		}

		template<CharType C1, CharType C2>
		static inline sv32 icmp(const C1* t1, const C2* t2)
		{
			while (*t1 && clower(*t1) == clower(*t2)) { ++t1; ++t2; }
			if (*t1 == *t2) return 0;
			return clower(*t1) < clower(*t2) ? -1 : 1;
		} 
		template<CharType C1, CharType C2>
		static inline sv32 icmp(const C1* t1, const C2* t2, uv32 len)
		{
			while (len-- && *t1 && clower(*t1) == clower(*t2)) { ++t1; ++t2; }
			if (len == 0 || *t1 == *t2) return 0;
			return clower(*t1) < clower(*t2) ? -1 : 1;
		}

		template<CharType C1, CharType C2>
		static inline bool equ(const C1* t1, const C2* t2) { return cmp(t1, t2) == 0; }

		template<CharType C1, CharType C2>
		static inline bool equ(const C1* t1, const C2* t2, uv32 len) { return cmp(t1, t2, len) == 0; }

		template<CharType C1, CharType C2>
		static inline bool iequ(const C1* t1, const C2* t2) { return icmp(t1, t2) == 0; }

		template<CharType C1, CharType C2>
		static inline bool iequ(const C1* t1, const C2* t2, uv32 len) { return icmp(t1, t2, len) == 0; }

		/*############################################################################################*/
		//find
		/*############################################################################################*/
		template<CharType C1, typename C2>
		static inline C1* find(const C1* str, C2 c)
		{
			while (*str && *str != c) str++;
			if (*str == 0) return nullptr;
			return (C1*)str;
		}

		template<CharType C1, CharType C2>
		static inline C1* find(const C1* str, const C2* sub, uv32 len)
		{
			while (true)
			{
				while (*str && *str != *sub) str++;
				if (*str == 0) return nullptr;
				if (equ(str, sub, len)) return (C1*)str;
				++str;
			}
		}

		template<CharType C1, CharType C2>
		static inline C1* find(const C1* str, const C2* sub) { return find(str, sub, length(sub)); }
		 
		namespace lib
		{
			template<CharType C1, CharType C2>
			static inline C1* findr(const C1* start, const C1* end, const C2 c)
			{
				while (*end != c && end > start) --end;
				if (end == start) return nullptr;
				return end;
			}
		} 
		template<CharType C1, CharType C2>
		static inline C1* findr(const C1* str, C2 c) { return lib::findr(str, str + length(str), c); }
		template<CharType C1, CharType C2>
		static inline C1* findr(const C1* str, uv32 len, C2 c) { return lib::findr(str, str + len, c); }
		/*############################################################################################*/
		//replace
		/*############################################################################################*/
		template<CharType T, CharType S, CharType D> static inline void replace(T* str, S src, D dst)
		{
			while (*str)
			{
				if (*str == src) *str = dst;
				++str;
			}
		}
		/*############################################################################################*/
		//反转指定长度字符串
		/*############################################################################################*/
		template<CharType T>
		static inline void flip(T* str, uv32 len)
		{
			T* end = str + len - 1;
			while (str < end) swap(*str++, *end--);
		}
		template<CharType T>
		static inline void flip(T* str) { flip(str, length(str)); }
		/*############################################################################################*/
		//大小写转换
		/*############################################################################################*/
		template<CharType T> static inline void lower(T* str) { while (*str) { *str = clower(*str); ++str; } }
		template<CharType T> static inline void upper(T* str) { while (*str) { *str = cupper(*str); ++str; } }

		/*############################################################################################*/
		//裁剪
		/*############################################################################################*/
		template<CharType T> static inline uv32 trim_back(T* str, uv32 len)
		{
			auto end = str + len - 1;
			while (*end && *end <= 32 && end >= str) --end;
			end[1] = 0;
			return uv32(end - str) + 1;
		}
		template<CharType T> static inline uv32 trim(T* str, uv32 len)
		{
			len = trim_back(str, len);
			if (len)
			{
				T* p = str;
				while (len && *p <= 32) { ++p; --len; }
				auto pre = uv32(p - str);
				if (pre <= 0) return len;
				for (uv32 i = 0; i < len; i++)
					str[i] = p[i];
				str[len] = 0;
			}
			return len;
		}
		/*############################################################################################*/
		//start_with
		/*############################################################################################*/
		template<CharType T1, CharType T2> static inline bool start_with(const T1* s1, const T2* s2)
		{
			while (*s1 && *s2 && *s1 == *s2) { ++s1; ++s2; }
			return *s2 == 0;
		}
		/*############################################################################################*/
		// format 
		/*############################################################################################*/
		template<CharType T>
		struct FloatStrInfo
		{
			uv16 number_cnt_;//正整数部分
			uv16 fraction_cnt_;//小数部分
			T* str_;
		};

		namespace lib
		{
			//转换无符号整数, 返回转换的字符串长度
			//失败 0 个字符转换
			template<CharType T, UintType V>
			static inline uv32 _from_val(T* buf, uv32 size, V val)
			{
				if (size <= 1) { *buf = 0; return 0; }

				auto p = buf;
				do
				{
					*p++ = val % 10 + '0';
					val /= 10;
					--size;
				} while (val != 0 && size > 1);

				if (val != 0 && size <= 1) { *buf = 0; return 0; }

				auto len = uv32(p - buf);
				flip(buf, len);
				*p = 0;
				return len;
			}

			//转换失败 number_cnt_ == 0
			FloatStrInfo<uc8> _from_fval(uc8* buf, uv32 size, fv64 fval, uv32 fraction_cnt);
			//转换失败 number_cnt_ == 0
			FloatStrInfo<uc16> _from_fval(uc16* buf, uv32 size, fv64 fval, uv32 fraction_cnt);
		}

		//
		template<CharType T, CharType C>
		static inline uv32 from_val(T* buf, uv32 size, C val)
		{
			buf[0] = val;
			buf[1] = 0;
			return 1;
		}

		template<CharType T>
		static inline uv32 from_val(T* buf, uv32 size, bool val)
		{
			if (size < 6) { *buf = 0; return 0; }

			if (val)
			{
				copy(buf, size, "true", 4);
				return 4;
			}
			copy(buf, size, "false", 5);
			return 5;
		}

		template<CharType T, UintType V>
		static inline uv32 from_val(T* buf, uv32 size, V val) { return lib::_from_val(buf, size, val); }

		template<CharType T, SintType V>
		static inline uv32 from_val(T* buf, uv32 size, V val)
		{
			if (val < 0)
			{
				val = -val;
				*buf++ = '-';
				size--;
				return lib::_from_val(buf, size, (uv64)val) + 1;
			}
			return lib::_from_val(buf, size, (uv64)val);
		}

		template<CharType T, FloatType V>
		static inline uv32 from_val(T* buf, uv32 size, V val, uv32 fraction = 6)
		{
			typedef SelectType_t<sizeof(T) == sizeof(uc8), uc8, uc16> TC;

			uv32 len = 0;
			if (val < 0)
			{
				val = -val;
				*buf++ = '-';
				size--;
				len++;
			}
			auto ret = lib::_from_fval((TC*)buf, size, (fv64)val, fraction);
			if (ret.number_cnt_ != 0) return ret.number_cnt_ + 1 + ret.fraction_cnt_ + len;
			return 0;
		}
		/**************************************************************************************************************/
		// unformat
		/**************************************************************************************************************/
		namespace lib
		{
			//返回第一个无法解析的字符位置
			template<CharType T>
			static inline T* _2_val(const T* str, uv64& val)
			{
				val = 0;

				auto p = str;
				sv8 v;
				while (true)
				{
					v = *p - '0';
					if (v < 0 || v >= 10) break;
					val = val * 10 + v;
					++p;
				}
				if (p == str) return nullptr;
				return (T*)p;
			}

			void* _2_fval(const uc8* str, fv64& val);
			void* _2_fval(const uc16* str, fv64& val);

			template<CharType T>
			static inline T* _2_val(const T* str, fv64& val)
			{
				using V = typename SameSizeType<T, uc8, uc16>::type;
				return (T*)_2_fval((V*)str, val);
			}
		}

		template<CharType T>
		static inline T* to_val(const T* str, bool& val)
		{
			T buf[8];
			{
				copy(buf, 8, str, 5);//false, 5个字节 
				buf[5] = 0;
				lower(buf);
			}

			if (equ(buf, "false"))
			{
				val = false;
				return (T*)(str + 5);
			}
			buf[4] = 0;
			if (equ(buf, "true"))
			{
				val = true;
				return (T*)(str + 4);
			}
			return nullptr;
		}
		template<CharType C, UintType T>
		static inline C* to_val(const C* str, T& val)
		{
			uv64 tv;
			auto ret = lib::_2_val(str, tv);
			val = (T)tv;
			return ret;
		}

		template<CharType C, typename T>
			requires IsSint_v<T> || IsFloat_v<T>
		static inline C * to_val(const C * str, T & val)
		{
			bool neg = false;
			if (*str == '-') { neg = true; ++str; }
			using V = typename SelectType<IsSint_v<T>, uv64, fv64>::type;
			V tv;
			auto ret = lib::_2_val(str, tv);
			val = (T)tv;
			if (neg) val = -val;
			return ret;
		}
	}
	 
	/*############################################################################################*/
	//utf8相关
	/*############################################################################################*/

	//跳过utf8的头部tag
	template<C8Type T>
	static inline T* skip_utf8_bom(const T* buf)
	{
		uv8* data = (uv8*)buf;
		if (data[0] == 0xEF && data[1] == 0xBB && data[2] == 0xBF)
		{
			return (T*)(buf + 3);
		}
		return (T*)buf;
	}

	enum class CharCode
	{
		Unknow,
		ASCII,
		UTF8,
		GBK,
	};

	namespace lib { CharCode _get_char_code(const void* _str); }

	//获取多字节编码
	template<C8Type T>
	CharCode get_char_code(const T* str) { return lib::_get_char_code(str); }
  
	
}

#endif//__cl_base_cstr__ 