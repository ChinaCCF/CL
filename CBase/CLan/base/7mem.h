#ifndef __clan_base_mem__
#define __clan_base_mem__

#include "6val.h"

namespace cl
{ 
    template<AllocMemType A, typename T, typename ... Args>
    inline T* _new(Args&& ... args)
    {
        auto p = (T*)A().alloc(sizeof(T));
        ::new(p)T(std::forward<Args>(args)...);
        return p;
    } 
    template<AllocMemType A, typename T>
    inline void _del(T* p)
    {
        p->~T();
        A().free(p);
    }

    template<AllocMemType A, typename T>
    inline T* _news(s32 N)
    {
        auto p = (T*)A().alloc(sizeof(T) * N);
        for (s32 i = 0; i < N; ++i) ::new(p + i)T();
        return p;
    }

    template<AllocMemType A, typename T>
    inline void _dels(T* p, s32 N) 
    {
        for (s32 i = 0; i < N; ++i) 
            (p + i)->~T();
        A().free(p); 
    }

	class Byte
	{
	public:
		template<IntType T>
		static inline void copy(void* _dst, const void* _src, T size)
		{ 
            memcpy(_dst, _src, (size_t)size);
		}
		template<IntType T>
		static inline void mov(void* _dst, const void* _src, T size)
		{
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

    //内存 大小端 工具类
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
        //改变当前值为另一端, 大端变小端 或 小端变大端
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
     
    //返回字符的16进制值, 失败返回0xCC
    u8 hex_char_val(char c);
    //返回字符的16进制值, 失败返回0xCC
    u8 hex_char_val(wchar c);

    //返回字符串长度, 失败返回0
    // buf_len : 不是内存大小, 而是能够存储多少个对应类型的字符
    // add_pre : add prefix, 表示是否添加前缀 0x
    s32 hex_mem(char* buf, s32 buf_len, const void* data, s32 data_size, bool add_pre = true);
    s32 hex_mem(wchar* buf, s32 buf_len, const void* data, s32 data_size, bool add_pre = true);

    //返回数据大小, 字符串可能以0x开始也可能不是, 失败返回0
	s32 unhex_mem(void* data, s32 data_size, const char* str, s32 str_len);
	s32 unhex_mem(void* data, s32 data_size, const wchar* str, s32 str_len);
}

#endif//__clan_base_mem__ 