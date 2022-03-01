#ifndef __cl_base_call__
#define __cl_base_call__
 
#include "6ptr.h"
 
namespace cl
{
	namespace lib
	{
		template <typename Return, typename... Args> class _iCall
		{
		public:
			virtual ~_iCall() {}
			virtual Return call(Args...) const = 0;
			virtual void clone_to(void* buf) = 0;
		};

		template <typename Return, typename... Args> 
		class NormalFun : public _iCall<Return, Args...>
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
		template <typename Obj, typename Return, typename... Args> requires std::is_class_v<Obj>
		class ClassMemFun : public _iCall<Return, Args...>
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
		template <typename Obj, typename Return, typename... Args> requires std::is_class_v<Obj>
		class ObjFun : public _iCall<Return, Args...>
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

		//call with buf size
		template <size_t ByteSize, typename Return, typename... Args>
		class _SizeCall
		{
			using iCall = _iCall<Return, Args...>;
			//lamda捕捉的对象尽量少, 否则超过这个缓存... 
			uv8 buf_[ByteSize];
			 
			iCall* call() const { return (iCall*)buf_; }
		public:
			~_SizeCall()
			{
				if (valid()) call()->~_iCall(); 
			}

			_SizeCall() { *((void**)buf_) = nullptr; }
			_SizeCall(const std::nullptr_t&) : _SizeCall() { }

			//函数指针
			_SizeCall(Return(*fun)(Args...)) : _SizeCall()
			{
				using Type = NormalFun<Return, Args...>;
				Type* p = (Type*)buf_;
				new(p)Type();
				p->fun_ = fun; 
			}

			//类成员函数指针
			template<typename Obj, typename Fun> requires std::is_class_v<Obj>
			_SizeCall(Obj* obj, Fun fun) : _SizeCall()
			{
				static_assert(std::is_class_v<Obj>, "must be class!");
				static_assert(ByteSize <= sizeof(buf_), "Class Fun buf overflow!");
				static_assert(std::is_member_function_pointer_v<Fun>, "must be class funtion!");

				using Type = ClassMemFun<Obj, Return, Args...>;
				Type* p = (Type*)buf_;
				new(p)Type();
				p->obj_ = obj;
				p->fun_ = fun; 
			}

			//仿函数
			template<typename Obj> requires std::is_class_v<Obj>
			_SizeCall(Obj&& obj) : _SizeCall()
			{
				static_assert(std::is_class_v<Obj>, "must be class!");
				static_assert(sizeof(Obj) <= sizeof(buf_), "lambda buf overflow!");

				using Type = ObjFun<Obj, Return, Args...>;
				Type* p = (Type*)buf_;
				new(p)Type(std::forward<Obj>(obj)); 
			}

			_SizeCall(const _SizeCall& c)
			{
				if (valid()) c.call()->clone_to(buf_);
			}

			bool valid() 
			{
				if (*((void**)buf_) != nullptr) return true;
				return false; 
			}
			operator bool() const { return valid(); }
			Return operator()(Args... args) const { return call()->call(std::forward<Args>(args)...); }

			//普通call复制
			_SizeCall& operator=(const _SizeCall& c) { this->~_SizeCall(); new(this)_SizeCall(c); return *this; }
			//函数指针赋值
			_SizeCall& operator=(Return(*fun)(Args...)) { this->~_SizeCall(); new(this)_SizeCall(fun); return *this; }
			//仿函数, lambda表达式赋值
			template<typename Obj> requires std::is_class_v<Obj>
			_SizeCall& operator=(Obj&& obj) { this->~_SizeCall(); new(this)_SizeCall(std::forward<Obj>(obj)); return *this; }

			//类成员函数指针
			template<typename Obj, typename Fun>
			void bind(Obj* obj, Fun fun)
			{
				this->~_SizeCall();
				new(this)_SizeCall(obj, fun);
			}
			template<typename Obj, typename Fun> void bind(Obj& obj, Fun fun) { bind(&obj, fun); }
		};
	}

	//定义CallSize模板
	template <size_t ByteSize, typename Return, typename... Args>
	class SizeCall : public lib::_SizeCall<ByteSize, Return, Args...>
	{
	public:
		SizeCall() {}
		template<typename T> 
		SizeCall(T&& t) : lib::_SizeCall<ByteSize, Return, Args...>(t) {}
	};

	//偏特化CallSize模板
	template <size_t ByteSize, typename Return, typename... Args>
	class SizeCall<ByteSize, Return(Args...)> : public lib::_SizeCall<ByteSize, Return, Args...>
	{
	public:
		SizeCall() {}
		template<typename T>
		SizeCall(T&& t) : lib::_SizeCall<ByteSize, Return, Args...>(t) {}
	};
	
	/*############################################################################################*/
	/*############################################################################################*/
	template<typename Return, typename... Args> 
	using Call = SizeCall<8 * sizeof(void*), Return, Args...>;
}

#endif//__cl_base_call__ 