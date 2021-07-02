#ifndef __clan_set_mix_obj__
#define __clan_set_mix_obj__

#include "../base/8str.h" 
#include "5map.h"

namespace cl
{
	namespace detail
	{
		enum class MixObjType
		{ 
			None,
			Null,
			Bool,
			Int,
			Float,
			Str,
			List,
			Map,
		};
	}

	template<CharType C, AllocMemType A>
	class _MixObj : public NoCopyObj
	{
		using ThisType = _MixObj<C, A>;
		using ValType = detail::MixObjType;
		using StrValType = _String<C, A, 0>;
		using ListValType = _List<ThisType, A>;
		using MapValType = _SerialHashMap<StrValType, ThisType, A>;

		using ListIt = typename _List<ThisType, A>::It;
		using MapIt = typename _SerialHashMap<StrValType, ThisType, A>::It;

	public:
		union Val
		{
			f64 fv_;
			s64 sv_;
			bool bv_;
			StrValType* str_;
			ListValType* list_;
			MapValType* map_;
		};
		mutable Val val_;
		mutable ValType val_type_ = ValType::Null;

	private:
		void _release()
		{
			if (val_type_ == ValType::Str)
				_del<A>(val_.str_);
			if (val_type_ == ValType::Map)
				_del<A>(val_.map_);
			if (val_type_ == ValType::List)
				_del<A>(val_.list_);

			val_type_ = ValType::Null;
		}
	public:
		_MixObj()
		{
			static_assert(sizeof(s64) >= sizeof(StrValType*) || sizeof(f64) >= sizeof(StrValType*), "s64 or f64 size is smaller than pointer!");
			val_.sv_ = 0;
		}
		_MixObj(_MixObj&& mo) noexcept
		{
			val_ = mo.val_;
			val_type_ = mo.val_type_;

			mo.val_.sv_ = 0;
			mo.val_type_ = ValType::Null;
		}

		template<typename T> _MixObj(T&& val) { this->operator=(std::forward<T>(val)); }
		~_MixObj() { _release(); }

		bool is_null() const { return val_type_ == ValType::Null; }
		bool is_bool() const { return val_type_ == ValType::Bool; }
		bool is_int() const { return val_type_ == ValType::Int; }
		bool is_float() const { return val_type_ == ValType::Float; }
		bool is_str() const { return val_type_ == ValType::Str; }
		bool is_list() const { return val_type_ == ValType::List; }
		bool is_map() const { return val_type_ == ValType::Map; }

		/*####################################################################################################*/
		//类型转换
		/*####################################################################################################*/
		template<BoolType T>
		operator T() const
		{
			[[likely]]
			if (val_type_ == ValType::Bool)
				return (T)val_.bv_;
			if (val_type_ == ValType::Str)
				return (T)*val_.str_;

			cl_assert(false);
			return T();
		};
		operator C() const
		{
			[[likely]]
			if (val_type_ == ValType::Int)
				return (C)val_.sv_;
			if (val_type_ == ValType::Str)
				return val_.str_->data()[0];

			cl_assert(false);
			return C();
		};
		template<IntType T>
		operator T() const
		{
			[[likely]]
			if (val_type_ == ValType::Int)
				return (T)val_.sv_;
			if (val_type_ == ValType::Str)
				return (T)*val_.str_;

			cl_assert(false);
			return T();
		};
		template<FloatType T>
		operator T() const
		{
			[[likely]]
			if (val_type_ == ValType::Float)
				return (T)val_.fv_;
			if (val_type_ == ValType::Str)
				return (T)*val_.str_;

			cl_assert(false);
			return T();
		};
		template<typename T> requires ToChars<T, C>::value
		operator T() const
		{
			[[likely]]
			if (val_type_ == ValType::Str)
				return (T)*val_.str_;

			cl_assert(false);
			return T();
		};
		/*####################################################################################################*/
		//赋值
		/*####################################################################################################*/
		void operator=(const std::nullptr_t&) { _release(); }
		void operator=(_MixObj&& mo) noexcept { _release(); new(this)_MixObj(std::move(mo)); }

