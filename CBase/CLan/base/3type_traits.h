#ifndef __clan_base_type_traits__
#define __clan_base_type_traits__

//对std的<type_traits>补充
//一些基础type_traits是无法复写的, 因为要用到编译器底层接口, 这些接口...
//例如 std::is_convertible, 就用到 __is_convertible_to, 这个东西是无法复写的
#include "2type.h"
#include <type_traits>

namespace cl
{
#define const_bool static constexpr bool
	/*#####################################################################################*/
	//类似于std::decay, 但是仅对有限的类型操作
	/*#####################################################################################*/
	template<typename T> struct RawType { using type = T; const_bool is_ptr = false; };
	template<typename T> struct RawType<T&> { using type = T; const_bool is_ptr = false; };
	template<typename T> struct RawType<T&&> { using type = T; const_bool is_ptr = false; };
	template<typename T> struct RawType<const T> { using type = T; const_bool is_ptr = false; };
	template<typename T> struct RawType<const T&> { using type = T; const_bool is_ptr = false; };

	template<typename T> struct RawType<T*> { using type = T*; const_bool is_ptr = true; };
	template<typename T> struct RawType<const T*> { using type = T*; const_bool is_ptr = true; };

	template<typename T> struct RawType<T[]> { using type = T*; const_bool is_ptr = true; };
	template<typename T> struct RawType<const T[]> { using type = T*; const_bool is_ptr = true; };
	template<typename T> struct RawType<const T(&)[]> { using type = T*; const_bool is_ptr = true; };

	template<typename T, int N> struct RawType<T[N]> { using type = T*; const_bool is_ptr = true; };
	template<typename T, int N> struct RawType<const T[N]> { using type = T*; const_bool is_ptr = true; };
	template<typename T, int N> struct RawType<const T(&)[N]> { using type = T*; const_bool is_ptr = true; };

	namespace detail
	{
		/*#####################################################################################*/
		/*#####################################################################################*/
		template<typename T> struct _IsC8 { const_bool value = false; };
		template<> struct _IsC8<char> { const_bool value = true; };

		template<typename T> struct _IsC16 { const_bool value = false; };
		template<> struct _IsC16<wchar> { const_bool value = true; };

		/*#####################################################################################*/
		/*#####################################################################################*/

		template<typename T> struct _IsBool { const_bool value = false; };
		template<> struct _IsBool<bool> { const_bool value = true; };
		/*#####################################################################################*/
		/*#####################################################################################*/

		template<typename T> struct _IsFloat { const_bool value = false; };
		template<> struct _IsFloat<f32> { const_bool value = true; };
		template<> struct _IsFloat<f64> { const_bool value = true; };

		/*#####################################################################################*/
		/*#####################################################################################*/

		template<typename T> struct _IsSignedInt { const_bool value = false; };
		template<> struct _IsSignedInt<s8> { const_bool value = true; };
		template<> struct _IsSignedInt<s16> { const_bool value = true; };
		template<> struct _IsSignedInt<s32> { const_bool value = true; };
		template<> struct _IsSignedInt<s64> { const_bool value = true; };

		template<typename T> struct _IsUnsignedInt { const_bool value = false; };
		template<> struct _IsUnsignedInt<u8> { const_bool value = true; };
		template<> struct _IsUnsignedInt<u16> { const_bool value = true; };
		template<> struct _IsUnsignedInt<u32> { const_bool value = true; };
		template<> struct _IsUnsignedInt<u64> { const_bool value = true; }; 
	}

	template<typename T> struct IsC8 { const_bool value = detail::_IsC8<RawType<T>::type>::value; };
	template<typename T> struct IsC16 { const_bool value = detail::_IsC16<RawType<T>::type>::value; };
	template<typename T> struct IsChar { const_bool value = detail::_IsC16<RawType<T>::type>::value || detail::_IsC8<RawType<T>::type>::value; };
	template<typename T> struct IsChars {const_bool value = IsSameType<RawType<T>::type, char*>::value || IsSameType<RawType<T>::type, wchar*>::value;};
	template<typename T> struct IsNotChars { const_bool value = !IsSameType<RawType<T>::type, char*>::value && !IsSameType<RawType<T>::type, wchar*>::value; };
	
	template<typename T> struct Convert2C8 { const_bool value = std::is_convertible<T, char*>::value || std::is_convertible<T, const char*>::value; };
	template<typename T> struct Convert2C16 { const_bool value = std::is_convertible<T, wchar*>::value || std::is_convertible<T, const wchar*>::value; };

	template<typename T> struct IsBool { const_bool value = detail::_IsBool<RawType<T>::type>::value; };
	template<typename T> struct IsFloat { const_bool value = detail::_IsFloat<RawType<T>::type>::value; };

	template<typename T> struct IsSignedInt { const_bool value = detail::_IsSignedInt<RawType<T>::type>::value; };
	template<typename T> struct IsUnsignedInt { const_bool value = detail::_IsUnsignedInt<RawType<T>::type>::value; };
	template<typename T> struct IsInt { const_bool value = IsSignedInt<T>::value || IsUnsignedInt<T>::value; };

	/*#####################################################################################*/
	//判断是否是数值类型
	/*#####################################################################################*/
	template<typename T> struct IsVal { const_bool value = IsBool<T>::value || IsFloat<T>::value || IsInt<T>::value || IsChar<T>::value; };

