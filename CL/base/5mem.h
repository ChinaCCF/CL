#ifndef __cl_base_mem__
#define __cl_base_mem__

#include <stdlib.h>
#include "4val.h"

namespace cl
{
	/*#####################################################################################*/
	//一些基础的内存函数
	/*#####################################################################################*/
	namespace mem
	{
		static inline void zero(_in void* _p, _in uv32 size)
		{
			auto p = (uv8*)_p;
			while (size--)  *p++ = 0;
		}

		static inline void copy(_out void* _dst, _in const void* _src, _in uv32 size)
		{
			auto dst = (uv8*)_dst; auto src = (uv8*)_src;
			for (uv32 i = 0; i < size; i++) dst[i] = src[i]; 
		}
		  
		static inline void rshift(_in_out void* _buf, uv32 buf_size, uv32 shift_size)
		{
			auto buf = (uv8*)_buf; buf += buf_size - 1;
			for (sv32 i = 0; i < (sv32)shift_size; i++)
				buf[shift_size - i] = buf[-i];
		}
	}

	/*#####################################################################################*/
	//默认内存申请对象
	/*#####################################################################################*/
	template<typename Allocator> 
	concept MemAllocType = requires(Allocator a, uvt size, void* p)
	{
		a.alloc(size);//具备alloc 指定大小函数  
		a.free(p);//释放任意大小指针空间 
			requires IsSame_v<decltype(a.alloc(1)), uv8*>;//确保申请函数返回uv8* 指针 
	};
	struct MemAllocator
	{
		uv8* alloc(_in uvt size) { return (uv8*)malloc(size); }
		void free(_in void* p) { ::free(p); }
	};

	/*#####################################################################################*/
	//默认对象申请对象
	/*#####################################################################################*/
	template<template<typename> class Allocator, typename Obj> 
	concept ObjAllocType = requires(Allocator<Obj> a, Obj * p)
	{
		a.alloc();
		a.free(p);
			requires IsSame_v<decltype(a.alloc()), Obj*>;
	};

	template<typename T>
	struct ObjAllocator
	{
		T* alloc() { return new T(); }
		void free(_in T* p) { delete p; }
	};

	/*#####################################################################################*/
	//hex and unhex
	/*#####################################################################################*/
	//返回字符串长度, 失败返回0 
	template<CharType T>
	static inline uv32 hex_mem(_out T* buf, _in uv32 size, _in const void* data, _in uv32 len)
	{
		if (size < (len << 1)) return 0;

		auto src = (const uv8*)data;
		auto dst = buf;

		static const uc8* arr = "0123456789ABCDEF";
		while (len--)
		{
			uv8 val = *src++;
			*dst++ = arr[(val & 0xF0) >> 4];
			*dst++ = arr[val & 0xF];
		}
		*dst = 0;
		return len << 1;
	}

	//成功返回数据大小, 否则返回0
	uv32 unhex_mem(_out void* data, _in uv32 size, _in const uc8* str, _in uv32 len);
	uv32 unhex_mem(_out void* data, _in uv32 size, _in const uc16* str, _in uv32 len);

	/*#####################################################################################*/
	//内存 大小端 工具类
	/*#####################################################################################*/
	class Endian
	{
		template<uvt Size> static inline void _size_swap(void* p) { static_assert(Size > 1, "1 or 0 not need swap!"); }

		template<> static inline void _size_swap<2>(void* _p)
		{
			uv8* p = (uv8*)_p;
			swap(p[0], p[1]);
		}
		template<> static inline void _size_swap<4>(void* _p)
		{
			uv8* p = (uv8*)_p;
			swap(p[0], p[3]);
			swap(p[1], p[2]);
		}
		template<> static inline void _size_swap<8>(void* _p)
		{
			uv8* p = (uv8*)_p;
			swap(p[0], p[7]);
			swap(p[1], p[6]);
			swap(p[2], p[5]);
			swap(p[3], p[4]);
		}
	public:
		//改变当前值为另一端, 大端变小端 或 小端变大端
		template<typename T> requires cl::IsInt_v<T> || cl::IsChar_v<T>
		static inline T change(T val)
		{
			static_assert(sizeof(T) >= 2, "can't change one byte endian!");
			_size_swap<sizeof(T)>(&val);
			return val;
		}

		//判断当前运行环境是小端还是大端
		bool is_little()
		{
			uv16 val = 0x1234;
			auto p = (uv8*)&val;
			if (p[0] == 0x34) return true;
			return false;
		}

		bool is_big() { return !is_little(); }
	};
	/*#####################################################################################*/
	//MemBuf
	/*#####################################################################################*/
	template<typename T, MemAllocType MA = MemAllocator>
	class MemBuf
	{
	public:
		uv32 size_ = 0;//可以存储 size 个 T 元素
		uv32 len_ = 0;//当前存储了 len 个 T 元素
		T* buf_ = nullptr;

		~MemBuf() { clear(); }

		void clear() { if (buf_) MA().free(buf_); }

		bool need(_in uv32 len)
		{
			len += len_;
			if (len < size_) return true; 

			auto byte_size = align_power2(len * sizeof(T));
			size_ = byte_size / sizeof(T);

			auto p = (T*)MA().alloc(byte_size);
			if (p == nullptr) return false;
			 
			if (buf_)
			{
				mem::copy(p, buf_, len_ * sizeof(T));
				MA().free(buf_);
			}

			buf_ = p;
			return true;
		}
	};
}

#endif//__cl_base_mem__ 