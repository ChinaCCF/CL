#ifndef __cl_base_cstr__
#define __cl_base_cstr__

#include <string.h>
#include "7call.h"

//重写基础的C字符串操作,兼容多字节和宽字节, 免去记忆多个函数名
namespace cl
{
	template<CharType T>
	static inline T clow(T c) { if (c >= 'A' && c <= 'Z') return c + 32; return c; }
	template<CharType T>
	static inline T cup(T c) { if (c >= 'a' && c <= 'z') return c - 32; return c; }

	template<CharType T>
	static inline uv32 slen(const T* str) { return (uv32)::strlen(str); }
	static inline uv32 slen(const uc16* str) { return (uv32)::wcslen((wchar_t*)str); }

	/*############################################################################################*/
	//srshift
	//指定长度len的字符串向右移动cnt个字符
	/*############################################################################################*/
	template<CharType C>
	static inline void srshift(C* buf, uv32 len, uv32 cnt)
	{
		buf += len;
		for (sv32 i = 0; i < (sv32)cnt; i++)
			buf[cnt - i] = buf[-i];
	}

	/*############################################################################################*/
	//copy
	/*############################################################################################*/
	template<CharType C1, CharType C2>
	static inline void scopy(C1* dst, uv32 size, const C2* src)
	{ 
		while (--size && *src) { *dst++ = *src++; }
		*dst = 0; 
		[[unlikely]]
		if (size == 0) CL_Throw(ExceptionCode::Buffer_OverFlow, 0, "");
	}
	 
	template<CharType C1, CharType C2>
	static inline void scopy(C1* dst, uv32 size, const C2* src, uv32 len)
	{ 
		while (--size && len-- && *src) { *dst++ = *src++; }
		*dst = 0; 
		[[unlikely]]
		if (size == 0) CL_Throw(ExceptionCode::Buffer_OverFlow, 0, "");
	}

	/*############################################################################################*/
	//compare
	/*############################################################################################*/
	template<CharType C1, CharType C2>
	static inline sv32 scmp(const C1* t1, const C2* t2)
	{
		while (*t1 && *t1 == *t2) { ++t1; ++t2; }
		if (*t1 == *t2) return 0;
		return *t1 < *t2 ? -1 : 1;
	}
	template<CharType C1, CharType C2>
	static inline sv32 sicmp(const C1* t1, const C2* t2)
	{
		while (*t1 && clow(*t1) == clow(*t2)) { ++t1; ++t2; }
		if (*t1 == *t2) return 0;
		return clow(*t1) < clow(*t2) ? -1 : 1;
	}

	template<CharType C1, CharType C2>
	static inline sv32 scmp(const C1* t1, const C2* t2, uv32 len)
	{
		while (len-- && *t1 && *t1 == *t2) { ++t1; ++t2; }
		if (len == 0 || *t1 == *t2) return 0;
		return *t1 < *t2 ? -1 : 1;
	}
	template<CharType C1, CharType C2>
	static inline sv32 sicmp(const C1* t1, const C2* t2, uv32 len)
	{
		while (len-- && *t1 && clow(*t1) == clow(*t2)) { ++t1; ++t2; }
		if (len == 0 || *t1 == *t2) return 0;
		return clow(*t1) < clow(*t2) ? -1 : 1;
	}

	template<CharType C1, CharType C2>
	static inline bool sequ(const C1* t1, const C2* t2) { return scmp(t1, t2) == 0; }

	template<CharType C1, CharType C2>
	static inline bool sequ(const C1* t1, const C2* t2, uv32 len) { return scmp(t1, t2, len) == 0; }

	template<CharType C1, CharType C2>
	static inline bool siequ(const C1* t1, const C2* t2) { return sicmp(t1, t2) == 0; }

	template<CharType C1, CharType C2>
	static inline bool siequ(const C1* t1, const C2* t2, uv32 len) { return sicmp(t1, t2, len) == 0; }

