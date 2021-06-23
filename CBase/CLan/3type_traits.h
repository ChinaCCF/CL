#ifndef __clan_type_traits__
#define __clan_type_traits__

//对std的<type_traits>补充
#include "2type.h"
#include <type_traits>

namespace clan
{
	/*#####################################################################################*/
	//类似于std::decay, 但是仅对有限的类型操作
	/*#####################################################################################*/
	template<typename T> struct RawType { using type = T; static constexpr bool is_ptr = false; };
	template<typename T> struct RawType<T&> { using type = T; static constexpr bool is_ptr = false; };
	template<typename T> struct RawType<T&&> { using type = T; static constexpr bool is_ptr = false; };
	template<typename T> struct RawType<const T> { using type = T; static constexpr bool is_ptr = false; };
	template<typename T> struct RawType<const T&> { using type = T; static constexpr bool is_ptr = false; };

	template<typename T> struct RawType<T*> { using type = T*; static constexpr bool is_ptr = true; };
	template<typename T> struct RawType<const T*> { using type = T*; static constexpr bool is_ptr = true; };

	template<typename T> struct RawType<T[]> { using type = T*; static constexpr bool is_ptr = true; };
	template<typename T> struct RawType<const T[]> { using type = T*; static constexpr bool is_ptr = true; };
	template<typename T> struct RawType<const T(&)[]> { using type = T*; static constexpr bool is_ptr = true; };

	template<typename T, int N> struct RawType<T[N]> { using type = T*; static constexpr bool is_ptr = true; };
	template<typename T, int N> struct RawType<const T[N]> { using type = T*; static constexpr bool is_ptr = true; };
	template<typename T, int N> struct RawType<const T(&)[N]> { using type = T*; static constexpr bool is_ptr = true; };

	namespace detail
	{
		/*#####################################################################################*/
		/*#####################################################################################*/
		template<typename T> struct _IsChar { static constexpr bool value = false; };
		template<> struct _IsChar<char> { static constexpr bool value = true; };

		template<typename T> struct _IsWChar { static constexpr bool value = false; };
		template<> struct _IsWChar<wchar> { static constexpr bool value = true; };

		/*#####################################################################################*/
		/*#####################################################################################*/

		template<typename T> struct _IsBool { static constexpr bool value = false; };
		template<> struct _IsBool<bool> { static constexpr bool value = true; };
		/*#####################################################################################*/
		/*#####################################################################################*/

		template<typename T> struct _IsFloat { static constexpr bool value = false; };
		template<> struct _IsFloat<f32> { static constexpr bool value = true; };
		template<> struct _IsFloat<f64> { static constexpr bool value = true; };

		/*#####################################################################################*/
		/*#####################################################################################*/

		template<typename T> struct _IsInt { static constexpr bool value = false; };
		template<> struct _IsInt<s8> { static constexpr bool value = true; };
		template<> struct _IsInt<s16> { static constexpr bool value = true; };
		template<> struct _IsInt<s32> { static constexpr bool value = true; };
		template<> struct _IsInt<s64> { static constexpr bool value = true; };
		template<> struct _IsInt<u8> { static constexpr bool value = true; };
		template<> struct _IsInt<u16> { static constexpr bool value = true; };
		template<> struct _IsInt<u32> { static constexpr bool value = true; };
		template<> struct _IsInt<u64> { static constexpr bool value = true; };
	}

	template<typename T> struct IsChar { using raw_t = typename RawType<T>::type; static constexpr bool value = detail::_IsChar<raw_t>::value; };
	template<typename T> struct IsWChar { using raw_t = typename RawType<T>::type; static constexpr bool value = detail::_IsWChar<raw_t>::value; };
	template<typename T> struct IsChars { using raw_t = typename RawType<T>::type; static constexpr bool value = detail::_IsWChar<raw_t>::value || detail::_IsChar<raw_t>::value; };

	template<typename T> struct IsBool { using raw_t = typename RawType<T>::type; static constexpr bool value = detail::_IsBool<raw_t>::value; };
	template<typename T> struct IsFloat { using raw_t = typename RawType<T>::type; static constexpr bool value = detail::_IsFloat<raw_t>::value; };
	template<typename T> struct IsInt { using raw_t = typename RawType<T>::type; static constexpr bool value = detail::_IsInt<raw_t>::value; };

	/*#####################################################################################*/
	//判断是否是数值类型
	/*#####################################################################################*/
	template<typename T> struct IsVal { static constexpr bool value = IsBool<T>::value || IsFloat<T>::value || IsInt<T>::value; };

	/*#####################################################################################*/
	//判断是否是指针
	/*#####################################################################################*/
	template<typename T> struct IsPtr { static constexpr bool value = RawType<T>::is_ptr; };//{ enum { value = std::is_pointer<T>::value }; };

	/*#####################################################################################*/
	//判断是否是相同的类型
	/*#####################################################################################*/
	template<typename T1, typename T2> struct IsSameType { static constexpr bool value = false; };
	template<typename T> struct IsSameType<T, T> { static constexpr bool value = true; };

	/*#####################################################################################*/
	//判断是否是类
	/*#####################################################################################*/
	template<typename T> struct IsClass { static constexpr bool value = std::is_class<T>::value; };

	/*#####################################################################################*/
	//判断是否右值移动赋值
	/*#####################################################################################*/
	template<typename T> struct IsMoveAssignable { static constexpr bool value = std::is_move_assignable<T>::value; };

	/*#####################################################################################*/
	//判断是否右值构造
	/*#####################################################################################*/
	template<typename T> struct IsMoveConstructible { static constexpr bool value = std::is_move_constructible<T>::value; };

	/*#####################################################################################*/
	//判断是否是虚基类, 类中有虚函数的类, std::is_abstract是判断是否声明了纯虚函数
	/*#####################################################################################*/
	template<typename T> struct IsVirtualClass { static constexpr bool value = std::is_polymorphic<T>::value; };
	 
	/*#####################################################################################*/
	//判断虚基类是否有虚析构函数
	/*#####################################################################################*/
	template<typename T> struct HasVirtualDestructor { static constexpr bool value = std::has_virtual_destructor<T>::value; };

	/*#####################################################################################*/
	//检查类
	/*#####################################################################################*/
	//常规类的右值赋值和右值构造检查
	template<typename T> struct NormalClassCheck { static constexpr bool value = IsMoveAssignable<T>::value && IsMoveConstructible<T>::value; };
	template<typename T> struct VirtualClassCheck { static constexpr bool value = IsVirtualClass<T>::value ? HasVirtualDestructor<T>::value : true; };

	template<typename T> struct MoveCheck { static constexpr bool value = IsClass<T>::value ? NormalClassCheck<T>::value : true; };
	template<typename T> struct DestructorCheck { static constexpr bool value = IsClass<T>::value ? VirtualClassCheck<T>::value : true; };

	//static_assert 不能输出中文
#define  clan_CheckClass(T) do{\
static_assert(clan::MoveCheck<T>::value : true, "class need move assign and construct!");\
static_assert(clan::DestructorCheck<T>::value, "virtual class need virtual destructor!");\
}while(false);

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
}
#endif//__clan_type_traits__ 