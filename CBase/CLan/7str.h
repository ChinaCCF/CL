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

	//这样写非常慢!!!!
	//for(s32 i = 0; i < len; i++) str[i] = g_char_lower[str[i]];
	template<CharsType T> inline void str_lower(T* str) { while (*str) { *str = char_lower((u8)*str); ++str; } }
	template<CharsType T> inline void str_upper(T* str) { while (*str) { *str = char_upper((u8)*str); ++str; } }

	inline s32 strlen(const char* str) { return (s32)::strlen(str); }
	inline s32 strlen(const wchar* str) { return (s32)::wcslen(str); }

	inline s32 strcmp(const char* str1, const char* str2) { return ::strcmp(str1, str2); }
	inline s32 strcmp(const wchar* str1, const wchar* str2) { return ::wcscmp(str1, str2); }

	inline s32 strcmp(const char* str1, const char* str2, s32 len) { return ::strncmp(str1, str2, len); }
	inline s32 strcmp(const wchar* str1, const wchar* str2, s32 len) { return ::wcsncmp(str1, str2, len); }

	inline bool strequ(const char* t1, const char* t2) { return strcmp(t1, t2) == 0; }
	inline bool strequ(const wchar* t1, const wchar* t2) { return strcmp(t1, t2) == 0; }

	inline bool strequ(const char* t1, const char* t2, s32 len) { return ::strncmp(t1, t2, len) == 0; }
	inline bool strequ(const wchar* t1, const wchar* t2, s32 len) { return ::wcsncmp(t1, t2, len) == 0; }

	//忽略大小写进行比较
	inline bool striequ(const char* t1, const char* t2) { return ::_stricmp(t1, t2) == 0; }
	inline bool striequ(const wchar* t1, const wchar* t2) { return ::_wcsicmp(t1, t2) == 0; }

	inline bool strniequ(const char* t1, const char* t2, s32 len) { return ::_strnicmp(t1, t2, len) == 0; }
	inline bool strniequ(const wchar* t1, const wchar* t2, s32 len) { return ::_wcsnicmp(t1, t2, len) == 0; }
}

#endif//__clan_str__ 