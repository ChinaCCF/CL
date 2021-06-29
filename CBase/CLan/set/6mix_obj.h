#ifndef __clan_set_mix_obj__
#define __clan_set_mix_obj__

#include "../base/8str.h"
#include "3list.h"
#include "5map.h"

namespace clan
{
    namespace detail
    {
        enum class MixObjType
        {
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
    class MixObj
    {
        using StrType = _String<C, A, 0>;
        using ValType = detail::MixObjType;
        using ListType = List<MixObj, A>;
        using MapType = HashMap<StrType, MixObj, A>;

        void _copy(const MixObj& mo);
    public: 
        union Val
        {
            f64 fv_;
            s64 sv_;
            bool bv_;
            StrType* str_;
            ListType* list_;
            MapType* map_;
        };
        
    public:
        ValType type_;
        Val val_;
        List<string> serial_;//当对象是map时候,保存串行化信息

        MixObj()
        {
            type_ = Type::Null;
            val_.sv_ = 0;
        }
        MixObj(MixObj&& mo) noexcept
        {
            type_ = mo.type_; mo.type_ = Type::Null;
            val_ = mo.val_; mo.val_.sv_ = 0;
            serial_ = std::move(mo.serial_);
        }
        MixObj(const MixObj& mo) { _copy(mo); }
        template<typename T>
        MixObj(const T& val)
        {
            type_ = Type::Null;
            val_.sv_ = 0;
            this->operator=(val);
        }
        ~MixObj() { release(); }

        void release();

        bool is_null() const { return type_ == Type::Null; }
        bool is_bool() const { return type_ == Type::Bool; }
        bool is_int() const { return type_ == Type::Int; }
        bool is_float() const { return type_ == Type::Float; }
        bool is_str() const { return type_ == Type::Str; }
        bool is_list() const { return type_ == Type::List; }
        bool is_map() const { return type_ == Type::Map; }

        /*####################################################################################################*/
        //类型转换
        /*####################################################################################################*/
    private:
        template<typename T>
        struct _ToStr
        {
            enum { value = IsStr<T>::value };
            inline T operator()(const ValType& type, const Val& val) const
            {
                if (type == Type::Null)
                    return T();
                cl_assert(type == Type::Str);
                cstr cs(*val.str_);
                return cs;
            }
        };
        template<typename T>
        struct _ToChar
        {
            enum { value = IsChar<T>::value };
            inline T operator()(const ValType& type, const Val& val) const
            {
                if (type == Type::Null)
                    return T();
                cl_assert(type == Type::Str);
                return (T)val.str_->data()[0];
            }
        };
        template<typename T>
        struct _ToInt
        {
            enum { value = IsInt<T>::value };
            inline T operator()(const ValType& type, const Val& val) const
            {
                if (type == Type::Null)
                    return T();
                if (type == Type::Int)
                    return (T)val.sv_;
                if (type == Type::Str)
                    return (T)*val.str_;
                cl_assert(false);
                return T();
            }
        };

        template<typename T>
        struct _ToFloat
        {
            enum { value = IsFloat<T>::value };
            inline T operator()(const ValType& type, const Val& val) const
            {
                if (type == Type::Null)
                    return T();
                if (type == Type::Float)
                    return (T)val.fv_;
                if (type == Type::Str)
                    return (T)*val.str_;
                cl_assert(false);
                return T();
            }
        };

