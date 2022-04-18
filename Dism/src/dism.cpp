#include "../dism.h"
#include <Windows.h>
#include <dismapi.h>
#include <libCL/io/3print.h>
#include <libCL/set/2arr.h>

namespace cl
{
	bool Dism::init(const wchar* log_path, const wchar* tmp)
	{
		static bool has_init = false;
		static bool last_ret = false;

		if (has_init) return last_ret;
		has_init = true;

		auto ret = DismInitialize(DismLogErrorsWarningsInfo, log_path, tmp);
		if (ret != S_OK)
		{
			if (ret == HRESULT_FROM_WIN32(ERROR_ELEVATION_REQUIRED))
				CPrint << "Use Admin account!";
			else
				CPrint << "unknow error " << GetLastError();
			return false;
		}
		else
			CPrint << "DismInitialize success!";
		last_ret = true;
		return true;
	}
	void Dism::uninit()
	{
		if (S_OK == DismShutdown())
			CPrint << "DismShutdown success!";
		else
			CPrint << "DismShutdown fail!";
	}

	bool Dism::mount(const wchar* wim_path,
					 const wchar* mount_path,
					 int index)
	{
		CPrint << "Mounting...";
		auto ret = DismMountImage(wim_path,
								  mount_path,
								  index,
								  nullptr,//name
								  DismImageIndex,
								  DISM_MOUNT_READWRITE,
								  nullptr,//cancel
								  nullptr,//call back
								  nullptr);//user data

		if (ret == S_OK)
		{
			CPrint << "DismMountImage success!";
			return true;
		}

		if (ret == 122)
		{
			ret = DismRemountImage(mount_path);
			if (ret == S_OK)
			{
				CPrint << "DismRemountImage success";
				return true;
			}
			else
				CPrint << "DismRemountImage fail " << GetLastError();
		}
		else
			CPrint << "DismMountImage fail :" << GetLastError();

		return false;
	}
	void Dism::unmount(const wchar* mount_path)
	{
		CPrint << "unMounting ...";
		if (S_OK != DismUnmountImage(mount_path,
									 DISM_DISCARD_IMAGE,
									 nullptr,//cancel
									 nullptr,//cb
									 nullptr))//user data
		{
			CPrint << "DismUnmountImage fail :" << GetLastError();
			return;
		}
		if (S_OK != DismCleanupMountpoints())
		{
			CPrint << "DismCleanupMountpoints fail :" << GetLastError();
			return;
		}
		CPrint << "unMounting and clean success!";
	}

	Dism Dism::open(const wchar* mount_path)
	{
		Dism dism;

		dism.mount_path_ = mount_path;
		auto ret = DismOpenSession(mount_path, nullptr, nullptr, &dism.session_);

		if (ret == S_OK)
			CPrint << "DismOpenSession success";
		else
		{
			if (ret == 122)
			{
				ret = DismRemountImage(mount_path);
				if (ret == S_OK)
					CPrint << "DismRemountImage success";
				else
					CPrint << "DismRemountImage fail " << GetLastError();
			}
			else
				CPrint << "DismOpenSession fail :" << GetLastError();
		}

		return dism;
	}

	Dism::~Dism()
	{
		if (session_ != 0)
		{
			DismSetReservedStorageState(session_, DISM_RESERVED_STORAGE_DISABLED);
			DismCloseSession(session_);
		}
	}

	const wchar* DismPackageFeatureState_to_str(::DismPackageFeatureState state)
	{
		switch (state)
		{
		case DismStateNotPresent: return L"DismStateNotPresent 不存在";
		case DismStateUninstallPending: return L"DismStateUninstallPending 卸载挂起";
		case DismStateStaged: return L"DismStateStaged 已暂存";
		case DismStateRemoved: return L"DismStateRemoved 不存在";
		case DismStateInstalled: return L"DismStateInstalled 已安装";
		case DismStateInstallPending: return L"DismStateInstallPending 安装挂起";
		case DismStateSuperseded: return L"DismStateSuperseded 已被较新的包或功能取代";
		case DismStatePartiallyInstalled: return L"DismStatePartiallyInstalled 已部分安装";
		default:
			break;
		}
		return L"";
	}