	/*#####################################################################################*/
	//判断是否是指针
	/*#####################################################################################*/
	template<typename T> struct IsPtr { const_bool value = RawType<T>::is_ptr; };//{ enum { value = std::is_pointer<T>::value }; };

	/*#####################################################################################*/
	//判断是否是相同的类型
	/*#####################################################################################*/
	template<typename T1, typename T2> struct IsSameType { const_bool value = false; };
	template<typename T> struct IsSameType<T, T> { const_bool value = true; };

	/*#####################################################################################*/
	//判断是否是类
	/*#####################################################################################*/
	template<typename T> struct IsClass { const_bool value = std::is_class<T>::value; };

	/*#####################################################################################*/
	//判断是否是成员函数
	/*#####################################################################################*/
	template<typename T> struct IsClassFun { const_bool value = std::is_member_function_pointer<T>::value; };
	/*#####################################################################################*/
	//判断是否右值移动赋值
	/*#####################################################################################*/
	template<typename T> struct IsMoveAssignable { const_bool value = std::is_move_assignable<T>::value; };

	/*#####################################################################################*/
	//判断是否右值构造
	/*#####################################################################################*/
	template<typename T> struct IsMoveConstructible { const_bool value = std::is_move_constructible<T>::value; };

	/*#####################################################################################*/
	//判断是否是虚基类, 类中有虚函数的类, std::is_abstract是判断是否声明了纯虚函数
	/*#####################################################################################*/
	template<typename T> struct IsVirtualClass { const_bool value = std::is_polymorphic<T>::value; };
	  
	/*#####################################################################################*/
	//检查类
	/*#####################################################################################*/
	namespace detail
	{
		//判断虚基类是否有虚析构函数
		template<typename T> struct HasVirtualDestructor { const_bool value = std::has_virtual_destructor<T>::value; }; 
		//常规类的右值赋值和右值构造检查
		template<typename T> struct NormalClassCheck { const_bool value = IsMoveAssignable<T>::value && IsMoveConstructible<T>::value; };
		//检查虚函数是否定义了虚析构函数
		template<typename T> struct VirtualClassCheck { const_bool value = IsVirtualClass<T>::value ? HasVirtualDestructor<T>::value : true; };
	} 
	//检查类是否定义了移动语义
	template<typename T> struct MoveCheck { const_bool value = IsClass<T>::value ? detail::NormalClassCheck<T>::value : true; };
	//检查类的是否定义了虚析构函数
	template<typename T> struct DestructorCheck { const_bool value = IsClass<T>::value ? detail::VirtualClassCheck<T>::value : true; };
	 
	//检查类的定义详情,是否定义了移动语义, 虚类是否定义了虚析构函数
#define  clan_CheckClass(T) \
static_assert(cl::MoveCheck<T>::value, "class need move assign and construct!");\
static_assert(cl::DestructorCheck<T>::value, "virtual class need virtual destructor!");

	/*#####################################################################################*/
	//选择类型, 等价于 typename std::conditional<bool, Type1, Type2>::type;
	/*#####################################################################################*/
	template<bool, typename Type1, typename Type2>
	struct SelectType
	{
		using type = Type1;
	};
	template<typename Type1, typename Type2>
	struct SelectType<false, Type1, Type2>
	{
		using type = Type2;
	};
	/*#####################################################################################*/
	//选择2个类型中存储空间大的一种,例如s8和s16,会选择s16 
	/*#####################################################################################*/
	template<typename T1, typename ... Args>
	struct MaxType
	{
		using T2 = typename MaxType<Args ...>::type;

		using type = typename SelectType < sizeof(T1) < sizeof(T2), T2, T1 > ::type;
		enum { size = sizeof(type) };
	};

	template<typename T>
	struct MaxType<T> { using type = T; enum { size = sizeof(T) }; };

	/*#####################################################################################*/
	//选择2个类型中存储空间小的一种,例如s8和s16,会选择s8 
	/*#####################################################################################*/
	template<typename T1, typename ... Args>
	struct MinType
	{
		using T2 = typename MinType<Args ...>::type;

		using type = typename SelectType < sizeof(T1) < sizeof(T2), T1, T2 > ::type;
		enum { size = sizeof(type) };
	};

	template<typename T>
	struct MinType<T> { using type = T; enum { size = sizeof(T) }; };

	/*#####################################################################################*/
	//判断某个类是否含有某个函数
	//先用这个宏定义类模板, 再调用具体模板, 使用例子:
	//Def_Class_Has(fun, void, int); //要判断某个类是否定义了函数fun, 返回值为void, 一个int为参数
	//auto v = class_has_fun<A>::value; //判断类A是否定义了函数fun
#define Def_Class_Has(FunName, Return, ...) \
	template <typename T> struct class_has_##FunName{ \
		typedef Return (T::* FunType)(##__VA_ARGS__); \
		template <typename X, X> struct MatchStruct; \
		template <typename S> static int match_test(MatchStruct<FunType, &S::FunName>*); \
		template <typename S> static char match_test(...); \
		const_bool value = sizeof(match_test<T>(0)) == sizeof(int); };

}
#endif//__clan_base_type_traits__ 