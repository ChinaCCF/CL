#ifndef __clan_mem__
#define __clan_mem__

#include "5val.h"

namespace clan
{
	class Byte
	{
	public:
		template<IntType T>
		static inline void copy(void* _dst, const void* _src, T size)
		{ 
			//auto dst = (u8*)_dst;
			//auto src = (u8*)_src;
			//for (T i = 0; i < size; i++)
			//	dst[i] = src[i];

            memcpy(_dst, _src, (size_t)size);
		}
		template<IntType T>
		static inline void mov(void* _dst, const void* _src, T size)
		{
			//if (_dst < _src)
			//	copy(_dst, _src, size);
			//else
			//{
			//	auto dst = (u8*)_dst;
			//	auto src = (u8*)_src;
			//	for (T i = size - 1; i >= 0; i--)
			//		dst[i] = src[i];
			//}

            memmove(_dst, _src, (size_t)size);
		}
		template<IntType T>
		static inline void zero(void* _dst, T size)
		{
			auto dst = (u8*)_dst;
			for (T i = 0; i < size; i++)
				dst[i] = 0;
		}
	};

    class Endian
    {
        template<size_t Size>
        static inline void _size_swap(void* p) { static_assert(false); }
        template<>
        static inline void _size_swap<2>(void* _p)
        {
            u8* p = (u8*)_p;
            swap(p[0], p[1]);
        }
        template<>
        static inline void _size_swap<4>(void* _p)
        {
            u8* p = (u8*)_p;
            swap(p[0], p[3]);
            swap(p[1], p[2]);
        }
        template<>
        static inline void _size_swap<8>(void* _p)
        {
            u8* p = (u8*)_p;
            swap(p[0], p[7]);
            swap(p[1], p[6]);
            swap(p[2], p[5]);
            swap(p[3], p[4]);
        }
    public:
        template<IntType T>
        static inline T change(T val)
        {
            static_assert(sizeof(T) >= 2);
            _size_swap<sizeof(T)>(&val);
            return val;
        }

        //判断当前运行环境是小端还是大端
        bool is_little()
        {
            u16 val = 0x1234;
            auto p = (u8*)&val;
            if (p[0] == 0x34) return true;
            return false;
        }

        bool is_big() { return !is_little(); }
    };
     
    //返回字符串长度, 失败返回0
    s32 hex_mem(char* buf, s32 buf_size, void* data, s32 data_size);
    s32 hex_mem(wchar* buf, s32 buf_size, void* data, s32 data_size);

    //返回数据大小, 字符串可能以0x开始也可能不是, 失败返回0
	s32 unhex_mem(void* data, s32 data_size, const char* str, s32 str_len);
	s32 unhex_mem(void* data, s32 data_size, const wchar* str, s32 str_len);
}

#endif//__clan_mem__ 