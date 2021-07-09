#include "../xml.h"

namespace cl
{
	namespace detail
	{
        template<CharType C>
		s32 _xml_decode_str(C* dst, const C* src)
		{
			auto org = dst;
			while (*src)
			{
				if (*src == '&')
				{
					if (_str_equ(src, "&lt;", 4))
					{
						*dst++ = '<';
						src += 4;
						continue;
					}

					if (_str_equ(src, "&gt;", 4))
					{
						*dst++ = '>';
						src += 4;
						continue;
					}

					if (_str_equ(src, "&amp;", 5))
					{
						*dst++ = '&';
						src += 5;
						continue;
					}

					if (_str_equ(src, "&apos;", 6))
					{
						*dst++ = '\'';//µ¥ÒýºÅ
						src += 6;
						continue;
					}

					if (_str_equ(src, "&quot;", 6))
					{
						*dst++ = '\"';//Ë«ÒýºÅ
						src += 6;
						continue;
					}
					cl_assert(false);
				}
				else
				{
					*dst++ = *src++;
				}
			}
			*dst = 0;
			return s32(dst - org);
        } 

        template<CharType C>
        s32 _xml_encode_str(C* dst, s32 size, const C* src)
        {
            auto org = dst;

            while (*src)
            {
                switch (*src)
                { 
                    case '<':
                    {
                        if (size <= 4) return 0;
                        _str_copy(dst, "&lt;");
                        dst += 4;
                        size -= 4;
                    }break;
                    case '>':
                    {
                        if (size <= 4) return 0;
                        _str_copy(dst, "&gt;");
                        dst += 4;
                        size -= 4;
                    }break;
                    case '&':
                    {
                        if (size <= 5) return 0;
                        _str_copy(dst, "&amp;");
                        dst += 5;
                        size -= 5;
                    }break;
                    case '\'':
                    {
                        if (size <= 6) return 0;
                        _str_copy(dst, "&apos;");
                        dst += 6;
                        size -= 6;
                    }break;
                    case '\"':
                    {
                        if (size <= 6) return 0;
                        _str_copy(dst, "&quot;");
                        dst += 6;
                        size -= 6;
                    }break;
                    default:
                    {
                        *dst++ = *src;
                    }break;
                } 
                ++src;
            }
            *dst = 0;

            return s32(dst - org);
        }

        s32 xml_decode_str(char* dst, const char* src) { return _xml_decode_str(dst, src); }
        s32 xml_decode_str(wchar* dst, const wchar* src) { return _xml_decode_str(dst, src); }

        s32 xml_encode_str(char* dst, s32 size, const char* src) { return _xml_encode_str(dst, size, src); }
        s32 xml_encode_str(wchar* dst, s32 size, const wchar* src) { return _xml_encode_str(dst, size, src); }
	}
}