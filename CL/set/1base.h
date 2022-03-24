#ifndef __cl_set_base__
#define __cl_set_base__

#include "../base/3type_traits.h"
#include "../base/9str.h"

namespace cl
{  
	namespace lib
	{
		template<typename T1, typename T2>
		struct Compare
		{
			int operator()(const T1& k1, const T2& k2)
			{
				if (k1 < k2) return -1;
				if (k1 == k2) return 0;
				return 1;
			}
		};

		template<ToCharPtrType T1, ToCharPtrType T2>
		struct Compare<T1, T2>
		{
			int operator()(const T1& k1, const T2& k2)
			{
				return cstr::cmp(k1, k2);
			}
		};
	}

	template<typename Key, typename Val>
	struct Pair
	{
		Key first;
		Val second;
		Pair() : first(Key()), second(Val()) {}
		Pair(Pair&& p) noexcept : first(std::move(p.first)), second(std::move(p.second)) {}
		Pair(const Pair& p) : first(p.first), second(p.second) {}
		Pair(const Key& key, const Val& val) : first(key), second(val) {}

		Pair& operator=(const Pair& p)
		{
			first = p.first;
			second = p.second;
			return *this;
		}
		Pair& operator=(Pair&& p) noexcept
		{
			first = std::move(p.first);
			second = std::move(p.second);
			return *this;
		} 

		static Pair make(const Key& key, const Val& val)
		{
			Pair pair;
			pair.first = key;
			pair.second = val;
			return pair;
		}
	};
}

#endif//__cl_set_base__