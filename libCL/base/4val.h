#ifndef __cl_base_val__
#define __cl_base_val__

#include "3type_traits.h"

//if x < y, then -(x < y) will be all ones, so r = y ^ (x ^ y) & ~0 = y ^ x ^ y = x. 
//Otherwise, if x >= y, then -(x < y) will be all zeros, so r = y ^ ((x ^ y) & 0) = y.
template<typename T> requires cl::IsInt_v<T> || cl::IsChar_v<T>
cl_si T cl_max(const T & x, const T & y) { return x ^ ((x ^ y) & -(x < y)); }

template<typename T> requires cl::IsInt_v<T> || cl::IsChar_v<T>
cl_si T cl_min(const T & x, const T & y) { return y ^ ((x ^ y) & -(x < y)); }

template<cl::FloatType T> cl_si T cl_max(const T& x, const T& y) { return x < y ? y : x; }
template<cl::FloatType T> cl_si T cl_min(const T& x, const T& y) { return x > y ? y : x; }

namespace cl
{
	template<typename T> requires cl::IsInt_v<T> || cl::IsChar_v<T>
	cl_si T abs(const T & val) { T mask = val >> (sizeof(T) * 8 - 1); return (val + mask) ^ mask; }

	cl_si fv32 abs(fv32 v) { auto tv = *(uv32*)&v; tv &= 0x7FFFFFFF; return *(fv32*)&tv; }
	cl_si fv64 abs(fv64 v) { auto tv = *(uv64*)&v; tv &= 0x7FFFFFFFFFFFFFFF; return *(fv64*)&tv; }

	cl_si sv64 mul(sv32 x, sv32 y) { return sv64(x) * sv64(y); }
	cl_si fv64 mul(fv32 x, fv32 y) { return fv64(x) * fv64(y); }

	template<typename T> 
	cl_si void swap(T& a, T& b)
	{ 
		T tmp = std::move(a); a = std::move(b); b = std::move(tmp);
	}
	template<typename T> 
	cl_si void swap(T* p, uv32 i, uv32 j) 
	{
		T tmp = std::move(p[i]); p[i] = std::move(p[j]); p[j] = std::move(tmp);
	}

	/*############################################################################################*/
	//memory size
	/*############################################################################################*/
	//KB
	consteval uv64 operator "" _KB(uv64 val) { return val * 1024; }
	//MB
	consteval uv64 operator "" _MB(uv64 val) { return val * 1024 * 1024; }
	//GB
	consteval uv64 operator "" _GB(uv64 val) { return val * 1024 * 1024 * 1024; }
	//TB
	consteval uv64 operator "" _TB(uv64 val) { return val * 1024 * 1024 * 1024 * 1024; }
	/*############################################################################################*/
	//计时字面量
	/*############################################################################################*/
	//纳秒
	consteval uv64 operator "" _ns(uv64 val) { return val; }
	//微秒
	consteval uv64 operator "" _us(uv64 val) { return val * 1000; }
	//毫秒
	consteval uv64 operator "" _ms(uv64 val) { return val * 1000 * 1000; }
	//秒
	consteval uv64 operator "" _s(uv64 val) { return val * 1000 * 1000 * 1000; }

	/*############################################################################################*/
	//显示时间字面量
	/*############################################################################################*/
	//秒
	consteval uv64 operator "" _sec(uv64 val) { return val; }
	//分
	consteval uv64 operator "" _min(uv64 val) { return val * 60; }
	//时
	consteval uv64 operator "" _hour(uv64 val) { return val * 60 * 60; }
	//day
	consteval uv64 operator "" _day(uv64 val) { return val * 60 * 60 * 24; }
	//month
	consteval uv64 operator "" _month(uv64 val) { return val * 60 * 60 * 24 * 30; }
	//year
	consteval uv64 operator "" _year(uv64 val) { return val * 60 * 60 * 24 * 365; }
	/*############################################################################################*/
	//本对象用来处理enum class对象, 因为C++11之后, enum class 对象不支持 | & 等运算了 
	/*############################################################################################*/
	template<typename T>
	struct EnumType
	{
		uv32 val_;
#if CL_Version == CL_Version_Debug
		T* eval_;//方便调试
		EnumType() { eval_ = (T*)&val_; }
#else
		EnumType() {}
#endif 
		EnumType(const T& t) : EnumType() { val_ = (uv32)t; }
		EnumType(const EnumType& e) : EnumType() { val_ = e.val_; }

		operator uv32() { return (uv32)val_; }
		operator T() { return val_; }
		operator bool() { return (uv32)val_ != 0; }

