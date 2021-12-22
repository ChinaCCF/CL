#ifndef __cl_base_type_traits__
#define __cl_base_type_traits__

//��std��<type_traits>���� 
//���� std::is_convertible, ���õ� __is_convertible_to, ����������޷���д�� 
#include <new>
#include <type_traits>
#include "2type.h"

namespace cl
{
#define cl_const_bool  static inline constexpr bool

	template<typename T> struct IsNull { cl_const_bool value = std::is_null_pointer<T>::value; };
	template<> struct IsNull<std::nullptr_t> { cl_const_bool value = true; };
	template<typename T> cl_const_bool IsNull_v = IsNull<T>::value;

	template<typename T> struct IsPtr { cl_const_bool value = std::is_pointer<T>::value; };
	template<typename T> cl_const_bool IsPtr_v = IsPtr<T>::value;

	template<typename T1, typename T2> struct IsSame { cl_const_bool value = std::is_same<T1, T2>::value; };
	template<typename T1, typename T2> cl_const_bool IsSame_v = IsSame<T1, T2>::value;

	//�ж��Ƿ��ǳ�Ա����
	template<typename T> struct IsClassMemFun { cl_const_bool value = std::is_member_function_pointer_v<T>; };
	template<typename T> cl_const_bool IsClassMemFun_v = IsClassMemFun<T>::value;

	//�ж��Ƿ���ֵ����
	template<typename T> struct HasMoveConstruct { cl_const_bool value = std::is_move_constructible_v<T>; };
	template<typename T> cl_const_bool HasMoveConstruct_v = HasMoveConstruct<T>::value;

	//�ж��Ƿ���ֵ�ƶ���ֵ
	template<typename T> struct HasMoveSet { cl_const_bool value = std::is_move_assignable_v<T>; };
	template<typename T> cl_const_bool HasMoveSet_v = HasMoveSet<T>::value;

	//�ж��Ƿ�������, �������麯��(�����Ǽ̳л����Լ�����), std::is_abstract���ж��Ƿ������˴��麯��
	template<typename T> struct IsVirtualClass { cl_const_bool value = std::is_polymorphic_v<T>; };
	template<typename T> cl_const_bool IsVirtualClass_v = IsVirtualClass<T>::value;

	//ѡ������, if Test == true, type == Type1, else type == Type2 
	template<bool Test, typename Type1, typename Type2> struct SelectType
	{ using type = typename std::conditional<Test, Type1, Type2>::type; };
	template<bool Test, typename Type1, typename Type2> using SelectType_t = typename SelectType<Test, Type1, Type2>::type;

	//�ж��ܷ�ת����ָ������
	template<typename Src, typename Dst> struct _To { cl_const_bool value = std::is_convertible_v<Src, Dst>; };
	template<typename Src, typename Dst> cl_const_bool _To_v = _To<Src, Dst>::value;

	//std::remove_all_extents �ǻ�ȡ�����Ԫ������
	//std::decay �и�ë��, const ch8 * �������˻�, �Ȿ���߼���û��
	//���� ch8* �� const ch8* ������ȫƥ��, ����һЩ�ϸ�ƥ���龰�³���
	namespace lib
	{
		template<typename T> struct RemoveConst { using type = T; };
		template<typename T> struct RemoveConst<const T> { using type = T; };
		template<typename T> struct RemoveConst<const T*> { using type = T*; };
	}
	template<typename T> struct RawType { using type = lib::RemoveConst<typename std::decay<T>::type>::type; };
	template<typename T> using RawType_t = typename RawType<T>::type;

	namespace lib
	{
		template<typename T> struct _IsBool { cl_const_bool value = false; };
		template<> struct _IsBool<bool> { cl_const_bool value = true; };
		template<typename T> cl_const_bool _IsBool_v = _IsBool<T>::value;
		/*######################################################################################*/
		template<typename T> struct _IsC8 { cl_const_bool value = false; };
		template<> struct _IsC8<uc8> { cl_const_bool value = true; };
		template<> struct _IsC8<ac8> { cl_const_bool value = true; };
		template<typename T> cl_const_bool _IsC8_v = _IsC8<T>::value;
		/*######################################################################################*/
		template<typename T> struct _IsC16 { cl_const_bool value = false; };
		template<> struct _IsC16<uc16> { cl_const_bool value = true; };
		template<> struct _IsC16<wchar_t> { cl_const_bool value = true; };
		template<typename T> cl_const_bool _IsC16_v = _IsC16<T>::value;
		/*######################################################################################*/
		template<typename T> struct _IsUint { cl_const_bool value = false; };
		template<> struct _IsUint<uv8> { cl_const_bool value = true; };
		template<> struct _IsUint<uv16> { cl_const_bool value = true; };
		template<> struct _IsUint<uv32> { cl_const_bool value = true; };
		template<> struct _IsUint<uv64> { cl_const_bool value = true; };
		template<typename T> cl_const_bool _IsUint_v = _IsUint<T>::value;
		/*######################################################################################*/
		template<typename T> struct _IsSint { cl_const_bool value = false; };
		template<> struct _IsSint<sv8> { cl_const_bool value = true; };
		template<> struct _IsSint<sv16> { cl_const_bool value = true; };
		template<> struct _IsSint<sv32> { cl_const_bool value = true; };
		template<> struct _IsSint<sv64> { cl_const_bool value = true; };
		template<typename T> cl_const_bool _IsSint_v = _IsSint<T>::value;
		/*######################################################################################*/
		template<typename T> struct _IsFloat { cl_const_bool value = false; };
		template<> struct _IsFloat<fv32> { cl_const_bool value = true; };
		template<> struct _IsFloat<fv64> { cl_const_bool value = true; };
		template<typename T> cl_const_bool _IsFloat_v = _IsFloat<T>::value;
	}