        template<typename T>
        struct _ToBool
        {
            enum { value = IsBool<T>::value };
            inline T operator()(const ValType& type, const Val& val) const
            {
                if (type == Type::Null)
                    return false;
                if (type == Type::Bool)
                    return (T)val.bv_;
                if (type == Type::Str)
                    return (T)*val.str_;
                cl_assert(false);
                return T();
            }
        };
    public:
        template<typename T>
        operator T() const
        {
            using Obj = typename SelectSupportT<_ToBool<T>, _ToFloat<T>, _ToInt<T>, _ToChar<T>, _ToStr<T>>::type;
            return Obj()(type_, val_);
        }
        /*####################################################################################################*/
        //赋值
        /*####################################################################################################*/
    private:
        template<typename T>
        struct _SetStr
        {
            enum { value = IsStr<T>::value };
            inline void operator()(MixObj* obj, T&& val)
            {
                obj->type_ = Type::Str;
                obj->val_.str_ = cl_new(string, std::forward<T>(val));
            }
        };
        template<typename T>
        struct _SetInt
        {
            enum { value = IsInt<T>::value };
            inline void operator()(MixObj* obj, const T& val)
            {
                obj->type_ = Type::Int;
                obj->val_.sv_ = (s64)val;
            }
        };
        template<typename T>
        struct _SetFloat
        {
            enum { value = IsFloat<T>::value };
            inline void operator()(MixObj* obj, const T& val)
            {
                obj->type_ = Type::Float;
                obj->val_.fv_ = (f64)val;
            }
        };
        template<typename T>
        struct _SetBool
        {
            enum { value = IsBool<T>::value };
            inline void operator()(MixObj* obj, const T& val)
            {
                obj->type_ = Type::Bool;
                obj->val_.bv_ = (bool)val;
            }
        };
    public:
        void operator=(const std::nullptr_t&) { release(); }
        void operator=(const MixObj& mo) { release(); _copy(mo); }
        void operator=(MixObj& mo) { release(); _copy(mo); }
        void operator=(MixObj&& mo) noexcept
        {
            release();
            new(this)MixObj(std::forward<MixObj>(mo));
        }
        template<typename T>
        void operator=(T&& val)
        {
            release();
            using Obj = typename SelectSupportT<_SetBool<T>, _SetFloat<T>, _SetInt<T>, _SetStr<T>>::type;
            static_assert(!IsSame<Obj, void>::value, "no support!");
            Obj()(this, std::forward<T>(val));
        }

        /*####################################################################################################*/
        //列表
        /*####################################################################################################*/
        MixObj& operator[](int index)
        {
            cl_assert(type_ == Type::List);
            return val_.list_->operator[](index);
        }
        template<typename T>
        void push(T&& val)
        {
            if (type_ == Type::Null)
            {
                type_ = Type::List;
                val_.list_ = cl_new(ListType);
            }
            cl_assert(type_ == Type::List);
            MixObj obj;
            obj = std::forward<T>(val);
            val_.list_->push_back(std::move(obj));
        }
        void push(MixObj&& obj)
        {
            if (type_ == Type::Null)
            {
                type_ = Type::List;
                val_.list_ = cl_new(ListType);
            }
            cl_assert(type_ == Type::List);
            val_.list_->push_back(std::move(obj));
        }
        ListType& list() const
        {
            cl_assert(type_ == Type::List);
            return *val_.list_;
        }
        /*####################################################################################################*/
        //映射
        /*####################################################################################################*/
        MixObj& operator[](const char* key)
        {
            if (type_ == Type::Null)
            {
                type_ = Type::Map;
                val_.map_ = cl_new(MapType);
            }
            cl_assert(type_ == Type::Map);
            auto map = val_.map_;
            auto it = map->find(key);
            if (it != map->end())
                return it->second;
            serial_.push_back(key);
            return map->create(it, key);
        }
        MixObj& operator[](const char* key) const
        {
            cl_assert(type_ == Type::Map);
            auto map = val_.map_;
            auto it = map->find(key);
            cl_assert(it != map->end());
            return it->second;
        }

        MixObj& operator[](const cstr& key) { return this->operator[](key.data()); }
        MixObj& operator[](const string& key) { return this->operator[](key.data()); }
        MixObj& operator[](const std::string& key) { return this->operator[](key.data()); }
        MapType& map() const
        {
            cl_assert(type_ == Type::Map);
            return *val_.map_;
        }
        bool find(const char* key)
        {
            cl_assert(type_ == Type::Map);
            return val_.map_->find(key) != val_.map_->end();
        }
        void remove(const char* key)
        {
            cl_assert(type_ == Type::Map);
            return val_.map_->remove(key);
        }
    };

    namespace detail
    {
        //这里强制允许转换为string
        template<>
        struct _IsStr<MixObj>
        {
            enum { value = true };
        };
    }

}
#endif//__clan_set_mix_obj__