	//{
//	cl::Print() << ""; 
//	DismDriverPackage* arr = nullptr;
//	UINT cnt = 0;
//	if (S_OK == DismGetDrivers(session_,
//							   true,//all driver
//							   &arr, &cnt))
//	{
//		CPrint << "DismGetDrivers success";
//		for (int i = 0; i < cnt; i++)
//		{
//			cl::Print() << arr[i].PublishedName;
//			//DismGetCapabilityInfo
//		}
//	}
//	else
//		CPrint << "DismGetDrivers fail " << GetLastError(); 
//	DismDelete(arr);
//}

	bool state_valid(::DismPackageFeatureState state)
	{
		if (state == DismStateNotPresent) return false;
		return true;
	}

	void Dism::enum_Capabilities(const Call<void(ItemInfo*)>& call)
	{
		DismCapability* arr = nullptr;
		UINT cnt = 0;
		if (S_OK != DismGetCapabilities(session_, &arr, &cnt))
		{
			CPrint << "DismGetCapabilities fail " << GetLastError();
			return;
		}
		 
		int valid_cnt = 0;
		ItemInfo tmp;
		for (uv32 i = 0; i < cnt; i++)
		{
			DismCapabilityInfo* info;
			if (S_OK != DismGetCapabilityInfo(session_, arr[i].Name, &info))
			{
				cl::Print() << "Capability\n" << "	Name : " << arr[i].Name;
				continue;
			}

			if (state_valid(info->State))
			{
				valid_cnt++;
				cl::Print() << "Capability\n"
					<< "    Name : " << info->Name << '\n'
					<< "    DisplayName : " << info->DisplayName << '\n'
					<< "    Description : " << info->Description << '\n'
					<< "    State : " << DismPackageFeatureState_to_str(info->State) << '\n';

				tmp.name_ = info->Name;
				tmp.display_name_ = info->DisplayName;
				tmp.description_ = info->Description;
				call(&tmp);
			}
			DismDelete(info);
		}
		cl::Print() << "Capability Valid Count : " << valid_cnt;
		DismDelete(arr);
	} 
	void Dism::remove_Capability(const wchar* name)
	{
		cl::Print() << "remove_Capability : " << name;
		auto ret = DismRemoveCapability(session_, name, nullptr, nullptr, nullptr);
		if (ret != S_OK)
			CPrint << "DismRemoveCapability fail " << GetLastError();
	}

	void Dism::enum_Features(const Call<void(ItemInfo* info)>& call)
	{
		DismFeature* arr = nullptr;
		UINT cnt = 0;
		if (S_OK != DismGetFeatures(session_, nullptr, DismPackageNone, &arr, &cnt))
		{
			CPrint << "DismGetFeatures fail " << GetLastError();
			return;
		}
		 
		ItemInfo tmp;  
		int valid_cnt = 0;
		for (uv32 i = 0; i < cnt; i++)
		{
			::DismFeatureInfo* info;
			if (S_OK != DismGetFeatureInfo(session_, arr[i].FeatureName, nullptr, DismPackageNone, &info))
			{
				cl::Print() << "Feature\n" << arr[i].FeatureName;
				continue;
			}
			 
			if (state_valid(info->FeatureState))
			{
				valid_cnt++;
				cl::Print() << "Feature\n"
					<< "    Name :" << info->FeatureName << '\n'
					<< "    DisplayName : " << info->DisplayName << '\n'
					<< "    Description : " << info->Description << '\n'
					<< "    State : " << DismPackageFeatureState_to_str(info->FeatureState);
				  
				tmp.name_ = info->FeatureName;
				tmp.display_name_ = info->DisplayName;
				tmp.description_ = info->Description; 
				call(&tmp);

				DismFeature* parents;
				UINT pcnt = 0;
				if (S_OK == DismGetFeatureParent(session_, arr[i].FeatureName, nullptr, DismPackageNone, &parents, &pcnt))
				{
					if (pcnt >= 1)
					{
						cl::Print() << "    Parent Feature:";
						for (uv32 j = 0; j < pcnt; j++)
							cl::Print() << "        " << parents[j].FeatureName;
					}

					DismDelete(parents);
				}
			} 
			DismDelete(info);
		}

		cl::Print() << "Features Valid Count : " << valid_cnt;
		DismDelete(arr);
	}
	void Dism::remove_Feature(const wchar* name)
	{
		cl::Print() << "remove_Feature : " << name;
		auto ret = DismDisableFeature(session_, name, nullptr, true, nullptr, nullptr, nullptr);
		if (ret != S_OK)
			CPrint << "DismDisableFeature fail " << GetLastError();
	}