	private:
		template<IntType R>
		uv32 _get_u32(const R& val) const { return (uv32)val; }
		uv32 _get_u32(const T& val) const { return (uv32)val; }
		uv32 _get_u32(const EnumType<T>& val) const { return val.val_; }
	public:
		template<typename E> inline EnumType operator=(E v) { val_ = _get_u32(v); return *this; }

		template<typename E> inline void operator+=(E v) { val_ += _get_u32(v); }
		template<typename E> inline void operator-=(E v) { val_ -= _get_u32(v); }
		/*############################################################################################*/
		template<typename E> inline bool operator== (E v) const { return val_ == _get_u32(v); }
		template<typename E> inline bool operator!= (E v) const { return val_ != _get_u32(v); }
		/*############################################################################################*/
		template<typename E> inline EnumType operator& (E v) const { return val_ & _get_u32(v); }
		template<typename E> inline EnumType operator| (E v) const { return val_ | _get_u32(v); }
		template<typename E> inline EnumType operator^ (E v) const { return val_ ^ _get_u32(v); }
		/*############################################################################################*/
		template<typename E> inline void operator&= (E v) { val_ &= _get_u32(v); }
		template<typename E> inline void operator|= (E v) { val_ |= _get_u32(v); }
		template<typename E> inline void operator^= (E v) { val_ ^= _get_u32(v); }
	};

	//2的指数都是最高为1,后面都是0,那么减一就变成 011...
	//例如 4  为  100,   4-1  为 011
	//     8  为  1000,  8-1  为 0111
	//     16 为  10000, 16-1 为 01111
	//     即 4 = 4-1 + 1, 8 = 8-1 + 1, ....
	//只要把某个数的最高位后面的所有位都置1,再加上1就是2指数
	//向上对齐2的指数, 不支持负数
	cl_si uv8  align_power2(uv8 v) { v--; v |= v >> 1; v |= v >> 2; v |= v >> 4; v++; return v; }
	cl_si uv16 align_power2(uv16 v) { v--; v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v++; return v; }
	cl_si uv32 align_power2(uv32 v) { v--; v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16; v++; return v; }
	cl_si uv64 align_power2(uv64 v) { v--; v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16; v |= v >> 32; v++; return v; }

	cl_si sv8  align_power2(sv8 v) { return (sv8)align_power2((uv8)v); }
	cl_si sv16 align_power2(sv16 v) { return (sv16)align_power2((uv16)v); }
	cl_si sv32 align_power2(sv32 v) { return (sv32)align_power2((uv32)v); }
	cl_si sv64 align_power2(sv64 v) { return (sv64)align_power2((uv64)v); }

	template<typename T, typename T2> requires (cl::IsInt_v<T> || cl::IsChar_v<T>) && (cl::IsInt_v<T2> || cl::IsChar_v<T2>)
	cl_si T align(T val, T2 align) { T n = align - 1; return (val + n) & ~n; }
	//求2的几多次方才能大于或等于某个数, 例如log2(8) = 3
	template<typename T> requires cl::IsInt_v<T> || cl::IsChar_v<T>
	cl_si sv32 log2(T val) { sv32 index = 0; while(val >>= 1) index++; return index; }

	/*############################################################################################*/
	//hash
	/*############################################################################################*/
	namespace lib
	{//todo 后面要修改为 Google 的 cityhash 
		template<FloatType T> 
		cl_si uv32 _hash(T val) { auto f = (uv32*)&val; return f[0]; };

		template<typename T> requires cl::IsInt_v<T> || cl::IsChar_v<T>
		cl_si uv32 _hash(T val) { return (uv32)(uvt)val; };

		template<PtrType T> cl_si uv32 _hash(T val) { return (uv32)(uvt)val; };

		template<typename T> requires ToC16Ptr_v<T> || ToC8Ptr_v<T>
		cl_si uv32 _hash(const T & val) //BKDRHash
		{
			using Type = SelectType_t<ToC8Ptr_v<T>, uc8, uc16>;
			const Type* str = (const Type*)val;
			uv32 seed = 1313;
			uv32 ret = 0;
			while (*str) ret = ret * seed + (*str++);
			return ret & 0x7FFFFFFF;
		};
	}

	template<typename T> 
	cl_si uv32 hash(const T& val) { return lib::_hash(val); }

	/*############################################################################################*/
	//random
	/*############################################################################################*/ 
	class Random
	{
	public:
		//每个线程都要自己调用一次
		static void init(uv32 init_v);
		static uv32 ma_x(uv32 m);
		static uv32 between(uv32 mi_n, uv32 ma_x);
	}; 
}

#endif//__cl_base_val__ 