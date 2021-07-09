#ifndef __clan_set_mix_obj__
#define __clan_set_mix_obj__

#ifdef XDEBUG
#include <windows.h>
#endif

#include "../base/8str.h" 
#include "5map.h"
 
namespace cl
{
	namespace detail
	{
		enum class MixObjType
		{
			None,//默认值, 初始状态, 表示没有经过赋值
			Null,//这是一个值, 表示null
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
			bool bv_; 
			s64 sv_;
			f64 fv_;
			StrValType* str_;
			ListValType* list_;
			MapValType* map_;
		};
		mutable ValType val_type_ = ValType::None;
		mutable Val val_;  
	private:
		void _release()
		{
			if (val_type_ == ValType::Str)
				_del<A>(val_.str_);
			if (val_type_ == ValType::Map)
				_del<A>(val_.map_);
			if (val_type_ == ValType::List)
				_del<A>(val_.list_);

			val_type_ = ValType::None;
		}
	public:
		_MixObj()
		{
			static_assert(sizeof(s64) >= sizeof(StrValType*) && sizeof(f64) >= sizeof(StrValType*), "s64 or f64 size is smaller than pointer!");
			val_.sv_ = 0;
		}
		_MixObj(_MixObj&& mo) noexcept
		{
			val_.sv_ = mo.val_.sv_;
			val_type_ = mo.val_type_;

			mo.val_.sv_ = 0;
			mo.val_type_ = ValType::None;
		}

		template<typename T> requires IsNotSameType<T, _MixObj>::value
			_MixObj(T&& val) { this->operator=(std::forward<T>(val)); }

		~_MixObj() 
		{
#ifdef XDEBUG
			OutputDebugStringA("~_MixObj\n");
#endif
			_release();
		}

		bool is_none() const { return val_type_ == ValType::None; }
		bool is_null() const { return val_type_ == ValType::Null; }
		bool is_bool() const { return val_type_ == ValType::Bool; }
		bool is_int() const { return val_type_ == ValType::Int; }
		bool is_float() const { return val_type_ == ValType::Float; }
		bool is_str() const { return val_type_ == ValType::Str; }
		bool is_list() const { return val_type_ == ValType::List; }
		bool is_map() const { return val_type_ == ValType::Map; }

		s32 size() const
		{
			if (val_type_ == ValType::None || val_type_ == ValType::Null) return 0;
			if (val_type_ == ValType::List) return val_.list_->size();
			if (val_type_ == ValType::Map) return val_.map_->size();
		}
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
		void operator=(const std::nullptr_t&) { _release(); val_type_ = ValType::Null; }
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
			if (val_type_ == ValType::Null || val_type_ == ValType::None)
			{
				val_type_ = ValType::List;
				val_.list_ = _new<A, ListValType>();
			}
			else
				cl_assert(val_type_ == ValType::List);

			val_.list_->push_back(std::move(obj));
		}
		template<typename T> requires IsNotSameType<T, _MixObj>::value
			void push(const T& val)
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
		template<typename T> requires ToChars<T, C>::value
			_MixObj& operator[](const T& key) const
		{
			if (val_type_ == ValType::Null || val_type_ == ValType::None)
			{
				val_type_ = ValType::Map;
				val_.map_ = _new<A, MapValType>();
			}
			else
				cl_assert(val_type_ == ValType::Map);

			auto map = val_.map_;
			return (*map)[key];
		}

		MapValType* map() const
		{
			cl_assert(val_type_ == ValType::Map);
			return val_.map_;
		}

		template<typename T> requires ToChars<T, C>::value
			bool contain(const T& key)
		{
			if (val_type_ == ValType::None || val_type_ == ValType::Null) return false;
			cl_assert(val_type_ == ValType::Map);
			auto map = val_.map_;
			return map->contain(key);
		}

		/*####################################################################################################*/
		//It
		/*####################################################################################################*/
		class It
		{
			friend class _MixObj<C, A>; 

			template<typename _K, typename _V>
			struct _ItPair
			{
				_K& first;
				_V& second;
				_ItPair() : first(*(_K*)nullptr), second(*(_V*)nullptr) {}
				_ItPair(_K& key, _V& val) : first(key), second(val) {}
			};
			using ItPair = _ItPair<StrValType, ThisType>;
			  
			ValType it_type_ = ValType::None;
			ItPair it_val_;

			static constexpr s32 Buf_Size = 32;
			u8 it_buf_[Buf_Size];//用来保存各个类型的It值, 注意这个值要放在最后, 否则会出现奇怪的内存越界问题 
		public:
			It()  
			{ 
				it_buf_[0] = 0; 
				static_assert(Buf_Size > sizeof(ListIt) && Buf_Size > sizeof(MapIt), "It buf over!");
			}
			It(const It& it)  
			{
				memcpy(this, &it, sizeof(it));
			}

#ifdef XDEBUG
			u64 _over_flow_ = 0;
			~It() { cl_assert(_over_flow_ == 0); }
#endif

			void update_it_val(const ThisType* self = nullptr)
			{
				if (it_type_ == ValType::List)
				{
					auto& it = *(ListIt*)it_buf_;
					new(&it_val_)ItPair(*(StrValType*)nullptr, *it);
				}
				else
				{
					if (it_type_ == ValType::Map)
					{
						auto& it = *(MapIt*)it_buf_;
						new(&it_val_)ItPair(it->first, it->second);
					}
					else
					{
						new(&it_val_)ItPair(*(StrValType*)nullptr, *(ThisType*)self);
					}
				}
			}

			ItPair& operator*() 
			{
				return it_val_;
			}
			ItPair* operator->() { return &it_val_; }
			It& operator++()
			{
				if (it_type_ == ValType::List)
				{
					auto& it = *(ListIt*)it_buf_;
					++it;
				}
				else
				{
					if (it_type_ == ValType::Map)
					{
						auto& it = *(MapIt*)it_buf_;
						++it;
					}
					else
					{
						it_type_ = ValType::None;
					}
				}
				update_it_val();
				return *this;
			}
			bool operator==(const It& it) const
			{
				if (it_type_ == ValType::List)
				{ 
					auto p1 = (ListIt*)it_buf_;
					auto p2 = (ListIt*)it.it_buf_;
					return *p1 == *p2;
				}
				else
				{
					if (it_type_ == ValType::Map)
					{
						auto p1 = (MapIt*)it_buf_;
						auto p2 = (MapIt*)it.it_buf_;
						return *p1 == *p2;
					}
					else
						return it_type_ == it.it_type_;
				}
			}
			bool operator!=(const It& it) const { return !operator==(it); }
		};

		It begin() const
		{
			It ret;
			ret.it_type_ = val_type_;

			if (val_type_ == ValType::List)
				*(ListIt*)(ret.it_buf_) = val_.list_->begin();
			else
			{
				if (val_type_ == ValType::Map)
					*(MapIt*)(ret.it_buf_) = val_.map_->begin(); 
			}
			ret.update_it_val(this);
			return ret;
		}
		It end() const
		{
			It ret;
			ret.it_type_ = val_type_;

			if (val_type_ == ValType::List)
			{
				auto end = val_.list_->end();
				*(ListIt*)(ret.it_buf_) = end;
			}
			else
			{
				if (val_type_ == ValType::Map)
					*(MapIt*)(ret.it_buf_) = val_.map_->end();
				else
				{
					ret.it_type_ = ValType::None; 
				} 
			}
			return ret;
		}
	};
}
#endif//__clan_set_mix_obj__