	template<typename T> struct IsC8 { cl_const_bool value = lib::_IsC8_v<RawType_t<T>>; };
	template<typename T> struct IsC16 { cl_const_bool value = lib::_IsC16_v<RawType_t<T>>; };
	template<typename T> struct IsChar { cl_const_bool value = IsC8<T>::value || IsC16<T>::value; };

	template<typename T> cl_const_bool IsC8_v = IsC8<T>::value;
	template<typename T> cl_const_bool IsC16_v = IsC16<T>::value;
	template<typename T> cl_const_bool IsChar_v = IsChar<T>::value;

	template<typename T> struct IsC8Ptr { cl_const_bool value = IsSame_v<RawType_t<T>, uc8*> || IsSame_v<RawType_t<T>, ac8*>; };
	template<typename T> struct IsC16Ptr { cl_const_bool value = IsSame_v<RawType_t<T>, uc16*>; };
	template<typename T> struct IsCharPtr { cl_const_bool value = IsC8Ptr<T>::value || IsC16Ptr<T>::value; };

	template<typename T> cl_const_bool IsC8Ptr_v = IsC8Ptr<T>::value;
	template<typename T> cl_const_bool IsC16Ptr_v = IsC16Ptr<T>::value;
	template<typename T> cl_const_bool IsCharPtr_v = IsC16Ptr<T>::value;

	template<typename T> struct IsBool { cl_const_bool value = lib::_IsBool_v<RawType_t<T>>; };
	template<typename T> struct IsFloat { cl_const_bool value = lib::_IsFloat_v<RawType_t<T>>; };
	template<typename T> struct IsUint { cl_const_bool value = lib::_IsUint_v<RawType_t<T>>; };
	template<typename T> struct IsSint { cl_const_bool value = lib::_IsSint_v<RawType_t<T>>; };
	template<typename T> struct IsInt { cl_const_bool value = IsUint<T>::value || IsSint<T>::value; };

	template<typename T> cl_const_bool IsBool_v = IsBool<T>::value;
	template<typename T> cl_const_bool IsFloat_v = IsFloat<T>::value;
	template<typename T> cl_const_bool IsUint_v = IsUint<T>::value;
	template<typename T> cl_const_bool IsSint_v = IsSint<T>::value;
	template<typename T> cl_const_bool IsInt_v = IsInt<T>::value;

	/*#####################################################################################*/
	namespace lib
	{
		//��ָ���ܹ�ת�����ַ���ָ�� ����Ҳ���ܹ�ת��Ϊ�ַ�ָ��
		template<typename T, typename CharT> struct _2CxxPtr
		{
			cl_const_bool value = !IsNull_v<T> && !IsInt_v<T> && (_To_v<T, CharT*> || _To_v<T, const CharT*>);
		};
		template<typename T, typename CharT> cl_const_bool _2CxxPtr_v = _2CxxPtr<T, CharT>::value;
	}

	template<typename T> struct ToC8Ptr { cl_const_bool value = lib::_2CxxPtr_v<RawType_t<T>, uc8> || lib::_2CxxPtr_v<RawType_t<T>, ac8>; };
	template<typename T> struct ToC16Ptr { cl_const_bool value = lib::_2CxxPtr_v<RawType_t<T>, uc16>; }; 
	template<typename T> cl_const_bool ToC8Ptr_v = ToC8Ptr<T>::value;
	template<typename T> cl_const_bool ToC16Ptr_v = ToC16Ptr<T>::value;

	template<typename T> concept C8Type = IsC8_v<T>;
	template<typename T> concept C16Type = IsC16_v<T>;
	template<typename T> concept CharType = IsChar_v<T>;

	template<typename T> concept C8PtrType = ToC8Ptr_v<T>;
	template<typename T> concept C16PtrType = ToC16Ptr_v<T>;

	template<typename T> concept BoolType = IsBool_v<T>;
	template<typename T> concept FloatType = IsFloat_v<T>;
	template<typename T> concept UintType = IsUint_v<T>;
	template<typename T> concept SintType = IsSint_v<T>;
	template<typename T> concept IntType = IsInt_v<T>;
	 
