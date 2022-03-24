#ifndef __cl_base_mem__
#define __cl_base_mem__

#include <stdlib.h>
#include "4val.h"

namespace cl
{
	//所有内存申请操作, 若失败, 抛出异常
	/*#####################################################################################*/
	//一些基础的内存函数
	/*#####################################################################################*/
	namespace mem
	{
		static inline void zero(void* _p, uv32 size)
		{
			auto p = (uv8*)_p;
			while (size--)  *p++ = 0;
		}

		static inline void copy(_out void* _dst, const void* _src, uv32 size)
		{
			auto dst = (uv8*)_dst; auto src = (uv8*)_src;
			for (uv32 i = 0; i < size; i++) dst[i] = src[i]; 
		}
		  
		static inline void rshift(_out void* _buf, uv32 buf_size, uv32 shift_size)
		{
			auto buf = (uv8*)_buf; buf += buf_size - 1;
			for (sv32 i = 0; i < (sv32)shift_size; i++)
				buf[shift_size - i] = buf[-i];
		}

		/*#####################################################################################*/
		//hex and unhex
		/*#####################################################################################*/
		//返回字符串长度, 失败返回0 
		template<CharType T>
		static inline uv32 hex(_out T* buf, uv32 size, const void* data, uv32 len)
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
		uv32 unhex(_out void* data, uv32 size, const uc8* str, uv32 len);
		uv32 unhex(_out void* data, uv32 size, const uc16* str, uv32 len);
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
		uv8* alloc(uvt size)
		{
			auto p = (uv8*)malloc(size);
			if (p == nullptr)
				CL_Throw(ExceptionCode::Memory_Alloc_Fail, 0, "");
			return p;
		}
		void free(void* p) { ::free(p); }
	};

	template<class MA, typename T, typename ... Args>
	T* alloc_obj(Args&& ... args)
	{
		auto p = (T*)MA().alloc(sizeof(T));
		new(p)T(std::forward<Args>(args)...);
		return p;
	}
	template<class MA, typename T>
	void free_obj(T*& p)
	{
		if (p == nullptr) return;
		p->~T();
		MA().free(p);
		p = nullptr;
	}

	template<class MA, typename T>
	T* alloc_objs(uv32 n)
	{
		auto p = (T*)MA().alloc(sizeof(T) * n);
		for (uv32 i = 0; i < n; i++)
			new(p + i)T();
		return p;
	}
	template<class MA, typename T>
	void free_objs(T*& p, uv32 n)
	{
		if (p == nullptr) return; 
		for (uv32 i = 0; i < n; i++)
			(p + i)->~T();
		MA().free(p);
		p = nullptr;
	}
	/*#####################################################################################*/
	//默认对象申请对象
	/*#####################################################################################*/
	/*template<template<typename> class Allocator, typename Obj> 
	concept ObjAllocType = requires(Allocator<Obj> a, Obj* p)
	{
		a.alloc();
		a.free(p);
		a.alloc_n(2);
		a.free_n(p, 3);
			requires IsSame_v<decltype(a.alloc()), Obj*>;
			requires IsSame_v<decltype(a.alloc_n(2)), Obj*>;
	};

	template<typename T>
	struct ObjAllocator
	{
		T* alloc() { return new T(); }
		void free( T* p) { delete p; }

		T* alloc_n(uv32 cnt) 
		{
			auto p = (T*)malloc(sizeof(T) * cnt);
			for (uv32 i = 0; i < cnt; i++)
				new(p + i)T();
			return p;
		}

		void free_n( T* p, uv32 cnt)
		{
			for (uv32 i = 0; i < cnt; i++)
				(p + i)->~T();
			::free(p);
		}
	};*/
	 
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
	//Buf
	/*#####################################################################################*/
	template<typename T, class MA = MemAllocator, uv32 Extra_N = 0> 
	class MemBuf
	{
		uv32 size_ = 0;//可以存储 size 个 T 元素 
		T* buf_ = nullptr;
	public: 
		MemBuf() {} 
		MemBuf(MemBuf&& buf) : size_(buf.size_), buf_(buf.buf_)
		{
			buf.buf_ = nullptr;
			buf.size_ = 0; 
		}
		 
		~MemBuf() { free(); }

		uv32 size() const { return size_; }
		T* data() const { return buf_; }
		 
		void free() { if (buf_) { MA().free(buf_); buf_ = nullptr; } }
		  
		void alloc(uv32 size) 
		{
			buf_ = (T*)MA().alloc(sizeof(T) * (size + Extra_N));
			size_ = size; 
		}
		 
		void operator=(MemBuf&& buf)
		{
			free();
			size_ = buf.size_;
			buf_ = buf.buf_; 
			buf.size_ = 0;
			buf.buf_ = nullptr;
		}

		void copy(uv32 self_offset, const T* buf, uv32 offset, uv32 cnt)
		{
			auto dst = buf_ + self_offset;
			auto src = buf + offset;
			for (uv32 i = 0; i < cnt; i++)
				dst[i] = src[i];
		} 
	};
	 
}

#endif//__cl_base_mem__ 