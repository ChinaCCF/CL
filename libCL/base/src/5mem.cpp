#include "../5mem.h"

namespace cl
{
	namespace mem
	{
		//uv32 mx = max_u8;
		//for (uv32 i = 0; i <= mx; i++)
		//{
		//	if (i % 16 == 0) printf("\n");
		//	if (i >= '0' && i <= '9')
		//		printf("0x%02x,", i - '0');
		//	else
		//	{
		//		if (i >= 'A' && i <= 'F')
		//			printf("0x%02x,", i - 'A' + 10);
		//		else
		//		{
		//			if (i >= 'a' && i <= 'f')
		//				printf("0x%02x,", i - 'a' + 10);
		//			else
		//				printf("0xCC,");
		//		}
		//	}
		//}
		const uv8 g_unhex_val[256] =
		{
			0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,
			0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,
			0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,
			0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,
			0xCC,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,
			0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,
			0xCC,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,
			0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,
			0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,
			0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,
			0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,
			0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,
			0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,
			0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,
			0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,
			0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,
		};

		template<CharType T>
		inline uv32 _unhex(void* data, uv32 size, const T* src, uv32 len)
		{
			[[unlikely]]
			if (size < (len >> 1)) return 0;

			auto dst = (uv8*)data;

			//skip 0x
			[[likely]]
			if (src[0] == '0')
			{
				if (src[1] == 'x' || src[1] == 'X')
					src += 2;
				else
					return 0;
			}
			auto org = src;

			uv8 vh, vl;
			while (src[0] != 0 && src[1] != 0)
			{
				vh = g_unhex_val[src[0]];
				vl = g_unhex_val[src[1]];

				if (vh == 0xCC || vl == 0xCC) break;

				*dst++ = vh << 4 | vl;
				src += 2;
			};

			if (src == org) return 0;

			*dst = 0;
			return uv32(dst - (uv8*)data);
		}

		uv32 unhex(_out void* data, uv32 size, const tc8* str, uv32 len)
		{
			return _unhex(data, size, str, len);
		}
		uv32 unhex(_out void* data, uv32 size, const tc16* str, uv32 len)
		{
			return _unhex(data, size, str, len);
		}
	}
}