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

 