		template<BoolType T>
		void operator=(T val)
		{
			_release();
			val_type_ = ValType::Bool;
			val_.sv_ = val;
		};
		void operator=(C val)
		{
			_release();
			val_type_ = ValType::Int;
			val_.sv_ = val;
		};
		template<IntType T>
		void operator=(T val)
		{
			_release();
			val_type_ = ValType::Int;
			val_.sv_ = val;
		};
		template<FloatType T>
		void operator=(T val)
		{
			_release();
			val_type_ = ValType::Float;
			val_.fv_ = val;
		};
		template<typename T> requires ToChars<T, C>::value
		void operator=(T&& val)
		{
			_release();
			val_type_ = ValType::Str;
			val_.str_ = _new<A, StrValType>(std::forward<T>(val));
		}
		/*####################################################################################################*/
		//列表
		/*####################################################################################################*/
		_MixObj& operator[](int index) const
		{
			cl_assert(val_type_ == ValType::List);
			return &val_.list_->operator[](index);
		} 
		void push(_MixObj&& obj)
		{
			if (val_type_ == ValType::Null)
			{
				val_type_ = ValType::List;
				val_.list_ = _new<A, ListValType>();
			}
			else
				cl_assert(val_type_ == ValType::List);

			val_.list_->push_back(std::move(obj));
		}
		template<typename T>
		void push(T&& val)
		{
			_MixObj obj = std::forward<T>(val);
			push(std::move(obj));
		}
		ListValType* list() const
		{
			cl_assert(val_type_ == ValType::List);
			return val_.list_;
		}
		/*####################################################################################################*/
		//映射
		/*####################################################################################################*/
		_MixObj& operator[](const C* key) const
		{
			if (val_type_ == ValType::Null)
			{
				val_type_ = ValType::Map;
				val_.map_ = _new<A, MapValType>();
			}
			else
				cl_assert(val_type_ == ValType::Map);

			auto map = val_.map_;
			return (*map)[key]; 
		} 
		template<typename T> requires ToChars<T, C>::value
		_MixObj& operator[](const T& key) const { return this->operator[]((const C*)key); } 

		MapValType* map() const
		{
			cl_assert(val_type_ == ValType::Map);
			return val_.map_;
		}
		bool find(const C* key)
		{
			cl_assert(val_type_ == ValType::Map);
			auto map = val_.map_;
			return map->find(key) != map->end();
		}

		/*####################################################################################################*/
		//It
		/*####################################################################################################*/ 
		class It
		{
			friend class ThisType;
			using PairType = Pair<StrValType*, ThisType*>;
	 
			u8 buf_[32];//用来保存各个类型的It值
			ValType pair_val_type_ = ValType::None;
			PairType pair_val_;
		public:
			It()
			{
				buf_[0] = 0;
				static_assert(sizeof(buf_) > sizeof(ListIt) || sizeof(buf_) > sizeof(MapIt), "It buf over!");
			}

			PairType& operator*() { return pair_val_; }
			PairType* operator->() { return &pair_val_; }
			It& operator++()
			{
				if (pair_val_type_ == ValType::List)
				{
					auto& it = *(ListIt*)buf_;
					++it;

					pair_val_.first = nullptr;
					pair_val_.second = &*it;
				}
				else
				{
					if (pair_val_type_ == ValType::Map)
					{
						auto& it = *(MapIt*)buf_;
						++it;

						pair_val_.first = &it->first;
						pair_val_.second = &it->second;
					} 
					else
					{
						pair_val_type_ = ValType::None; 
					}
				}
				return *this;
			}
			bool operator==(const It& it)
			{
				if (pair_val_type_ == ValType::List)
				{
					auto p1 = (ListIt*)buf_;
					auto p2 = (ListIt*)it.buf_;
					return *p1 == *p2;
				}
				else
				{
					if (pair_val_type_ == ValType::Map)
					{
						auto p1 = (MapIt*)buf_;
						auto p2 = (MapIt*)it.buf_;
						return *p1 == *p2;
					}
					else
						return pair_val_type_ == it.pair_val_type_;
				}
			}
			bool operator!=(const It& it) { return !this->operator==(it); }
		};

		It begin() const
		{
			It ret;
			ret.pair_val_type_ = val_type_;

			if (val_type_ == ValType::List)
			{
				auto& it = *(ListIt*)ret.buf_;
				it = val_.list_->begin();

				ret.pair_val_.first = nullptr;
				ret.pair_val_.second = &*it;
			}
			else
			{
				if (val_type_ == ValType::Map)
				{
					auto& it = *(MapIt*)ret.buf_;
					it = val_.map_->begin();

					ret.pair_val_.first = &it->first;
					ret.pair_val_.second = &it->second;
				}
				else
				{  
					ret.pair_val_.first = nullptr;
					ret.pair_val_.second = (ThisType*)this;
				}
			}
			return ret;
		}
		It end() const
		{
			It ret;
			ret.pair_val_type_ = val_type_;

			if (val_type_ == ValType::List)
			{
				auto& it = *(ListIt*)ret.buf_;
				it = val_.list_->end();
			}
			else
			{
				if (val_type_ == ValType::Map)
				{
					auto& it = *(MapIt*)ret.buf_;
					it = val_.map_->end();
				}
				else
				{
					ret.pair_val_type_ = ValType::None;
				}
			}
			return ret;
		}
	}; 
}
#endif//__clan_set_mix_obj__
