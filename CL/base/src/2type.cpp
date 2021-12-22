#include "../2type.h"  
#include <string.h>

namespace cl
{ 
	static inline void scopy(uc16* dst, const uc16* src)
	{
		while (*src) { *dst++ = *src++; }
		*dst = 0; 
	}

	Exception::Exception(const uc16* file, uv32 line, uv16 main_code, uv16 sub_code, const uc16* msg)
	{
		buf_[0] = 0;

		line_ = line;
		main_code_ = main_code;
		sub_code_ = sub_code;

		file_ = buf_; 
		auto len = wcslen((wchar_t*)file);
		if (len >= 512) return; 
		scopy(file_, file);
		
		msg_ = file_ + len;
		auto remain = 512 - len;
		len = wcslen((wchar_t*)msg);
		if (len >= remain)
		{
			msg_[0] = 0;
			return;
		} 
		scopy(msg_, msg);
	}
}