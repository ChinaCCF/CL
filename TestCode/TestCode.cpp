// TestCode.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "lib_code.h"

#include <cstdio>
#include <cwchar>

#include <string>
#include <type_traits>
#include <utility>

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP | WINAPI_PARTITION_SYSTEM)
#include <Userenv.h>
#pragma comment(lib, "Userenv.lib")
#endif

const std::wstring g_NSudoLogSplitter = L"****************************************************************\r\n";
static Mile::CriticalSection g_NSudoLogLock;
static std::wstring g_NSudoLog = g_NSudoLogSplitter;

EXTERN_C LPCWSTR WINAPI NSudoReadLog()
{
	Mile::AutoCriticalSectionLock Lock(g_NSudoLogLock);

	return g_NSudoLog.c_str();
}

EXTERN_C VOID WINAPI NSudoWriteLog(
	_In_ LPCWSTR Sender,
	_In_ LPCWSTR Content)
{
	Mile::AutoCriticalSectionLock Lock(g_NSudoLogLock);

	SYSTEMTIME SystemTime = { 0 };
	::GetLocalTime(&SystemTime);

	g_NSudoLog += Mile::FormatUtf16String(
		L"\r\n"
		L"Sender: %s\r\n"
		L"DateTime: %d-%.2d-%.2d %.2d:%.2d:%.2d\r\n"
		L"Process ID: %d\r\n"
		L"Thread ID: %d\r\n"
		L"\r\n"
		L"%s\r\n"
		L"\r\n",
		Sender,
		SystemTime.wYear,
		SystemTime.wMonth,
		SystemTime.wDay,
		SystemTime.wHour,
		SystemTime.wMinute,
		SystemTime.wSecond,
		::GetCurrentProcessId(),
		::GetCurrentThreadId(),
		Content);
	g_NSudoLog += g_NSudoLogSplitter;
}

