#include "../8cstr.h"

namespace cl
{
	namespace lib
	{
		//1字节 0xxxxxxx 
		//2字节 110xxxxx 10xxxxxx //0xC0  1
		//3字节 1110xxxx 10xxxxxx 10xxxxxx //0xE0  2
		//4字节 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx //0xF0  3
		//5字节 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx //0xF8 4
		//6字节 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx //0xFC 5
		CharCode get_char_code(const void* _str)
		{
			auto str = (const uv8*)_str;
			while (*str && *str < 0x80) str++;//跳过ascii部分 
			if (*str == 0) return CharCode::ASCII;//前面都是ascii部分 

			auto c = *str++;

			//判断的原理: 根据字符串是否符合utf8的编码规则来判断
			uv32 remain_cnt = 0;
			do
			{
				if ((c & 0xE0) == 0xC0) { remain_cnt = 1; break; }
				if ((c & 0xF0) == 0xE0) { remain_cnt = 2; break; }
				if ((c & 0xF8) == 0xF0) { remain_cnt = 3; break; }
				if ((c & 0xFC) == 0xF8) { remain_cnt = 4; break; }
				if ((c & 0xFE) == 0xFC) { remain_cnt = 5; break; }
				return CharCode::GBK;
			} while (false);

			for (uv32 i = 0; i < remain_cnt; i++)
			{
				c = str[i];
				if ((c & 0xC0) != 0x80) return CharCode::GBK;
			}
			return CharCode::UTF8;
		}
		 
		//把小数字符串转为浮点数, 支持科学计数法
		template<CharType T>
		static inline T* _do2fv(const T* str, fv64& val)
		{ 
			val = 0;
			 
			uv64 uv;
			auto end = _2v(str, uv);
			if (end == nullptr) return nullptr;

			val = (fv64)uv;

			str = end;
			if (*str == '.')
			{
				++str;
				end = _2v(str, uv);
				if (end)
				{//整数变为小数 
					fv64 fv = (fv64)uv;
					auto cnt = uv32(end - str);
					while (cnt--) fv /= 10;
					val += fv; 
					str = end;
				}
				else
					return nullptr;
			} 
			if (*str == 'e' || *str == 'E')
			{
				++str;
				sv64 sv;
				end = tov(str, sv);
				if (end)
				{
					if (val < -10 || val > 10) CL_Throw(ExceptionCode::Float_Format_Error, 0, "float type erro!");
					if (sv > 0) while (sv > 0) { val *= 10; --sv; }
					else while (sv < 0) { val /= 10; ++sv; }
					str = end;
				}
				else
				{
					//不强制小数后面带e为错误
					--str;
				}
			}
			return (T*)str;
		}

		void* _2fv(const uc8* str, fv64& val) { return (void*)_do2fv(str, val); }
		void* _2fv(const uc16* str, fv64& val) { return (void*)_do2fv(str, val); }
	} 
}