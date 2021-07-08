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
					if (CStrX::cmp(src, "&lt;", 4) == 0)
					{
						*dst++ = '<';
						src += 4;
						continue;
					}

					if (CStrX::cmp(src, "&gt;", 4) == 0)
					{
						*dst++ = '>';
						src += 4;
						continue;
					}

					if (CStrX::cmp(src, "&amp;", 5) == 0)
					{
						*dst++ = '&';
						src += 5;
						continue;
					}

					if (CStrX::cmp(src, "&apos;", 6) == 0)
					{
						*dst++ = '\'';//µ¥ÒýºÅ
						src += 6;
						continue;
					}

					if (CStrX::cmp(src, "&quot;", 6) == 0)
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
                        CStrX::copy(dst, "&lt;", 4);
                        dst += 4;
                        size -= 4;
                    }break;
                    case '>':
                    {
                        if (size <= 4) return 0;
                        CStrX::copy(dst, "&gt;", 4);
                        dst += 4;
                        size -= 4;
                    }break;
                    case '&':
                    {
                        if (size <= 5) return 0;
                        CStrX::copy(dst, "&amp;", 5);
                        dst += 5;
                        size -= 5;
                    }break;
                    case '\'':
                    {
                        if (size <= 6) return 0;
                        CStrX::copy(dst, "&apos;", 6);
                        dst += 6;
                        size -= 6;
                    }break;
                    case '\"':
                    {
                        if (size <= 6) return 0;
                        CStrX::copy(dst, "&quot;", 6);
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