	template<typename T> concept ClassMemFunType = IsClassMemFun_v<T>;
	template<typename T> concept PtrType = IsPtr_v<T>;

	/*#####################################################################################*/
	//���ݴ��ݽ����������ܷ�תΪ��ͬ���ַ���,����ѡ���ַ�����
	/*#####################################################################################*/
	template<typename T, typename DefaultC8Type = uc8> struct GetCharType
	{
		using type = typename SelectType<ToC16Ptr_v<T>, uc16, DefaultC8Type>::type;
	};
	template<typename T, typename DefaultC8Type = uc8> using GetCharType_t = typename GetCharType<T, DefaultC8Type>::type;
	 
	/*#####################################################################################*/
	//ѡ����ͬ��С������
	/*#####################################################################################*/
	template<typename Org, typename ChoiceType, typename ... Args>
	struct SameSizeType
	{
		using type = typename SelectType < sizeof(Org) == sizeof(ChoiceType), ChoiceType, typename SameSizeType<Org, Args ...>::type>::type;
	};
	template<typename Org, typename ChoiceType>
	struct SameSizeType<Org, ChoiceType>
	{
		using type = typename SelectType < sizeof(Org) == sizeof(ChoiceType), ChoiceType, void > ::type;
	};
	template<typename Org, typename ChoiceType, typename ... Args> using SameSizeType_t = typename SameSizeType<Org, ChoiceType, Args ...>::type;
	/*#####################################################################################*/
	//ѡ��2�������д洢�ռ���һ��,����sv8��sv16,��ѡ��sv16 
	/*#####################################################################################*/
	template<typename T1, typename ... Args>
	struct MaxType
	{
		using T2 = typename MaxType<Args ...>::type;

		using type = typename SelectType < sizeof(T1) < sizeof(T2), T2, T1 > ::type;
		enum { size = sizeof(type) };
	};
	template<typename T> struct MaxType<T> { using type = T; enum { size = sizeof(T) }; };
	template<typename T1, typename ... Args> using MaxType_t = typename MaxType<T1, Args ...>::type;
	/*#####################################################################################*/
	//ѡ��2�������д洢�ռ�С��һ��,����sv8��sv16,��ѡ��sv8 
	/*#####################################################################################*/
	template<typename T1, typename ... Args>
	struct MinType
	{
		using T2 = typename MinType<Args ...>::type;

		using type = typename SelectType < sizeof(T1) < sizeof(T2), T1, T2 > ::type;
		enum { size = sizeof(type) };
	};
	template<typename T> struct MinType<T> { using type = T; enum { size = sizeof(T) }; };
	template<typename T1, typename ... Args> using MinType_t = typename MinType<T1, Args ...>::type;
	/*#####################################################################################*/
	//check class, �ж����Ƿ����Ԥ�ڶ���
	/*#####################################################################################*/
	namespace lib
	{
		template<typename T> struct _CheckClassMove { cl_const_bool value = HasMoveSet<T>::value && HasMoveConstruct<T>::value; };
		template<typename T> struct _CheckVirtualClass { cl_const_bool value = IsVirtualClass<T>::value ? std::has_virtual_destructor<T>::value : true; };
	}
	//������Ƿ������ƶ�����
	template<typename T> struct CheckClassMove { cl_const_bool value = std::is_class<T>::value ? lib::_CheckClassMove<T>::value : true; };
	//�������Ƿ���������������
	template<typename T> struct CheckVirtualClass { cl_const_bool value = std::is_class<T>::value ? lib::_CheckVirtualClass<T>::value : true; };

	//�����Ķ�������,�Ƿ������ƶ�����, �����Ƿ���������������
#define CL_Check_Class_Def(T) \
static_assert(cl::CheckClassMove<T>::value, "class need move set and construct!");\
static_assert(cl::CheckVirtualClass<T>::value, "virtual class need virtual destructor!");
	/*#####################################################################################*/
	//�ж�ĳ�����Ƿ���ĳ������
	/*#####################################################################################*/
	//��������궨����ģ��, �ٵ��þ���ģ��, ʹ������:
	//CL_Check_Class_Fun(fun, void, int); //Ҫ�ж�ĳ�����Ƿ����˺���fun, ����ֵΪvoid, һ��intΪ����
	//auto v = cl::Check_Class_fun<A>::value; //�ж���A�Ƿ����˺���fun

#define CL_Check_Class_Fun(Name, Return, ...) namespace cl{\
	template <typename Obj> class Check_Class_##Name{\
		template <typename T, Return(T::*)(__VA_ARGS__) = &T::Name>\
		cl_const_bool _check(T*) { return true; };\
		cl_const_bool _check(...) { return false; };\
	public: cl_const_bool value = _check((Obj*)0); };}
	  
}
#endif//__cl_base_type_traits__ 