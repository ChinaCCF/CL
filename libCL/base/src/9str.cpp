#include "../9str.h"
#include <Windows.h>

namespace cl
{
	//linux  16 2 8
				//const char* page = "zh_CN.gbk";
				//if (!is_gbk) page = "zh_CN.utf8";
				//set_code_page(page);
				//s32 size = len * 4 + 1;
				//string ret(size);
				//char* p = ret.data();
				//auto cnt = (s32)wcstombs(p, str, size - 1);
				//p[cnt] = 0;
				//ret.set_len(cnt - 1);
				//return ret;

	//8 2 16
				//	const char* page = "zh_CN.gbk";
				//if (!is_gbk) page = "zh_CN.utf8";
				//set_code_page(page);
				//s32 size = len + 1;
				//auto ws = (wchar*)cl_alloc(size * sizeof(wchar));
				//auto end = mbstowcs(ws, str, len);
				//ws[end] = ws[len] = 0;
				//WStr ret
				//	ret.str_ = ws;
				//ret.len_ = end;
				//return ret;

	StringW c8_2_c16(uv32 code, const uc8* str, uv32 len)
	{
		uv32 size = len + 1;
		StringW ret;
		ret.reserve(size);
		len = MultiByteToWideChar(code, //CodePage
								  0, //Flags
								  (const char*)str, //MultiByte
								  size,
								  ret.data(), //目标缓存
								  size);// in characters  
		ret.length(len - 1);//包含了空白字符 
		return ret;
	}
	String c16_2_c8(uv32 code, const tc16* str, uv32 len)
	{
		uv32 size = len * 4 + 1; //一个宽字节字符极端情况下占据4个字节
		String ret;
		ret.reserve(size);
		len = WideCharToMultiByte(code,//CodePage
								  0,//flags
								  (const wchar*)str,
								  len + 1,// in characters
								  (char*)ret.data(), //目标
								  size,//in bytes
								  nullptr,
								  nullptr);
		ret.length(len - 1);
		return ret;
	}


	StringW UTF8::to_UTF16(const uc8* str, uv32 len)
	{
		return c8_2_c16(CP_UTF8, str, len);
	}
	String UTF8::to_gbk(const uc8* str, uv32 len)
	{
		auto mid = c8_2_c16(CP_UTF8, str, len);
		return c16_2_c8(CP_ACP, (tc16*)mid.data(), mid.length());
	}
	String UTF8::from_UTF16(const tc16* str, uv32 len)
	{
		return c16_2_c8(CP_UTF8, str, len);
	} 
	String UTF8::from_gbk(const ac8* str, uv32 len)
	{
		auto mid = c8_2_c16(CP_ACP, (uc8*)str, len);
		return c16_2_c8(CP_UTF8, (tc16*)mid.data(), mid.length());
	}

	String UTF16::to_gbk(const tc16* str, uv32 len) { return c16_2_c8(CP_ACP, str, len); }
	StringW UTF16::from_gbk(const ac8* str, uv32 len) { return c8_2_c16(CP_ACP, (uc8*)str, len); }

	String rand_str(const tc8* letter, uv32 letter_cnt, uv32 str_dst_len)
	{
		//CL_Assert(letter_cnt < 128);

		String ret;
		ret.reserve(str_dst_len + 2);
		 
		auto p = ret.data();
		uv32 index = 0;
		while(index < str_dst_len)
		{
			uv32 r = Random::ma_x(0x7FFF);
			p[index++] = letter[(r & 0xFF) % letter_cnt];
			p[index++] = letter[((r >> 8) & 0xFF) % letter_cnt];
		}
		ret.length(str_dst_len);
		return ret;
	}
}