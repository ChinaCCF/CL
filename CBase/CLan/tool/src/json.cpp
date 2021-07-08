#include "../json.h"

namespace cl
{
	namespace detail
	{
		template<CharType C>
		inline s32 _json_decode_str(const C* src, s32 len)
		{
			auto org = src;
			auto dst = (C*)src;
		label_redo:
			//跳过非转移符
			while (len && *src != '\\') { *dst++ = *src++; len--; };
			if (len == 0)
			{
				*dst = 0;
				return s32(dst - org);
			}

			if (src[1] == 'u')
			{
				unhex_mem(dst, 2, src + 2, 4);
				src += 6;
				len -= 6;
				dst += 2 / sizeof(C);
			}
			else
			{
				switch (src[1])
				{
					case '"': *dst = '"'; break;
					case '/': *dst = '/'; break;
					case 'b': *dst = '\b'; break;
					case 'f': *dst = '\f'; break;
					case 'n': *dst = '\n'; break;
					case 'r': *dst = '\r'; break;
					case 't': *dst = '\t'; break;
					case '\\': *dst = '\\'; break;
					default: cl_assert(false); break;
				}
				src += 2;
				len -= 2;
				dst += 1;
			}
			goto label_redo;
		}

		s32 json_decode_str(const char* src, s32 len)
		{
			return _json_decode_str(src, len);
		}
		s32 json_decode_str(const wchar* src, s32 len)
		{
			return _json_decode_str(src, len);
		}

		template<CharType C>
		inline void _json_encode_str(C* dst, s32 size, const C* _src, bool encode_unicode)
		{
			using utype = typename SameSizeType<C, u8, u16>::type;

			auto src = (const utype*)_src;
			auto p = (utype*)dst;
			s32 len = size;

			while (*src && len > 6)
			{
				if (*src > 127)
				{
					if (encode_unicode)
					{
						*p++ = '\\'; *p++ = 'u';
						hex_mem((C*)p, 5, src, 2, false);

						p += 4;
						src += 2 / sizeof(C);
						len -= 6;
					}
					else
					{//其他的编码最高位必定是1
						*p++ = *src++;
						len--;
					}
				}
				else
				{
					switch (*src)
					{
						case '"': *p++ = '\\'; *p++ = '"'; src++; len -= 2; break;
						case '/': *p++ = '\\'; *p++ = '/'; src++; len -= 2; break;
						case '\b': *p++ = '\\'; *p++ = 'b'; src++; len -= 2; break;
						case '\f': *p++ = '\\'; *p++ = 'f'; src++; len -= 2; break;
						case '\n': *p++ = '\\'; *p++ = 'n'; src++; len -= 2; break;
						case '\r': *p++ = '\\'; *p++ = 'r'; src++; len -= 2; break;
						case '\t': *p++ = '\\'; *p++ = 't'; src++; len -= 2; break;
						case '\\': *p++ = '\\'; *p++ = '\\'; src++; len -= 2; break;
						default:
						{
							*p++ = *src++;
							len--;
						}break;
					}
				}
			}

			*p = 0;
		}

		void json_encode_str(char* dst, s32 size, const char* _src, bool encode_unicode)
		{
			_json_encode_str(dst, size, _src, encode_unicode);
		}
		void json_encode_str(wchar* dst, s32 size, const wchar* _src, bool encode_unicode)
		{
			_json_encode_str(dst, size, _src, encode_unicode);
		}

		template<CharType C>
		C* _json_parse_other_val(const C* str, detail::JsonOtherVal& val)
		{
			C buf[6];
			CStr::copy(buf, 6, str, 6);
			buf[5] = 0;
			CStr::lower(buf);
			if (CStrX::cmp(buf, "false", 5) == 0)
			{
				val = detail::JsonOtherVal::False;
				return (C*)(str + 5);
			}
			if (CStrX::cmp(buf, "true", 4) == 0)
			{
				val = detail::JsonOtherVal::True;
				return (C*)(str + 4);
			}
			if (CStrX::cmp(buf, "null", 4) == 0)
			{
				val = detail::JsonOtherVal::Null;
				return (C*)(str + 4);
			}
			cl_assert(false);
			return nullptr;
		}

		char* json_parse_other_val(const char* str, detail::JsonOtherVal& val)
		{
			return _json_parse_other_val(str, val);
		}
		wchar* json_parse_other_val(const wchar* str, detail::JsonOtherVal& val)
		{
			return _json_parse_other_val(str, val);
		}
	}
	
}