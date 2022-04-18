#pragma once
#ifndef __dll_dism__
#define __dll_dism__

#include <libCL/base/1cfg.h>

#if CL_Compiler == CL_Compiler_VC

#	ifndef Dism_DLL
#		if CL_Version == CL_Version_Debug  
#			pragma comment(lib, "DismD.lib")
#		else
#			pragma comment(lib, "Dism.lib")
#		endif 
#	endif

#	ifdef Dism_DLL
#		define dism_api __declspec(dllexport)
#	else
#		define dism_api __declspec(dllimport)
#	endif  

#endif

#include <libCL/base/2type.h>
#include <libCL/base/9str.h>  
#include <libWL/base/1type.h>
#include <libCL/base/7call.h>
#include <libCL/set/3list.h>

//DLL不要暴露对象的构造和析构函数, 否则可能会导致
//_ASSERTE(__acrt_first_block == header);
namespace cl
{
#pragma warning(disable : 4251) 
	typedef UINT DismSession;
	   
	struct ItemInfo
	{
		StringW name_;
		StringW display_name_;
		StringW description_;
	};

	struct PackageInfo
	{
		StringW name_;
		StringW display_name_;
		StringW description_;
		List<StringW> features_;
	};

	dism_api void _WIMCloseHandle(HANDLE);
	dism_api void _DismDelete(HANDLE);

	template<typename> struct dism_api Fun_WIMCloseHandle { void operator()(HANDLE ptr) { _WIMCloseHandle(ptr); } };
	template<typename> struct dism_api Fun_DismDelete { void operator()(HANDLE ptr) { _DismDelete(ptr); } };
	//struct dism_api Fun_DismCloseSession { void operator()(DismSession ptr); };

	class dism_api Wim
	{
		Ptr<HANDLE, Fun_WIMCloseHandle> wim_;
		StringW wim_path_;
		StringW tmp_path_;
	public:
		static Wim open(const StrViewW& wim_path);

		Wim(HANDLE wim) : wim_(wim) {}
		Wim(Wim&& wim) noexcept :
			wim_(std::move(wim.wim_)),
			wim_path_(std::move(wim.wim_path_)),
			tmp_path_(std::move(wim.tmp_path_))
		{

		}

		operator HANDLE () { return wim_; }

		bool valid() { return wim_.valid(); }

		bool set_tmp_dir(const wchar* tmp);
		bool del_sub(uv32 index);
		bool rebuild(const StrViewW& dst, const wchar* tmp, int* arr_index, int cnt);

		void print_info();
	};

	class dism_api Dism
	{
		StringW mount_path_;
		DismSession session_ = 0;
	public:
		static bool init(const wchar* log_path, const wchar* tmp);
		static void uninit();

		static bool mount(const wchar* wim_path,
						  const wchar* mount_path,
						  int index);
		static void unmount(const wchar* mount_path);

		static Dism open(const wchar* mount_path);

		Dism() {}
		Dism(Dism&& wim) noexcept :
			session_(std::move(wim.session_)),
			mount_path_(std::move(wim.mount_path_))
		{
			wim.session_ = 0;
		}
		~Dism();

		bool valid() { return session_; }

		void enum_Capabilities(const Call<void(ItemInfo* info)>& call);
		void remove_Capability(const wchar* name);
		void enum_Features(const Call<void(ItemInfo* info)>& call);
		void remove_Feature(const wchar* name);
		void enum_Packages(const Call<void(PackageInfo* info)>& call);
		void remove_Package(const wchar* name);

		void enum_app(const Call<void(ItemInfo* info)>& call);
		void remove_app(const wchar* name);
		void commit();
	};
}

#endif//__dll_dism__