EXTERN_C HRESULT WINAPI create_proc(
	_In_ ProcUserType UserModeType,
	_In_ NSUDO_PRIVILEGES_MODE_TYPE PrivilegesModeType,
	_In_ ProcessMandatory _mandatory,
	_In_ ProcessPriorityLevel _priority,
	_In_ WindowShowType _show_type,
	_In_ DWORD WaitInterval,
	_In_ BOOL CreateNewConsole,
	_In_ LPCWSTR CommandLine,
	_In_opt_ LPCWSTR CurrentDirectory)
{

	DWORD mandatory;
	switch (_mandatory)
	{
	case ProcessMandatory::Untrusted:
		mandatory = SECURITY_MANDATORY_UNTRUSTED_RID;
		break;
	case ProcessMandatory::Low:
		mandatory = SECURITY_MANDATORY_LOW_RID;
		break;
	case ProcessMandatory::Medium:
		mandatory = SECURITY_MANDATORY_MEDIUM_RID;
		break;
	case ProcessMandatory::MediumAbove:
		mandatory = SECURITY_MANDATORY_MEDIUM_PLUS_RID;
		break;
	case ProcessMandatory::High:
		mandatory = SECURITY_MANDATORY_HIGH_RID;
		break;
	case ProcessMandatory::System:
		mandatory = SECURITY_MANDATORY_SYSTEM_RID;
		break;
	case ProcessMandatory::Protected:
		mandatory = SECURITY_MANDATORY_PROTECTED_PROCESS_RID;
		break;
	default:
		return E_INVALIDARG;
	}

	DWORD priority;
	switch (_priority)
	{
	case ProcessPriorityLevel::Idle:
		priority = IDLE_PRIORITY_CLASS;
		break;
	case ProcessPriorityLevel::Low:
		priority = BELOW_NORMAL_PRIORITY_CLASS;
		break;
	case ProcessPriorityLevel::Medium:
		priority = NORMAL_PRIORITY_CLASS;
		break;
	case ProcessPriorityLevel::MediumAbove:
		priority = ABOVE_NORMAL_PRIORITY_CLASS;
		break;
	case ProcessPriorityLevel::High:
		priority = HIGH_PRIORITY_CLASS;
		break;
	case ProcessPriorityLevel::RealTime:
		priority = REALTIME_PRIORITY_CLASS;
		break;
	default:
		return E_INVALIDARG;
	}

	DWORD show_type;
	switch (_show_type)
	{
	case WindowShowType::Default:
		show_type = SW_SHOWDEFAULT;
		break;
	case WindowShowType::Show:
		show_type = SW_SHOW;
		break;
	case WindowShowType::Hide:
		show_type = SW_HIDE;
		break;
	case WindowShowType::Maximize:
		show_type = SW_MAXIMIZE;
		break;
	case WindowShowType::Minimize:
		show_type = SW_MINIMIZE;
		break;
	default:
		return E_INVALIDARG;
	}

	HRESULT hr = S_OK;

	DWORD SessionID = static_cast<DWORD>(-1);

	HANDLE cur_proc_token = INVALID_HANDLE_VALUE;
	HANDLE dup_cur_proc_token = INVALID_HANDLE_VALUE;

	HANDLE sys_token = INVALID_HANDLE_VALUE;
	HANDLE dup_sys_token = INVALID_HANDLE_VALUE;

	HANDLE hToken = INVALID_HANDLE_VALUE;
	HANDLE OriginalToken = INVALID_HANDLE_VALUE;

	/*auto _clean = Mile::ScopeExitTaskHandler([&]()
	{
		if (cur_proc_token != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(cur_proc_token);
		}

		if (dup_cur_proc_token != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(dup_cur_proc_token);
		}

		if (sys_token != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(sys_token);
		}

		if (dup_sys_token != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(dup_sys_token);
		}

		if (hToken != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(hToken);
		}

		if (OriginalToken != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(OriginalToken);
		}

		::SetThreadToken(nullptr, nullptr);
	});*/

	hr = Mile::ApiResult(::OpenProcessToken(::GetCurrentProcess(), MAXIMUM_ALLOWED, &cur_proc_token));
	if (hr != S_OK)
		return hr;

	hr = Mile::ApiResult(::DuplicateTokenEx(cur_proc_token,
											MAXIMUM_ALLOWED,
											nullptr,
											SecurityImpersonation,
											TokenImpersonation,
											&dup_cur_proc_token));

	if (hr != S_OK)
	{
		return hr;
	}

	{
		LUID_AND_ATTRIBUTES tmp_privilege;
		hr = Mile::ApiResult(::LookupPrivilegeValueW(nullptr, SE_DEBUG_NAME, &tmp_privilege.Luid));
		if (hr != S_OK)
			return hr;

		tmp_privilege.Attributes = SE_PRIVILEGE_ENABLED;

		hr = Mile::modify_token_privileges(dup_cur_proc_token, &tmp_privilege, 1);
		if (hr != S_OK)
			return hr;
	}


	hr = Mile::ApiResult(::SetThreadToken(nullptr, dup_cur_proc_token));
	if (hr != S_OK)
		return hr;

	SessionID = Mile::get_active_session_id();
	if (SessionID == static_cast<DWORD>(-1))
		return Mile::HResult::FromWin32(ERROR_NO_TOKEN);

	hr = Mile::create_system_token(MAXIMUM_ALLOWED, &sys_token);
	if (hr != S_OK)
		return hr;

	hr = Mile::ApiResult(::DuplicateTokenEx(sys_token,
											MAXIMUM_ALLOWED,
											nullptr,
											SecurityImpersonation,
											TokenImpersonation,
											&dup_sys_token));
	if (hr != S_OK)
		return hr;

	hr = Mile::set_token_all_privilege(dup_sys_token,
									   SE_PRIVILEGE_ENABLED);
	if (hr != S_OK)
		return hr;

	hr = Mile::ApiResult(::SetThreadToken(nullptr, dup_sys_token));
	if (hr != S_OK)
		return hr;

	if (ProcUserType::System_Trusted_Installer == UserModeType)
	{
		hr = Mile::create_token_for_service(L"TrustedInstaller",
											MAXIMUM_ALLOWED,
											&OriginalToken);
		if (hr != S_OK)
			return hr;
	}
	else if (ProcUserType::System == UserModeType)
	{
		hr = Mile::create_system_token(MAXIMUM_ALLOWED, &OriginalToken);
		if (hr != S_OK)
			return hr;
	}
	else if (ProcUserType::Current_User == UserModeType)
	{
		hr = Mile::create_token_for_current_user(SessionID, &OriginalToken);
		if (hr != S_OK)
			return hr;
	}
	else if (ProcUserType::Current_Proc == UserModeType)
	{
		hr = Mile::ApiResult(::OpenProcessToken(::GetCurrentProcess(), MAXIMUM_ALLOWED, &OriginalToken));
		if (hr != S_OK)
			return hr;
	}
	else if (ProcUserType::Current_Proc_Drop_Right == UserModeType)
	{
		HANDLE hCurrentProcessToken = nullptr;
		hr = Mile::ApiResult(::OpenProcessToken(::GetCurrentProcess(), MAXIMUM_ALLOWED, &hCurrentProcessToken));
		if (hr == S_OK)
		{
			hr = Mile::create_lua_token(hCurrentProcessToken, &OriginalToken);

			::CloseHandle(hCurrentProcessToken);
		}

		if (hr != S_OK)
			return hr;
	}
	else if (ProcUserType::Current_User_Admin == UserModeType)
	{
		HANDLE hCurrentProcessToken = nullptr;
		hr = Mile::create_token_for_current_user(SessionID, &hCurrentProcessToken);
		if (hr == S_OK)
		{
			TOKEN_LINKED_TOKEN LinkedToken = { 0 };
			DWORD ReturnLength = 0;

			hr = Mile::ApiResult(::GetTokenInformation(hCurrentProcessToken,
													   TokenLinkedToken,
													   &LinkedToken,
													   sizeof(TOKEN_LINKED_TOKEN),
													   &ReturnLength));
			if (hr == S_OK)
			{
				hr = Mile::ApiResult(::DuplicateTokenEx(LinkedToken.LinkedToken,
														MAXIMUM_ALLOWED,
														nullptr,
														SecurityIdentification,
														TokenPrimary,
														&OriginalToken));

				::CloseHandle(LinkedToken.LinkedToken);
			}

			::CloseHandle(hCurrentProcessToken);
		}

		if (hr != S_OK)
			return hr;
	}
	else
		return E_INVALIDARG;

	hr = Mile::ApiResult(::DuplicateTokenEx(OriginalToken,
											MAXIMUM_ALLOWED,
											nullptr,
											SecurityIdentification,
											TokenPrimary,
											&hToken));
	if (hr != S_OK)
		return hr;

	hr = Mile::ApiResult(::SetTokenInformation(hToken,
											   TokenSessionId,
											   (PVOID)&SessionID,
											   sizeof(DWORD)));
	if (hr != S_OK)
		return hr;

	switch (PrivilegesModeType)
	{
	case NSUDO_PRIVILEGES_MODE_TYPE::ENABLE_ALL_PRIVILEGES:

		hr = Mile::set_token_all_privilege(hToken, SE_PRIVILEGE_ENABLED);
		if (hr != S_OK)
			return hr;

		break;
	case NSUDO_PRIVILEGES_MODE_TYPE::DISABLE_ALL_PRIVILEGES:

		hr = Mile::set_token_all_privilege(hToken, 0);
		if (hr != S_OK)
			return hr;

		break;
	default:
		break;
	}

	if (ProcessMandatory::Untrusted != _mandatory)
	{
		hr = Mile::set_token_mandatory(hToken, mandatory);
		if (hr != S_OK)
			return hr;
	}

	DWORD dwCreationFlags = CREATE_SUSPENDED | CREATE_UNICODE_ENVIRONMENT;

	if (CreateNewConsole)
	{
		dwCreationFlags |= CREATE_NEW_CONSOLE;
	}

	STARTUPINFOW StartupInfo = { 0 };
	PROCESS_INFORMATION ProcessInfo = { 0 };

	StartupInfo.cb = sizeof(STARTUPINFOW);

	StartupInfo.lpDesktop = const_cast<LPWSTR>(L"WinSta0\\Default");

	StartupInfo.dwFlags |= STARTF_USESHOWWINDOW;
	StartupInfo.wShowWindow = static_cast<WORD>(show_type);

	LPVOID lpEnvironment = nullptr;

	hr = Mile::ApiResult(::CreateEnvironmentBlock(&lpEnvironment, hToken, TRUE));
	if (hr == S_OK)
	{
		std::wstring ExpandedString = Mile::ExpandEnvironmentStringsW(std::wstring(CommandLine));
		if (hr == S_OK)
		{
			hr = Mile::ApiResult(::CreateProcessAsUserW(hToken,
														nullptr,
														const_cast<LPWSTR>(ExpandedString.c_str()),
														nullptr,
														nullptr,
														FALSE,
														dwCreationFlags,
														lpEnvironment,
														CurrentDirectory,
														&StartupInfo,
														&ProcessInfo));
			if (hr == S_OK)
			{
				::SetPriorityClass(ProcessInfo.hProcess, priority);

				::ResumeThread(ProcessInfo.hThread);

				::WaitForSingleObjectEx(ProcessInfo.hProcess, WaitInterval, FALSE);

				::CloseHandle(ProcessInfo.hProcess);
				::CloseHandle(ProcessInfo.hThread);
			}
		}

		::DestroyEnvironmentBlock(lpEnvironment);
	}

	if (hr != S_OK)
	{
		::NSudoWriteLog(
			L"NSudoCreateProcess",
			Mile::FormatUtf16String(
				L"%s failed, returns %d.",
				L"Create process",
				hr).c_str());

		return hr;
	}

	::NSudoWriteLog(
		L"NSudoCreateProcess",
		L"Everything seems to be OK");

	return S_OK;
}

