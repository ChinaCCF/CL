#ifndef __clan_base_val__
#define __clan_base_val__

#include "4concept.h"

//if x < y, then -(x < y) will be all ones, so r = y ^ (x ^ y) & ~0 = y ^ x ^ y = x. 
//Otherwise, if x >= y, then -(x < y) will be all zeros, so r = y ^ ((x ^ y) & 0) = y.
template<clan::IntType T> inline T cl_max(const T& x, const T& y) { return x ^ ((x ^ y) & -(x < y)); }
template<clan::IntType T> inline T cl_min(const T& x, const T& y) { return y ^ ((x ^ y) & -(x < y)); }

template<clan::FloatType T> inline T cl_max(const T& x, const T& y) { return x < y ? y : x; }
template<clan::FloatType T> inline T cl_min(const T& x, const T& y) { return x > y ? y : x; }

namespace clan
{
	template<IntType T> inline T abs(T val) { T mask = val >> (sizeof(T) * 8 - 1); return (val + mask) ^ mask; }
	inline f32 abs(f32 v) { auto tv = *(u32*)&v; tv &= 0x7FFFFFFF; return *(f32*)&tv; }
	inline f64 abs(f64 v) { auto tv = *(u64*)&v; tv &= 0x7FFFFFFFFFFFFFFF; return *(f64*)&tv; }

	template<MoveType T> inline void swap(T& a, T& b) { T tmp = std::move(a); a = std::move(b); b = std::move(tmp); }

	/*############################################################################################*/
	//本对象用来处理enum class对象, 因为C++11之后, enum class 对象不支持 | &等运算了 
	/*############################################################################################*/
	template <typename T>
	struct EnumType;

	namespace detail
	{
		template<typename T, typename R>
		concept _EnumVal = IsInt<R>::value || IsSameType<T, R>::value;

		template<typename T, typename R>
		concept _EnumType = IsInt<R>::value || IsSameType<T, R>::value || IsSameType<R, EnumType<T>>::value;
	}

	template<typename T>
	struct EnumType
	{
		T val_ = T(0);

		EnumType() {}
		template<typename R> requires detail::_EnumVal<T, R> EnumType(R v) { val_ = (T)v; }

		//不能定义这个操作, 否则  
		// EnumType e; bool val = e == 1.0; 会编译成功!
		//operator u32() { return (u32)val_; }
		operator T() { return val_; }
		operator bool() { return (u32)val_ != 0; }

		template<typename R> requires detail::_EnumVal<T, R> inline EnumType operator=(R v) { return val_ = (T)v; }

		template<IntType R> inline void operator+=(R v) { val_ = T((u32)val_ + (u32)v); }
		template<IntType R> inline void operator-=(R v) { val_ = T((u32)val_ - (u32)v); }

		template<typename R> requires detail::_EnumType<T, R> inline bool operator== (R v) const { return (u32)val_ == (u32)EnumType(v).val_; }
		template<typename R> requires detail::_EnumType<T, R> inline bool operator!= (R v) const { return (u32)val_ != (u32)EnumType(v).val_; }
		/******************************************************************************************************************************************/
		template<typename R> requires detail::_EnumType<T, R> inline EnumType operator& (R v) const { return T((u32)val_ & (u32)EnumType(v).val_); }
		template<typename R> requires detail::_EnumType<T, R> inline EnumType operator| (R v) const { return T((u32)val_ | (u32)EnumType(v).val_); }
		template<typename R> requires detail::_EnumType<T, R> inline EnumType operator^ (R v) const { return T((u32)val_ ^ (u32)EnumType(v).val_); }
		/******************************************************************************************************************************************/
		template<typename R> requires detail::_EnumType<T, R> inline void operator&= (R v) { val_ = T((u32)val_ & (u32)EnumType(v).val_); }
		template<typename R> requires detail::_EnumType<T, R> inline void operator|= (R v) { val_ = T((u32)val_ | (u32)EnumType(v).val_); }
		template<typename R> requires detail::_EnumType<T, R> inline void operator^= (R v) { val_ = T((u32)val_ ^ (u32)EnumType(v).val_); }
		/******************************************************************************************************************************************/
		template<typename R> requires detail::_EnumType<T, R> inline bool operator> (R v) const { return (u32)val_ > (u32)EnumType(v).val_; }
		template<typename R> requires detail::_EnumType<T, R> inline bool operator>= (R v) const { return (u32)val_ >= (u32)EnumType(v).val_; }
		template<typename R> requires detail::_EnumType<T, R> inline bool operator< (R v) const { return (u32)val_ < (u32)EnumType(v).val_; }
		template<typename R> requires detail::_EnumType<T, R> inline bool operator<= (R v) const { return (u32)val_ <= (u32)EnumType(v).val_; }
	};

	template<IntType T, IntType R> inline T align(T val, R align) { T n = align - 1; return (val + n) & ~n; }

	//2的指数都是最高为1,后面都是0,那么减一就变成 011...
	//例如 4  为  100,   4-1  为 011
	//     8  为  1000,  8-1  为 0111
	//     16 为  10000, 16-1 为 01111
	//     即 4 = 4-1 + 1, 8 = 8-1 + 1, ....
	//只要把某个数的最高位后面的所有位都置1,再加上1就是2指数
	//向上对齐2的指数, 不支持负数
	inline u8  align_power2(u8 v) { v--; v |= v >> 1; v |= v >> 2; v |= v >> 4; v++; return v; }
	inline u16 align_power2(u16 v) { v--; v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v++; return v; }
	inline u32 align_power2(u32 v) { v--; v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16; v++; return v; }
	inline u64 align_power2(u64 v) { v--; v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16; v |= v >> 32; v++; return v; }

	inline s8  align_power2(s8 v) { return (s8)align_power2((u8)v); }
	inline s16 align_power2(s16 v) { return (s16)align_power2((u16)v); }
	inline s32 align_power2(s32 v) { return (s32)align_power2((u32)v); }
	inline s64 align_power2(s64 v) { return (s64)align_power2((u64)v); }

	//求2的几多次方才能大于或等于某个数, 例如log2(8) = 3
	template<IntType T>
	inline s32 log2(T val) { s32 index = 0; while (val >>= 1) index++; return index; }

	namespace detail
	{
		template<FloatType T>
		u32 _hash(T val)
		{
			auto f = (u32*)&val;
			return f[0];
		};

		template<IntType T>
		u32 _hash(T val)
		{ 
			return (u32)(u64)val;
		};

		template<PtrType T>
		u32 _hash(T val)
		{
			return (u32)(u64)val;
		};
		 
		template<StrType T>
		u32 _hash(const T& val)
		{
			using Type = SelectType<IsStrC8<T>::value, char, wchar>::type;
			Type* str = (const Type*)val;
			u32 seed = 1313;
			u32 ret = 0;
			while (*str) ret = ret * seed + (*str++);
			return ret & 0x7FFFFFFF;
		};
	}

	template<typename T>
	u32 hash(const T& val) { return detail::_hash(val); }
}

#endif//__clan_base_val__ 