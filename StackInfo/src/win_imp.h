#include "../StackInfo.h"

#include <Windows.h> 
#include <dbghelp.h>   
#include <tlhelp32.h>
#include <psapi.h>  

#include <CL/base/2type.h>
#include <CL/base/9str.h>
#include <CL/io/2path.h>
#include <CL/io/3print.h>

#include <CL/task/lock.h>

#include <WL/base/2ptr.h>

#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "User32.lib")

//All DbgHelp functions, are single threaded. 
//Therefore, calls from more than one thread to this function 
//will likely result in unexpected behavior or memory corruption.

namespace cl
{
#pragma warning(disable:4996)
#pragma warning(disable:26451)

	namespace lib
	{
		//struct IMAGEHLP_MODULE64_V2
		//{
		//	DWORD    SizeOfStruct;           // set convert sizeof(IMAGEHLP_MODULE64)
		//	DWORD64  BaseOfImage;            // base load address of module
		//	DWORD    ImageSize;              // virtual size of the loaded module
		//	DWORD    TimeDateStamp;          // date/time stamp from pe header
		//	DWORD    CheckSum;               // checksum from the pe header
		//	DWORD    NumSyms;                // number of symbols in the symbol table
		//	SYM_TYPE SymType;                // type of symbols loaded
		//	CHAR     ModuleName[32];         // module name
		//	CHAR     ImageName[256];         // image name
		//	CHAR     LoadedImageName[256];   // symbol file name
		//};

		//struct IMAGEHLP_MODULE64_V3 : public IMAGEHLP_MODULE64_V2
		//{
		//	CHAR     LoadedPdbName[256];     // pdb file name
		//	DWORD    CVSig;                  // Signature of the CV record in the debug directories
		//	CHAR     CVData[MAX_PATH * 3];   // Contents of the CV record
		//	DWORD    PdbSig;                 // Signature of PDB
		//	GUID     PdbSig70;               // Signature of PDB (VC 7 and up)
		//	DWORD    PdbAge;                 // DBI age of pdb
		//	BOOL     PdbUnmatched;           // loaded an unmatched pdb
		//	BOOL     DbgUnmatched;           // loaded an unmatched dbg
		//	BOOL     LineNumbers;            // we have line number information
		//	BOOL     GlobalSymbols;          // we have internal symbol information
		//	BOOL     TypeInfo;               // we have type information
		//									 // new elements: 17-Dec-2003
		//	BOOL     SourceIndexed;          // pdb supports source server
		//	BOOL     Publics;                // contains public symbols
		//									 // new element: 15-Jul-2009
		//	DWORD    MachineType;            // IMAGE_FILE_MACHINE_XXX from ntimage.h and winnt.h
		//	DWORD    Reserved;               // Padding - don't remove.
		//};

		static cl::SpinLock g_locker;

		//初始化符号路径(即*.pdb文件的路径)
		//符号路径1;符号路径2;符号路径3;...
		cl::StringW _init_sym_path(const cl::StrViewW& custom_path)
		{
			cl::StringW ret;
			ret.reserve(4096);

			cl::StrViewW path = custom_path;
			cl::StringW cur = cl::Path::work_dir();
			cl::StringW exe = cl::Path::exe_dir();

			if (cur == exe) exe.clear();
			if (cur == path) path.clear();
			if (exe == path) path.clear();

			if (path.length())
			{
				ret << path;
				ret << ';';
			}
			{
				ret << cur;
				ret << ';';

				cl::Path::join(cur, "pdb;");
				ret << cur;
			}
			if (exe.length())
			{
				ret << exe;
				ret << ';';
				cl::Path::join(exe, "pdb;");
				ret << exe;
			}
#if CL_Version == CL_Version_Debug
			cl::Print() << ret << "\n";
#endif 
			return ret;
		}

		//加载模块
		void _init_load_module_from_TH32(HANDLE proc, DWORD proc_id)
		{
			wl::Ptr<HANDLE, wl::Fun_CloseHanle> snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, proc_id);
			if (snapshot == INVALID_HANDLE_VALUE)
				return;

			//加载模块信息
			MODULEENTRY32W me;
			me.dwSize = sizeof(me);
			if (!Module32FirstW(snapshot, &me)) return;

			do
			{
				SymLoadModuleExW(proc, // proc
								 0,    // file 
								 me.szExePath,
								 me.szModule,//module name
								 (DWORD64)me.modBaseAddr, //base addr of dll
								 me.modBaseSize,//dll size
								 nullptr,//
								 0);//flags
			} while (Module32NextW(snapshot, &me));
		}