#include <Dism/dism.h>
#include <libCL/io/3print.h>
#include <libCL/set/3list.h>

struct TI
{
	cl::StringW str;
};
void fun(int i, cl::Call<void(TI*)> call)
{
	TI inf;
	inf.str = L"str "; inf.str << i;
	call(&inf);
	int dd = 0;
}

int main(int argc, const char* argv[])
{
	
	{
		cl::List<cl::StringW> list;
		auto lam = [&list](TI* inf)->void
		{
			list.push_back(std::move(inf->str));
		};

		fun(1, lam);
		fun(2, [&list](TI* inf)->void
			{
				list.push_back(std::move(inf->str));
			});
		int  i = 0;
	}
	//cl::StringW pre_path = L"C:\\Users\\ccf\\Desktop\\iso\\";

	//{
	//	auto wim_path = pre_path;
	//	wim_path << L"install.wim";
	//	//wim_path << L"1.wim";

	//	auto wim = cl::Wim::open(wim_path.data());
	//	if (wim.valid())
	//	{
	//		auto tmp = pre_path; tmp << L"tmp";
	//		wim.set_tmp_dir(tmp.data());//临时目录要提前创建
	//		wim.print_info();

	//		wim.del_sub(5);
	//		wim.del_sub(5);
	//		wim.del_sub(1);
	//		wim.del_sub(1);
	//		wim.del_sub(1);

	//		int arr[] = { 1 };
	//		auto ret_wim = pre_path; ret_wim << L"1.wim";  tmp << L"2";
	//		wim.rebuild(ret_wim.data(), tmp.data(), arr, 1);//临时目录要提前创建
	//	}
	//}

	//int i = 0;
	//{
	//	auto log = pre_path; log << L"log.txt";
	//	auto tmp = pre_path; tmp << L"tmp";
	//	auto tmp2 = pre_path; tmp2 << L"tmp2";
	//	auto wim = pre_path; wim << L"1.wim";

	//	cl::Dism::init(log.data(), tmp.data());//临时目录要提前创建 
	//	//cl::Dism::mount(wim.data(), tmp2.data(), 1);

	//	{
	//		auto dism = cl::Dism::open(tmp2.data());


	//		cl::List<cl::StringW> list;

	//		//{//移除app  
	//		//	list.clear();
	//		//	dism.enum_app([&list](cl::ItemInfo* info)->void 
	//		//	{
	//		//		list.push_back(info->name_);
	//		//	});

	//		//	for (auto& p : list)
	//		//	{
	//		//		if(p != L"Microsoft.StorePurchaseApp_11811.1001.1813.0_neutral_~_8wekyb3d8bbwe" && 
	//		//		   p != L"Microsoft.VCLibs.140.00_14.0.27323.0_x64__8wekyb3d8bbwe"&&
	//		//		   p != L"Microsoft.WindowsCalculator_2020.1906.55.0_neutral_~_8wekyb3d8bbwe" &&
	//		//		   p != L"Microsoft.WindowsStore_11910.1002.513.0_neutral_~_8wekyb3d8bbwe" )
	//		//			dism.remove_app(p.data());
	//		//	}
	//		//	 
	//		//	dism.commit();
	//		//}

	//		//{//Capability 
	//		//	list.clear();
	//		//	dism.enum_Capabilities([&list](cl::ItemInfo* info)->void
	//		//	{
	//		//		list.push_back(info->name_);
	//		//	});

	//		//	for (auto& p : list)
	//		//	{
	//		//		if (p != L"DirectX.Configuration.Database~~~~0.0.1.0" &&
	//		//			p != L"Language.Basic~~~zh-CN~0.0.1.0" &&
	//		//			p != L"Language.Fonts.Hans~~~und-HANS~0.0.1.0" &&
	//		//			p != L"Microsoft.Windows.MSPaint~~~~0.0.1.0" &&
	//		//			p != L"Microsoft.Windows.Notepad~~~~0.0.1.0" &&
	//		//			p != L"Windows.Client.ShellComponents~~~~0.0.1.0")
	//		//			dism.remove_Capability(p.data());
	//		//	}
	//		//	dism.commit();
	//		//}

	//		//{//Features 
	//		//	list.clear();
	//		//	dism.enum_Features([&list](cl::ItemInfo* info)->void
	//		//	{
	//		//		list.push_back(info->name_);
	//		//	});

	//		//	for (auto& p : list)
	//		//	{
	//		//		if (p != L"Printing-PrintToPDFServices-Features" && 
	//		//			p != L"Windows-Defender-Default-Definitions" &&
	//		//			p != L"Printing-Foundation-Features" &&
	//		//			p != L"Printing-Foundation-InternetPrinting-Client" &&
	//		//			p != L"MicrosoftWindowsPowerShellV2Root"&&
	//		//			p != L"MicrosoftWindowsPowerShellV2" )
	//		//			dism.remove_Feature(p.data());
	//		//	}
	//		//	dism.commit();
	//		//}


	//		{//Package 
	//			dism.remove_Package(L"Microsoft-OneCore-DirectX-Database-FOD-Package~31bf3856ad364e35~amd64~~10.0.19041.1");
	//			dism.remove_Package(L"Microsoft-Windows-UserExperience-Desktop-Package~31bf3856ad364e35~amd64~~10.0.19041.1503");
	//			dism.commit();
	//		}

	//		{
	//			dism.enum_app([](cl::ItemInfo* info)->void {});
	//			dism.enum_Capabilities([](const cl::ItemInfo* info)->void {});
	//			dism.enum_Features([](const cl::ItemInfo* info)->void {});
	//			dism.enum_Packages([](cl::PackageInfo* info)->void {});
	//		}

	//	}

	//	cl::Dism::unmount(tmp2.data());
	//	cl::Dism::uninit();
	//	int i = 0;
	//}


	//auto c = argv[1][0];
	//int index = c - 'A';
	//printf("%c, %d\n", c, index);

	//create_proc((ProcUserType)index,//ProcUserType::System,
	//            NSUDO_PRIVILEGES_MODE_TYPE::DEFAULT,
	//            ProcessMandatory::Medium,
	//            ProcessPriorityLevel::Medium,
	//            WindowShowType::Show,
	//            0, true, 
	//            L"D:\\1.exe", nullptr);
	std::cout << "Hello World!\n";
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
