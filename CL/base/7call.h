#ifndef __cl_base_call__
#define __cl_base_call__
 
#include "6ptr.h"
 
namespace cl
{
	namespace lib
	{
		template <typename Return, typename... Args> class iCall
		{
		public:
			virtual ~iCall() {}
			virtual Return call(Args...) const = 0;
			virtual void clone_to(void* buf) = 0;
		};

		template <typename Return, typename... Args> 
		class NormalFun : public iCall<Return, Args...>
		{
		public:
			using FunType = Return(*)(Args...);
			FunType fun_ = nullptr;

			NormalFun() {}
			virtual Return call(Args... args) const override { return fun_(std::forward<Args>(args)...); }
			virtual void clone_to(void* buf) override
			{
				auto p = (NormalFun*)buf;
				new(p)NormalFun();
				p->fun_ = fun_;
			}
		};

		//类成员函数
		template <typename Obj, typename Return, typename... Args> 
		class ClassMemFun : public iCall<Return, Args...>
		{
		public:
			using FunType = Return(Obj::*)(Args...);
			Obj* obj_ = nullptr;
			FunType fun_ = nullptr;

			ClassMemFun() {}
			virtual Return call(Args... args) const override { return (obj_->*fun_)(std::forward<Args>(args)...); }
			virtual void clone_to(void* buf) override
			{
				auto p = (ClassMemFun*)buf;
				new(p)ClassMemFun();
				p->obj_ = obj_;
				p->fun_ = fun_;
			}
		};

		//仿函数对象
		template <typename Obj, typename Return, typename... Args>
		class ObjFun : public iCall<Return, Args...>
		{
		public:
			Obj obj_;

			ObjFun(Obj&& obj) : obj_(std::forward<Obj>(obj)) {}

			virtual ~ObjFun() {}  
			virtual Return call(Args... args) const override { return obj_(std::forward<Args>(args)...); }
			virtual void clone_to(void* buf) override
			{
				auto p = (ObjFun*)buf;
				new(p)ObjFun(obj_);
			}
		};
	}

	template <typename Return, typename... Args> class Call;

	template <typename Return, typename... Args> class Call<Return(Args...)>
	{
		using iCall = lib::iCall<Return, Args...>;
		//lamda捕捉的对象尽量少, 否则超过这个缓存... 
		uv64 buf_[16];
		iCall* call_ = nullptr;
	public:
		~Call() { if (call_) call_->~iCall(); }

		Call() { buf_[0] = 0; }
		Call(const std::nullptr_t&) : Call() { }

		//函数指针
		Call(Return(*fun)(Args...)) : Call()
		{
			using Type = lib::NormalFun<Return, Args...>;
			Type* p = (Type*)buf_;
			new(p)Type();
			p->fun_ = fun;
			call_ = p;
		}
		 
		//类成员函数指针
		template<typename Obj, typename Fun> Call(Obj* obj, Fun fun) : Call()
		{
			static_assert(std::is_class_v<Obj>, "must be class!");
			static_assert(std::is_member_function_pointer_v<Fun>, "must be class funtion!");
			  
			using Type = lib::ClassMemFun<Obj, Return, Args...>;
			Type* p = (Type*)buf_;
			new(p)Type();
			p->obj_ = obj;
			p->fun_ = fun;
			call_ = p;
		}

		//仿函数
		template<typename Obj> Call(Obj&& obj) : Call()
		{
			static_assert(std::is_class_v<Obj>, "must be object!");
			static_assert(sizeof(Obj) <= sizeof(buf_), "lambda buf overflow!");

			using Type = lib::ObjFun<Obj, Return, Args...>;
			Type* p = (Type*)buf_;
			new(p)Type(std::forward<Obj>(obj));
			call_ = p;
		}

		Call(const Call& c)
		{
			if (c.call_)
			{
				c.call_->clone_to(buf_);
				call_ = (iCall*)buf_;
			}
		}

		bool valid() { return call_ != nullptr; }
		operator bool() const { return valid(); }
		Return operator()(Args... args) const { return call_->call(std::forward<Args>(args)...); }

		Call& operator=(const Call& c) { this->~Call(); new(this)Call(c); return *this; }
		Call& operator=(Return(*fun)(Args...)) { this->~Call(); new(this)Call(fun); return *this; }
		template<typename Obj>	Call& operator=(Obj&& obj) { this->~Call(); new(this)Call(std::forward<Obj>(obj)); return *this; }

		//类成员函数指针
		template<typename Obj, typename Fun>
		void bind(Obj* obj, Fun fun)
		{
			this->~Call();
			new(this)Call(obj, fun);
		}  
	};
}

#endif//__cl_base_call__ 