	void Dism::enum_Packages(const Call<void(PackageInfo*)>& call)
	{
		DismPackage* arr = nullptr;
		UINT cnt = 0;
		if (S_OK != DismGetPackages(session_, &arr, &cnt))
		{
			CPrint << "DismGetPackages fail " << GetLastError();
			return;
		}
		PackageInfo tmp;
		for (uv32 i = 0; i < cnt; i++)
		{
			DismPackageInfo* info;
			if (S_OK != DismGetPackageInfo(session_, arr[i].PackageName, DismPackageName, &info))
				continue;

			cl::Print() << "Package\n"
				<< "    Name : " << info->PackageName << '\n'
				<< "    DisplayName : " << info->DisplayName << '\n'
				<< "    Description : " << info->Description << '\n'
				<< "    State : " << DismPackageFeatureState_to_str(info->PackageState);
				 
			tmp.features_.clear();
			if (info->FeatureCount >= 1)
			{
				cl::Print() << "    Include Feature :";
				for (uv32 j = 0; j < info->FeatureCount; j++)
				{
					cl::Print() << "        " << info->Feature[j].FeatureName;
					tmp.features_.push_back(info->Feature[j].FeatureName);
				}
			}

			tmp.name_ = info->PackageName;
			tmp.display_name_ = info->DisplayName;
			tmp.description_ = info->Description;
			call(&tmp);
			DismDelete(info);
		}

		DismDelete(arr); 
	}
	void Dism::remove_Package(const wchar* name)
	{
		cl::Print() << "remove_Package : " << name;
		auto ret = DismRemovePackage(session_, name, DismPackageName, nullptr, nullptr, nullptr);
		if (ret != S_OK)
			CPrint << "DismRemovePackage fail " << GetLastError();
	}

	void Dism::enum_app(const Call<void(ItemInfo*)>& call)
	{
		DismAppxPackage* arr;
		UINT cnt;
		if (S_OK != _DismGetProvisionedAppxPackages(session_, &arr, &cnt))
		{
			CPrint << "_DismGetProvisionedAppxPackages fail " << GetLastError();
			return;
		}
		ItemInfo tmp;
		for (uv32 i = 0; i < cnt; i++)
		{  
			auto info = arr + i;
			cl::Print() << "App\n"
				<< "    Name : " << info->PackageName << '\n'
				<< "    DisplayName : " << info->DisplayName;

			tmp.name_ = info->PackageName;
			tmp.display_name_ = info->DisplayName;
			tmp.description_ = "";
			call(&tmp); 
		}
		cl::Print() << "App Count : " << cnt;

		DismDelete(arr);
	}
	void Dism::remove_app(const wchar* name)
	{
		if (name == nullptr || *name == 0) return;
		cl::Print() << "remove_app : " << name;
		auto ret = _DismRemoveProvisionedAppxPackage(session_, name);
		if (ret != S_OK)
			CPrint << "_DismRemoveProvisionedAppxPackage fail " << GetLastError();
	}

	void Dism::commit()
	{
		auto ret = DismCommitImage(session_,
								   DISM_COMMIT_IMAGE,
								   nullptr,
								   nullptr,
								   nullptr);
		if (ret != S_OK)
			CPrint << "DismCommitImage fail " << GetLastError();
		else
			CPrint << "DismCommitImage success ";
	}
}