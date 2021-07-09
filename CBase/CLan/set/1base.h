#ifndef __clan_set_base__
#define __clan_set_base__

#include "../base/4concept.h"
#include "../base/8str.h"

namespace cl
{  
	template<typename T, typename R>
	struct CmpVal 
	{
		int operator()(const T& k1, const R& k2)
		{
			if (k1 < k2) return -1;
			if (k1 == k2) return 0;
			return 1;
		}
	};

	template<ToCharsType T1, ToCharsType T2>
	struct CmpVal<T1, T2>
	{
		int operator()(const T1& k1, const T2& k2)
		{
			using ct1 = SelectCharType<T1>::type;
			using ct2 = SelectCharType<T2>::type;
			return CStr::cmp((const ct1*)k1, (const ct2*)k2);
		}
	};
	 
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

		bool operator==(const Pair& p) { return p.first == first && p.second == second; }
		bool operator!=(const Pair& p) { return !operator==(p); }

		static Pair make(const Key& key, const Val& val)
		{
			Pair pair;
			pair.first = key;
			pair.second = val;
			return pair;
		}
	};
}

#endif//__clan_set_base__