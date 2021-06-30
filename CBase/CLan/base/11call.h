#ifndef __clan_base_call__
#define __clan_base_call__

#include <new>
#include "4concept.h"
#include "9obj.h"


namespace cl
{
	namespace detail
	{
		//由于需要继承, 所以Args&&会出错
		template <typename Return, typename... Args>
		class iCall
		{
		public:
			virtual ~iCall() {}
			virtual Return call(Args...) const = 0;
			virtual void copy_to(void* buf) = 0;
		};

		template <typename Return, typename... Args>
		//普通函数
		class NormalFun : public iCall<Return, Args...>
		{
		public:
			using FunType = Return(*)(Args...);
			FunType fun_ = nullptr;

			NormalFun() {}
			virtual Return call(Args... args) const override { return fun_(std::forward<Args>(args)...); }
			virtual void copy_to(void* buf) override
			{
				auto p = (NormalFun*)buf;
				new(p)NormalFun();
				p->fun_ = fun_;
			}
		};

		template <typename Obj, typename Return, typename... Args>
		//类成员函数指针
		class ClassFun : public iCall<Return, Args...>
		{
		public:
			using FunType = Return(Obj::*)(Args...);
			Obj* obj_ = nullptr;
			FunType fun_ = nullptr;

			ClassFun() {}
			virtual Return call(Args... args) const override { return (obj_->*fun_)(std::forward<Args>(args)...); }
			virtual void copy_to(void* buf) override
			{
				auto p = (ClassFun*)buf;
				new(p)ClassFun();
				p->obj_ = obj_;
				p->fun_ = fun_;
			}
		};

		template <typename Obj, typename Return, typename... Args>
		//仿函数
		class ObjFun : public iCall<Return, Args...>
		{
		public:
			Obj obj_;

			ObjFun(const Obj& obj) : obj_(obj) {} //由于匿名的lamda表达式不支持赋值拷贝, 必须构造赋值
			virtual ~ObjFun() {} //这里需要注意一下, 为啥iCall需要什么虚函数, 其实就是怕这里捕捉了什么奇怪对象
			virtual Return call(Args... args) const override { return obj_(std::forward<Args>(args)...); }
			virtual void copy_to(void* buf) override
			{
				auto p = (ObjFun*)buf;
				new(p)ObjFun(obj_); 
			}
		}; 
	}

	template <typename Return, typename... Args>
	class Call;

	//通用函数定义
	//支持 纯函数, 成员函数, lamda表达式(注意不要捕捉太多对象, 本对象64个字节,  实际可用为56字节)
	//扣除对齐等额外字节, 大概能够捕捉7个s64
	template <typename Return, typename... Args>
	class Call<Return(Args...)>
	{  
		using CallType = detail::iCall<Return, Args...>;
		//lamda捕捉的对象尽量少, 否则超过这个缓存...
		//class类对象指针 vt + obj_ + fun = 8 + 8 + 8 = 24
		u64 buf_[8];//64个字节
		CallType* call_ = nullptr; 
	public:
		~Call() { if (call_) call_->~iCall(); }

		Call() { clan_CheckClass(Call); }
		Call(const std::nullptr_t&) { }

		//函数指针
		Call(Return(*fun)(Args...))
		{
			using Type = detail::NormalFun<Return, Args...>;
			Type* p = (Type*)buf_;
			new(p)Type();
			p->fun_ = fun;
			call_ = p; 
		}

		//类成员函数指针
		template<ClassType Obj, ClassFunType Fun>
		Call(Obj* obj, Fun fun)
		{ 
			buf_[0] = 0;//成员未初始化警告

			using Type = detail::ClassFun<Obj, Return, Args...>;
			Type* p = (Type*)buf_;
			new(p)Type();
			p->obj_ = obj;
			p->fun_ = fun;
			call_ = p; 
		}

		//仿函数
		template<ClassType Obj>
		Call(const Obj& obj)
		{ 
			static_assert(sizeof(Obj) <= sizeof(buf_), "Call buf overflow!");
			buf_[0] = 0;//成员未初始化警告 

			using Type = detail::ObjFun<Obj, Return, Args...>;
			Type* p = (Type*)buf_;
			new(p)Type(obj); 
			call_ = p; 
		}

		Call(const Call& c)
		{ 
			if (c.call_)
			{
				c.call_->copy_to(buf_);
				call_ = (CallType*)buf_;
			} 
		}

		Call& operator=(const Call& c)
		{
			this->~Call();
			new(this)Call(c);
			return *this;
		}

		//函数指针
		Call& operator=(Return(*fun)(Args...))
		{
			this->~Call();
			new(this)Call(fun);
			return *this;
		}

		//类成员函数指针
		template<ClassType Obj, ClassFunType Fun>
		void bind(Obj* obj, Fun fun)
		{
			this->~Call();
			new(this)Call(obj, fun);
			return *this;
		}

		//仿函数
		template<ClassType Obj>
		Call& operator=(const Obj& obj)
		{
			this->~Call();
			new(this)Call(obj);
			return *this;
		}

		bool valid() { return call_ != nullptr; }
		operator bool() const { return valid(); }

		Return operator()(Args... args) const { return call_->call(std::forward<Args>(args)...); }
	};
}

#endif//__clan_base_call__ 