	/*############################################################################################*/
	//find
	/*############################################################################################*/
	template<CharType C1, typename C2>
	static inline C1* sfind(const C1* str, C2 c)
	{
		while (*str && *str != c) str++;
		if (*str == 0) return nullptr;
		return (C1*)str;
	} 

	template<CharType C1, CharType C2>
	static inline C1* sfind(const C1* str, const C2* sub, uv32 len)
	{
		while (true)
		{
			while (*str && *str != *sub) str++;
			if (*str == 0) return nullptr;
			if (sequ(str, sub, len)) return (C1*)str;
			++str;
		}
	}

	template<CharType C1, CharType C2>
	static inline C1* sfind(const C1* str, const C2* sub) { return sfind(str, sub, slen(sub)); }
	
	namespace lib
	{
		template<CharType C1, CharType C2>
		static inline C1* sfindr(const C1* start, const C1* end, const C2 c)
		{
			while (*end != c && end > start) --end;
			if (end == start) return nullptr;
			return end;
		}
	} 

	template<CharType C1, CharType C2>
	static inline C1* sfindr(const C1* str, C2 c) { return lib::sfindr(str, str + slen(str), c); }
	template<CharType C1, CharType C2>
	static inline C1* sfindr(const C1* str, uv32 len, C2 c) { return lib::sfindr(str, str + len, c); }
	/*############################################################################################*/
	//replace
	/*############################################################################################*/
	template<CharType T, CharType S, CharType D> static inline void sreplace(T* str, S src, D dst)
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
	static inline void sflip(T* str, uv32 len)
	{
		T* end = str + len - 1;
		while (str < end) swap(*str++, *end--);
	}
	template<CharType T>
	static inline void sflip(T* str) { sflip(str, slen(str)); }
	/*############################################################################################*/
	//大小写转换
	/*############################################################################################*/
	template<CharType T> static inline void slow(T* str) { while (*str) { *str = clow(*str); ++str; } }
	template<CharType T> static inline void sup(T* str) { while (*str) { *str = cup(*str); ++str; } }
	/*############################################################################################*/
	//裁剪
	/*############################################################################################*/
	template<CharType T> static inline uv32 strim_back(T* str, uv32 len)
	{
		auto end = str + len - 1;
		while (*end && *end <= 32 && end >= str) --end;
		end[1] = 0;
		return uv32(end - str) + 1;
	}
	template<CharType T> static inline uv32 strim(T* str, uv32 len)
	{
		len = strim_back(str, len);
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

	namespace lib { CharCode get_char_code(const void* _str); }

	//获取多字节编码
	template<C8Type T>
	CharCode get_char_code(const T* str) { return lib::get_char_code(str); }
  
	/*############################################################################################*/
	// format 
	/*############################################################################################*/
	template<CharType T>
	struct FloatStrInfo
	{
		uv16 number_;
		uv16 fraction_;
		T* str_;
	};

	namespace lib
	{
		template<CharType T, UintType V>
		static inline uv32 _2s(T* buf, uv32 size, V val)
		{
			if (size <= 1) { *buf = 0; return 0; }

			auto p = buf;
			do
			{
				*p++ = val % 10 + '0';
				val /= 10;
				--size;
			}while (val != 0 && size > 1);
			//缓冲区不足, 有一种情况, val==0, size==1, 这里也会认为是缓冲不足
			if (size <= 1) { *buf = 0; return 0; }

			auto len = uv32(p - buf);
			sflip(buf, len);
			*p = 0;
			return len;
		}
		 
		//fval必须为正数
		template<CharType T, FloatType V>
		static inline FloatStrInfo<T> _2s(T* buf, uv32 size, V fval, uv32 fraction = 6)
		{
			FloatStrInfo<T> ret;
			ret.str_ = nullptr;

			auto p = buf;

			uv64 uval = (uv64)fval;
			ret.number_ = _2s(p, size, uval);

			if (ret.number_ == 0) return ret;
			//                      '.'             0
			if (size < ret.number_ + 1 + fraction + 1) { ret.number_ = 0; return ret; }
			 
			auto cnt = fraction;
			{//计算小数
				fval -= uval;  
				while (cnt--) fval *= 10;

				uval = (uv64)fval;
				fval -= uval;
				if (fval > 0.5) uval += 1;
			}

			p += ret.number_;
			*p++ = '.';

			cnt = _2s(p, size - ret.number_ - 1, uval);
			auto dif = fraction - cnt;
			srshift(p, cnt, dif);
			  
			for (uv32 i = 0; i < dif; i++)
				p[i] = '0';

			ret.fraction_ = fraction;
			ret.str_ = buf;
			return ret;
		}
	}

	template<CharType T, CharType C>
	static inline uv32 tos(T* buf, uv32 size, C val)
	{
		buf[0] = val;
		buf[1] = 0;
		return 1;
	}

	template<CharType T>
	static inline uv32 tos(T* buf, uv32 size, bool val)
	{
		if (size < 6) { *buf = 0; return 0; }

		if (val)
		{
			scopy(buf, size, "true", 4);
			return 4;
		}
		scopy(buf, size, "false", 5);
		return 5;
	}

	template<CharType T, UintType V>
	static inline uv32 tos(T* buf, uv32 size, V val) { return lib::_2s(buf, size, val); }

	template<CharType T, SintType V>
	static inline uv32 tos(T* buf, uv32 size, V val)
	{ 
		if (val < 0)
		{
			val = -val;
			*buf++ = '-';
			size--; 
			return lib::_2s(buf, size, (uv64)val) + 1;
		}
		return lib::_2s(buf, size, (uv64)val); 
	}

	template<CharType T, FloatType V>
	static inline uv32 tos(T* buf, uv32 size, V val, uv32 fraction = 6)
	{
		uv32 len = 0;
		if (val < 0)
		{
			val = -val;
			*buf++ = '-';
			size--;
			len++;
		}
		auto ret = lib::_2s(buf, size, val, fraction);
		if (ret.number_ != 0)
			return ret.number_ + 1 +  ret.fraction_ + len;
		return 0;
	}
	/**************************************************************************************************************/
	// unformat
	/**************************************************************************************************************/
	namespace lib
	{  
		//返回第一个无法解析的字符位置
		template<CharType T>
		static inline T* _2v(const T* str, uv64& val)
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

		void* _2fv(const uc8* str, fv64& val);
		void* _2fv(const uc16* str, fv64& val);

		template<CharType T>
		static inline T* _2v(const T* str, fv64& val)
		{
			using V = typename SameSizeType<T, uc8, uc16>::type;
			return (T*)_2fv((V*)str, val);
		} 
	}

	template<CharType T>
	static inline T* tov(const T* str, bool& val)
	{
		T buf[8];
		{
			scopy(buf, 8, str, 5);//false, 5个字节 
			buf[5] = 0;
			slow(buf);
		}

		if (sequ(buf, "false"))
		{
			val = false;
			return (T*)(str + 5);
		}
		buf[4] = 0;
		if (sequ(buf, "true"))
		{
			val = true;
			return (T*)(str + 4);
		}
		return nullptr;
	}
	template<CharType C, UintType T>
	static inline C* tov(const C* str, T& val) 
	{
		uv64 tv;
		auto ret = lib::_2v(str, tv); 
		val = (T)tv;
		return ret;
	}

	template<CharType C, typename T>
	requires IsSint_v<T> || IsFloat_v<T>
	static inline C* tov(const C* str, T& val) 
	{
		bool neg = false;
		if (*str == '-') { neg = true; ++str; }
		using V = typename SelectType<IsSint_v<T>, uv64, fv64>::type;
		V tv;
		auto ret = lib::_2v(str, tv); 
		val = (T)tv;
		if (neg) val = -val; 
		return ret;
	} 
}

#endif//__cl_base_cstr__ 