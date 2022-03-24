#include "../2type.h"  
#include <string.h>

namespace cl
{ 
	static inline uv32 slen(const uc16* str)
	{
		auto start = str;
		while (*str) ++str;
		return (uv32)(str - start);
	}
	static inline uv32 scopy(_out uc16* dst, uv32 size, const uc16* src)
	{
		auto len = slen(src);
		if (len >= size) return 0;
		for (uv32 i = 0; i < len; i++) dst[i] = src[i]; 
		dst[len] = 0;
		return len;
	}

	Exception::Exception(const uc16* file, uv32 line, uv16 main_code, uv16 sub_code, const uc16* msg)
	{
		buf_[0] = 0;

		line_ = line;
		main_code_ = main_code;
		sub_code_ = sub_code;

		file_ = buf_;
		file_[0] = 0;
		auto len = scopy(file_, 512, file);
		if (len == 0) return;
		 
		len += 1;
		msg_ = file_ + len;
		msg_[0] = 0;
		scopy(msg_, 512 - len, msg);
	}
}