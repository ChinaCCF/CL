#ifndef __clan_set_base__
#define __clan_set_base__

#include "../base/4concept.h"
#include "../base/8str.h"

namespace clan
{
	template<typename T>
	struct CmpVal
	{
		int operator()(const T& k1, const T& k2)
		{
			if (k1 < k2) return -1;
			if (k1 == k2) return 0;
			return 1;
		}
	};

	template<StrType T>
	struct CmpVal<T>
	{
		int operator()(const T& k1, const T& k2)
		{
			return Str::cmp((const char*)k1, (const char*)k2);
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