		//加载模块, 其实和_load_module_from_TH32一样, 只是从不同的地方加载
		void _init_load_module_from_PSAPI(HANDLE proc)
		{
			cl::MemBuf<uv8> buf;
			buf.alloc(8192);

			DWORD bytes;
			do
			{
				bool ret = EnumProcessModulesEx(proc, (HMODULE*)buf.data(), buf.size(), &bytes, LIST_MODULES_64BIT);
				if (bytes > buf.size())
				{
					buf.free();
					buf.alloc(bytes + sizeof(HMODULE*));
				}
				else
					break;
			} while (true);

			auto cnt = bytes / sizeof(HMODULE);

			auto p = (HMODULE*)buf.data();

			cl::MemBuf<wchar> _file, _name;
			_file.alloc(4096);
			_name.alloc(4096);

			auto file = _file.data();
			auto name = _name.data();

			MODULEINFO mi;
			for (size_t i = 0; i < cnt; i++)
			{
				GetModuleInformation(proc, p[i], &mi, sizeof(mi));
				GetModuleFileNameExW(proc, p[i], file, 4096);
				GetModuleBaseNameW(proc, p[i], name, 4096);

				SymLoadModuleExW(proc, 0, file, name, (DWORD64)mi.lpBaseOfDll, mi.SizeOfImage, nullptr, 0);
			}
		} 

		class Symbol
		{
			HANDLE proc_ = nullptr;
			DWORD proc_id_ = 0;
		public:
			Symbol() { }
			~Symbol() { SymCleanup(proc_); }

			void init(const cl::StrViewW& custom_path)
			{
				if (proc_) return; //已经初始化过了

				proc_ = GetCurrentProcess();//无需关闭
				proc_id_ = GetCurrentProcessId();

				DWORD options = SymGetOptions();
				options |= SYMOPT_DEFERRED_LOADS;//延迟加载符号
				options |= SYMOPT_DISABLE_SYMSRV_AUTODETECT;//禁用sym SRV
				options |= SYMOPT_LOAD_LINES;//加载文件行数
				options |= SYMOPT_OMAP_FIND_NEAREST;//但代码被优化时,加载最近的行
				options |= SYMOPT_FAIL_CRITICAL_ERRORS;//禁止加载调试信息文件失败时弹出对话框
				options |= SYMOPT_NO_PROMPTS;//抑制代理服务器的对话框认证
				options = SymSetOptions(options);

				auto paths = _init_sym_path(custom_path);

#if CL_Version == CL_Version_Debug

#endif
				if (!SymInitializeW(proc_,
									(wchar*)paths.data(),
									FALSE))//遍历模块
					return;

				_init_load_module_from_TH32(proc_, proc_id_);
				_init_load_module_from_PSAPI(proc_);
			}

			//根据addr来填充SymbolObj
			//本函数非线程安全, 所以需要加锁
			cl::StringW get_fun(void* addr)
			{
				cl::MemBuf<uv8> buf;
				buf.alloc(4096);

				auto sym = (SYMBOL_INFOW*)buf.data();
				mem::zero(sym, sizeof(SYMBOL_INFOW));
				sym->SizeOfStruct = sizeof(SYMBOL_INFOW);
				sym->MaxNameLen = 1024;

				auto name = (wchar*)(buf.data() + 2048);
				uv32 len = 0;
				{
					LockGuard(g_locker);

					if (!SymFromAddrW(proc_, (DWORD64)addr, nullptr, sym))
						return L"";

					len = UnDecorateSymbolNameW(sym->Name, name, 1024, UNDNAME_COMPLETE);
				}
				name[len] = 0;
				return name;
			}

			//根据addr来填充SymbolObj
			//本函数非线程安全, 所以需要加锁
			cl::StringW get_file_and_line(void* addr, uv32& line)
			{
				line = 0;

				IMAGEHLP_LINEW64 info;
				mem::zero(&info, sizeof(IMAGEHLP_LINEW64));
				info.SizeOfStruct = sizeof(IMAGEHLP_LINEW64);

				{
					LockGuard(g_locker);
					DWORD offset;
					if (!SymGetLineFromAddrW64(proc_, (DWORD64)addr, &offset, &info)) return L"";

					line = info.LineNumber;
					return info.FileName;
				}
			}

		};
		uv8 g_symbol_buf[sizeof(Symbol)];
	}
	stack_api void stack_init(const wchar* pdb_dir)
	{
		auto sym = (lib::Symbol*)lib::g_symbol_buf;
		new(sym)lib::Symbol();
		sym->init(pdb_dir);
	}
	stack_api void stack_uninit()
	{
		auto sym = (lib::Symbol*)lib::g_symbol_buf;
		sym->~Symbol();
	}

	stack_api StackInfo* stack_alloc()
	{
		auto stack = (StackInfo*)VirtualAlloc(nullptr, 8192, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (stack == nullptr) return nullptr;
		stack->size_ = CaptureStackBackTrace(0, 8192 / sizeof(void*), (void**)stack->buf_, nullptr);
		return stack;
	}
	stack_api void stack_free(StackInfo* stack)
	{
		VirtualFree(stack, 0, MEM_RELEASE);
	}

	stack_api bool frame_info(StackInfo* stack, uv32 index, FrameInfo* frame)
	{
		if (index >= stack->size_) return false;
		auto addrs = (void**)stack->buf_;
		auto sym = (lib::Symbol*)lib::g_symbol_buf;
		auto addr = addrs[index];
		{ 
			//frame->fun_ = sym->get_fun(addr);
			//frame->file_ = sym->get_file_and_line(addr, frame->line_);
			sym->get_fun(addr);
			sym->get_file_and_line(addr, frame->line_);
		}
		return frame->fun_.length() && frame->file_.length();
	}
}


