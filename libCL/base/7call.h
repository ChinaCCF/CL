#ifndef __cl_base_call__
#define __cl_base_call__
 
#include "6ptr.h"
 
namespace cl
{
	namespace lib
	{
		template <typename Return, typename... Args> 
		class iCall
		{
		public:
			virtual ~iCall() {}
			virtual Return iCall_call(Args...) const = 0;
			virtual void iCall_clone_to(void* buf) = 0;
		};

		template <typename Return, typename... Args> 
		class NormalFun : public iCall<Return, Args...>
		{
		public:
			using FunType = Return(*)(Args...);
			FunType fun_ = nullptr;

			NormalFun() {}
			virtual Return iCall_call(Args... args) const override { return fun_(std::forward<Args>(args)...); }
			virtual void iCall_clone_to(void* buf) override
			{
				auto p = (NormalFun*)buf;
				new(p)NormalFun();
				p->fun_ = fun_;
			}
		};

		//类成员函数
		template <typename Obj, typename Return, typename... Args> requires std::is_class_v<Obj>
		class ClassMemFun : public iCall<Return, Args...>
		{
		public:
			using FunType = Return(Obj::*)(Args...);
			Obj* obj_ = nullptr;
			FunType fun_ = nullptr;

			ClassMemFun() {}
			virtual Return iCall_call(Args... args) const override { return (obj_->*fun_)(std::forward<Args>(args)...); }
			virtual void iCall_clone_to(void* buf) override
			{
				auto p = (ClassMemFun*)buf;
				new(p)ClassMemFun();
				p->obj_ = obj_;
				p->fun_ = fun_;
			}
		};

		//仿函数对象
		template <typename Obj, typename Return, typename... Args> requires std::is_class_v<Obj>
		class ObjFun : public iCall<Return, Args...>
		{
		public:
			Obj obj_;

			ObjFun(const Obj& obj) : obj_(obj) {} 

			virtual ~ObjFun() {}  
			virtual Return iCall_call(Args... args) const override { return obj_(std::forward<Args>(args)...); }
			virtual void iCall_clone_to(void* buf) override
			{
				auto p = (ObjFun*)buf;
				new(p)ObjFun(obj_);
			}
		}; 
	}

	template <size_t ByteSize, typename Return, typename... Args>
	class SizeCall;

	//call with buf size
	template <size_t ByteSize, typename Return, typename... Args>
	class SizeCall<ByteSize, Return(Args...)>
	{
		using iCall = lib::iCall<Return, Args...>;
		//lamda捕捉的对象尽量少, 否则超过这个缓存... 
		uv8 buf_[ByteSize];

		iCall* get_call() const { return (iCall*)buf_; }
	public:
		~SizeCall()
		{
			if (valid()) get_call()->~iCall();
		}

		SizeCall() { *((void**)buf_) = nullptr; }
		SizeCall(const std::nullptr_t&) : SizeCall() { }

		//函数指针
		SizeCall(Return(*fun)(Args...)) : SizeCall()
		{
			using Type = lib::NormalFun<Return, Args...>;
			Type* p = (Type*)buf_;
			new(p)Type();
			p->fun_ = fun;
		}

		//类成员函数指针
		template<typename Obj, typename Fun> requires std::is_class_v<Obj>
		SizeCall(Obj* obj, Fun fun) : SizeCall()
		{
			static_assert(std::is_class_v<Obj>, "must be class!");
			static_assert(ByteSize <= sizeof(buf_), "Class Fun buf overflow!");
			static_assert(std::is_member_function_pointer_v<Fun>, "must be class funtion!");

			using Type = lib::ClassMemFun<Obj, Return, Args...>;
			Type* p = (Type*)buf_;
			new(p)Type();
			p->obj_ = obj;
			p->fun_ = fun;
		}

		//仿函数, lambda 基本都是拷贝构造
		template<typename Obj> requires std::is_class_v<Obj>
		SizeCall(const Obj& obj) : SizeCall()
		{
			static_assert(sizeof(Obj) <= sizeof(buf_), "lambda buf overflow!");

			using Type = lib::ObjFun<Obj, Return, Args...>;
			Type* p = (Type*)buf_;
			new(p)Type(obj);
		}

		SizeCall(const SizeCall& c)
		{
			if (valid()) c.get_call()->iCall_clone_to(buf_);
		}

		bool valid()
		{
			if (*((void**)buf_) != nullptr) return true;
			return false;
		}

		operator bool() const { return valid(); }
		Return operator()(Args... args) const { return get_call()->iCall_call(std::forward<Args>(args)...); }

		//普通call复制
		SizeCall& operator=(const SizeCall& c) { this->~SizeCall(); new(this)SizeCall(c); return *this; }
		//函数指针赋值
		SizeCall& operator=(Return(*fun)(Args...)) { this->~SizeCall(); new(this)SizeCall(fun); return *this; }
		//仿函数, lambda表达式赋值 
		template<typename Obj> requires std::is_class_v<Obj>
		SizeCall& operator=(const Obj& obj) { this->~SizeCall(); new(this)SizeCall(obj); return *this; }

		//类成员函数指针
		template<typename Obj, typename Fun>
		void bind(Obj* obj, Fun fun)
		{
			this->~SizeCall();
			new(this)SizeCall(obj, fun);
		}
		template<typename Obj, typename Fun> void bind(Obj& obj, Fun fun) { bind(&obj, fun); }
	}; 

	template<typename Return, typename... Args> 
	using Call = SizeCall<8 * sizeof(void*), Return, Args...>;
}

#endif//__cl_base_call__ 