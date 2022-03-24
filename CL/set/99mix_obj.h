#ifndef __cl_set_mix_obj__
#define __cl_set_mix_obj__

#include "../base/8str.h" 
#include "5map.h"
 
//混合对象, 现时用于json解析
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

	template<CharType C, AllocFreeType A>
	class _MixObj : public NoCopyObj
	{
	public:
		using ThisType = _MixObj<C, A>;
		using MixValType = detail::MixObjType;
		using MixValStrType = _NString<C, A, 0>;
		using MixValListType = _List<ThisType, A>;
		using MixValMapType = _HashMap<MixValStrType, ThisType, A>;

		using ListIt = typename MixValListType::It;
		using MapIt = typename MixValMapType::It;
		 
		union Val
		{
			bool bv_; 
			s64 sv_;
			f64 fv_;
			MixValStrType* str_;
			MixValListType* list_;
			MixValMapType* map_;
		};
		mutable MixValType val_type_;
		mutable Val val_;  
	private:
		void _release()
		{
			if (val_type_ == MixValType::Str)
				_del<A>(val_.str_);
			if (val_type_ == MixValType::Map)
				_del<A>(val_.map_);
			if (val_type_ == MixValType::List)
				_del<A>(val_.list_);

			val_type_ = MixValType::None;
		} 
	public:
		_MixObj()
		{ 
			cl_CheckClass(_MixObj);
			static_assert(sizeof(s64) >= sizeof(MixValStrType*) && sizeof(f64) >= sizeof(MixValStrType*), "s64 or f64 size is smaller than pointer!");
			val_type_ = MixValType::None;
			val_.sv_ = 0;
		} 
		_MixObj(_MixObj&& mo) noexcept
		{  
			val_.sv_ = mo.val_.sv_;
			val_type_ = mo.val_type_;

			mo.val_.sv_ = 0;
			mo.val_type_ = MixValType::None;
		}
		_MixObj(const std::nullptr_t&)
		{   
			val_type_ = MixValType::Null;
			val_.sv_ = 0;
		} 

		_MixObj(bool val)
		{   
			val_type_ = MixValType::Bool;
			val_.bv_ = val;
		};
		_MixObj(C val)
		{  
			val_type_ = MixValType::Int;
			val_.sv_ = val;
		};
		template<IntType T> _MixObj(T val)
		{  
			val_type_ = MixValType::Int;
			val_.sv_ = val;
		};
		template<FloatType T> _MixObj(T val)
		{  
			val_type_ = MixValType::Float;
			val_.fv_ = val;
		}; 
		_MixObj(const C* val)
		{  
			val_type_ = MixValType::Str;
			val_.str_ = _new<A, MixValStrType>(val);
		}
		_MixObj(const MixValStrType& val)
		{  
			val_type_ = MixValType::Str;
			val_.str_ = _new<A, MixValStrType>(std::move(val));
		}
		_MixObj(MixValStrType&& val)
		{  
			val_type_ = MixValType::Str;
			val_.str_ = _new<A, MixValStrType>(std::move(val));
		}
		  
		~_MixObj() { _release(); }

		template<typename T>
		void operator=(T&& val) { _release(); new(this)_MixObj(std::forward<T>(val)); }

		bool is_none() const { return val_type_ == MixValType::None; }
		bool is_null() const { return val_type_ == MixValType::Null; }
		bool is_bool() const { return val_type_ == MixValType::Bool; }
		bool is_int() const { return val_type_ == MixValType::Int; }
		bool is_float() const { return val_type_ == MixValType::Float; }
		bool is_str() const { return val_type_ == MixValType::Str; }
		bool is_list() const { return val_type_ == MixValType::List; }
		bool is_map() const { return val_type_ == MixValType::Map; }

		s32 size() const
		{
			if (val_type_ == MixValType::None || val_type_ == MixValType::Null) return 0;
			if (val_type_ == MixValType::List) return val_.list_->size();
			if (val_type_ == MixValType::Map) return val_.map_->size();
			cl_assert(false);
			return 0;
		}
		/*####################################################################################################*/
		//类型转换
		/*####################################################################################################*/ 
		operator bool() const
		{
			[[likely]]
			if (val_type_ == MixValType::Bool)
				return val_.bv_;
			if (val_type_ == MixValType::Str)
				return (bool)*val_.str_;

			cl_assert(false);
			return false;
		};
		operator C() const
		{
			[[likely]]
			if (val_type_ == MixValType::Int)
				return (C)val_.sv_;
			if (val_type_ == MixValType::Str)
				return val_.str_->data()[0];

			cl_assert(false);
			return C();
		};
		template<IntType T>
		operator T() const
		{
			[[likely]]
			if (val_type_ == MixValType::Int)
				return (T)val_.sv_;
			if (val_type_ == MixValType::Str)
				return (T)*val_.str_;

			cl_assert(false);
			return T();
		};
		template<FloatType T>
		operator T() const
		{
			[[likely]]
			if (val_type_ == MixValType::Float)
				return (T)val_.fv_;
			if (val_type_ == MixValType::Str)
				return (T)*val_.str_;

			cl_assert(false);
			return T();
		};  
		operator C*() const
		{
			[[likely]]
			if (val_type_ == MixValType::Str)
				return (C*)*val_.str_;

			cl_assert(false);
			return nullptr;
		}; 
		/*####################################################################################################*/
		//列表
		/*####################################################################################################*/
		_MixObj& operator[](int index) const
		{
			cl_assert(val_type_ == MixValType::List);
			return &val_.list_->operator[](index);
		}
		void push(_MixObj&& obj)
		{
			if (val_type_ == MixValType::Null || val_type_ == MixValType::None)
			{
				val_type_ = MixValType::List;
				val_.list_ = _new<A, MixValListType>();
			}
			else
				cl_assert(val_type_ == MixValType::List);

			val_.list_->push_back(std::move(obj));
		}

		template<BaseValType T>
		void push(const T& val)
		{
			_MixObj obj = val;
			push(std::move(obj));
		}
		template<ToCharPtrType T>
		void push(T&& val)
		{
			using _CharType = SelectCharType<T>::type;
			_MixObj obj = (const _CharType*)val;
			push(std::move(obj));
		}
		MixValListType* list() const
		{
			cl_assert(val_type_ == MixValType::List);
			return val_.list_;
		}
		/*####################################################################################################*/
		//映射
		/*####################################################################################################*/
		template<ToCharPtrType T> 
		_MixObj& operator[](const T& key) const
		{
			if (val_type_ == MixValType::Null || val_type_ == MixValType::None)
			{
				val_type_ = MixValType::Map;
				val_.map_ = _new<A, MixValMapType>();
			}
			else
				cl_assert(val_type_ == MixValType::Map);

			auto map = val_.map_;
			return (*map)[key];
		}

		MixValMapType* map() const
		{
			cl_assert(val_type_ == MixValType::Map);
			return val_.map_;
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
			using ItPair = _ItPair<MixValStrType, ThisType>;
			  
			MixValType it_type_ = MixValType::None;
			ItPair it_val_;

			static constexpr s32 Buf_Size = 32;
			u8 it_buf_[Buf_Size];//用来保存各个类型的It值, 注意这个值要放在最后, 否则会出现奇怪的内存越界问题 

			//更新it的值
			void _update_val(const ThisType* self = nullptr)
			{
				if (it_type_ == MixValType::List)
				{
					auto& it = *(ListIt*)it_buf_;
					new(&it_val_)ItPair(*(MixValStrType*)nullptr, *it);
				}
				else
				{
					if (it_type_ == MixValType::Map)
					{
						auto& it = *(MapIt*)it_buf_;
						new(&it_val_)ItPair(it->first, it->second);
					}
					else
					{
						new(&it_val_)ItPair(*(MixValStrType*)nullptr, *(ThisType*)self);
					}
				}
			}
		public:
			It()  
			{ 
				it_buf_[0] = 0; 
				static_assert(Buf_Size > sizeof(ListIt) && Buf_Size > sizeof(MapIt), "It buf over!");
			}
			It(const It& it)  
			{
				it_buf_[0] = 0; 
				memcpy(this, &it, sizeof(it));
			} 
			 
			ItPair& operator*() 
			{
				return it_val_;
			}
			ItPair* operator->() { return &it_val_; }
			It& operator++()
			{
				if (it_type_ == MixValType::List)
				{
					auto& it = *(ListIt*)it_buf_;
					++it;
				}
				else
				{
					if (it_type_ == MixValType::Map)
					{
						auto& it = *(MapIt*)it_buf_;
						++it;
					}
					else
					{
						it_type_ = MixValType::None;
					}
				}
				_update_val();
				return *this;
			}
			bool operator==(const It& it) const
			{
				if (it_type_ == MixValType::List)
				{ 
					auto p1 = (ListIt*)it_buf_;
					auto p2 = (ListIt*)it.it_buf_;
					return *p1 == *p2;
				}
				else
				{
					if (it_type_ == MixValType::Map)
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

		template<ToCharPtrType T>
		It find(const T& key)
		{ 
			cl_assert(val_type_ == MixValType::Map);
			auto map = val_.map_;

			It ret;
			ret.it_type_ = MixValType::Map; 
			*(MapIt*)(ret.it_buf_) = val_.map_->find(key); 
			ret._update_val(this);
			return ret; 
		}

		It begin() const
		{
			It ret;
			ret.it_type_ = val_type_;

			if (val_type_ == MixValType::List)
				*(ListIt*)(ret.it_buf_) = val_.list_->begin();
			else
			{
				if (val_type_ == MixValType::Map)
					*(MapIt*)(ret.it_buf_) = val_.map_->begin(); 
			}
			ret._update_val(this);
			return ret;
		}
		It end() const
		{
			It ret;
			ret.it_type_ = val_type_;

			if (val_type_ == MixValType::List)
			{
				auto end = val_.list_->end();
				*(ListIt*)(ret.it_buf_) = end;
			}
			else
			{
				if (val_type_ == MixValType::Map)
					*(MapIt*)(ret.it_buf_) = val_.map_->end();
				else
				{
					ret.it_type_ = MixValType::None; 
				} 
			}
			return ret;
		}
	};
}
#endif//__cl_set_mix_obj__
