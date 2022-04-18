#pragma once
#ifndef __cstack_self__
#define __cstack_self__

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <atomic>
#include <mutex>    

namespace cl
{  
    /*################################################################################################*/
    //stack
    /*################################################################################################*/ 
    class StackNode
    { 
    public:
		s32 size_ = 0;//name或者file的大小
		s32 line_ = 0;
		StackNode* next_ = nullptr;
		wchar* name_ = nullptr;
		wchar* file_ = nullptr;

        void init(s32 size)
        {
            size_ = size - sizeof(StackNode);
            size_ /= sizeof(wchar);

            auto p = (u8*)this;
            p += sizeof(StackNode);
            name_ = (wchar*)p;
        } 
	};

    stack_api void stack_init(const wchar* pdb_dir = nullptr);
    stack_api void stack_uninit();

    stack_api StackNode* stack_alloc(const wchar* start_fun = nullptr);
    stack_api void stack_free(StackNode* nodes);

    stack_api void stack_print(StackNode* node);
     
}

namespace cl
{
	namespace detail
	{
		namespace
		{
			static inline void* win32_alloc(s64 size)
			{
				return VirtualAlloc(nullptr, size, MEM_COMMIT, PAGE_READWRITE);
			}

			static inline void win32_free(void* p)
			{
				VirtualFree(p, 0, MEM_RELEASE);
			}

			static inline void _pause()
			{
#if defined(CL_Compiler) && CL_Compiler == CL_Compiler_VC
				_mm_pause();
#elif defined(__x86_64__) || defined(__i386__)
				__asm__ volatile("pause" ::: "memory");
#elif defined(__aarch64__) || (defined(__arm__) && __ARM_ARCH >= 7)
				__asm__ volatile("yield" ::: "memory");
#else
				struct timespec ts = { 0 };
				nanosleep(&ts, 0);
#endif
			}

			class SpinLocker
			{
				std::atomic<bool> flag_;
			public:
				SpinLocker(const SpinLocker&) = delete;
				SpinLocker& operator = (const SpinLocker&) = delete;

				SpinLocker(void) { flag_.store(false, std::memory_order_relaxed); }

				void lock(void)
				{
					while (flag_.exchange(true, std::memory_order_acquire))
					{
						while (true)
						{
							_pause();//pause指令,延迟大约12纳秒
							if (!flag_.load(std::memory_order_relaxed)) break;
							std::this_thread::yield();
							if (!flag_.load(std::memory_order_relaxed)) break;
						}
					}
				}
				bool try_lock(void) { return !flag_.exchange(true, std::memory_order_acquire); }
				void unlock(void) { flag_.store(false, std::memory_order_release); }
			};

			template<typename T>
			struct LockGuardImp
			{
				T& lock_;
				LockGuardImp(T& lock) : lock_(lock) { lock_.lock(); }
				~LockGuardImp() { lock_.unlock(); }
			};

#define __LockGuard(x, line)  LockGuardImp<decltype(x)> _##line(x)   //LockGuardImp _123(x)
#define _LockGuard(x, line)   __LockGuard(x, line)                   //__LockGuard(x, 123)
#define LockGuard(x)          _LockGuard(x, __LINE__)                //_LockGuard(x, __LINE__)
		}
	}
}

#endif//__cstack_self__

#if defined(CL_Sys) && CL_Sys == CL_Sys_Win

#include <dbghelp.h>   
#include <tlhelp32.h>
#include <psapi.h>  

#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "User32.lib")

#include <CL/base/5exception.h>
#include <CL/base/8str.h>
#include <CL/base/10ptr.h>

//All DbgHelp functions, are single threaded. 
//Therefore, calls from more than one thread to this function will likely result in unexpected behavior or memory corruption.

namespace cl
{
#pragma warning(disable:4996)
#pragma warning(disable:26451)

	namespace detail
	{
		namespace
		{
			struct IMAGEHLP_MODULE64_V2
			{
				DWORD    SizeOfStruct;           // set convert sizeof(IMAGEHLP_MODULE64)
				DWORD64  BaseOfImage;            // base load address of module
				DWORD    ImageSize;              // virtual size of the loaded module
				DWORD    TimeDateStamp;          // date/time stamp from pe header
				DWORD    CheckSum;               // checksum from the pe header
				DWORD    NumSyms;                // number of symbols in the symbol table
				SYM_TYPE SymType;                // type of symbols loaded
				CHAR     ModuleName[32];         // module name
				CHAR     ImageName[256];         // image name
				CHAR     LoadedImageName[256];   // symbol file name
			};

			struct IMAGEHLP_MODULE64_V3 : public IMAGEHLP_MODULE64_V2
			{
				CHAR     LoadedPdbName[256];     // pdb file name
				DWORD    CVSig;                  // Signature of the CV record in the debug directories
				CHAR     CVData[MAX_PATH * 3];   // Contents of the CV record
				DWORD    PdbSig;                 // Signature of PDB
				GUID     PdbSig70;               // Signature of PDB (VC 7 and up)
				DWORD    PdbAge;                 // DBI age of pdb
				BOOL     PdbUnmatched;           // loaded an unmatched pdb
				BOOL     DbgUnmatched;           // loaded an unmatched dbg
				BOOL     LineNumbers;            // we have line number information
				BOOL     GlobalSymbols;          // we have internal symbol information
				BOOL     TypeInfo;               // we have type information
												 // new elements: 17-Dec-2003
				BOOL     SourceIndexed;          // pdb supports source server
				BOOL     Publics;                // contains public symbols
												 // new element: 15-Jul-2009
				DWORD    MachineType;            // IMAGE_FILE_MACHINE_XXX from ntimage.h and winnt.h
				DWORD    Reserved;               // Padding - don't remove.
			};
			 
			struct _CloseHandle { void free(void* p) { CloseHandle(p); } };

			//pdb 符号路径
			//例: C:\pdb; D:\ABC\pdb 
			static u8* g_init_proc_buf = nullptr;
			static SpinLocker g_locker;

			//初始化符号路径(即*.pdb文件的路径)
			//符号路径1;符号路径2;符号路径3;...
			void _init_sym_path(const wchar* self_path)
			{
				s32 size = 1024 * 8;
				s32 size_cur = 1024 * 4;
				s32 size_mod = 1024 * 4;

				auto path = (wchar*)g_init_proc_buf;
				auto dir_cur = path + size;
				auto dir_mod = dir_cur + size_cur;


				//指定符号路径
				if (str_valid(self_path))
				{
					auto len = str_copy(path, size, self_path);
					path += len;
					*path++ = ';';
					size -= len + 1;
				}
				cl_assert(size > 4);//确保不溢出


				s32 len_cur = GetCurrentDirectoryW(size_cur, dir_cur);
				{
					if (dir_cur[len_cur - 1] == '\\')
					{
						dir_cur[len_cur - 1] = 0;
						len_cur -= 1;
					}
				}
				s32 len_mod = GetModuleFileNameW(NULL, dir_mod, size_mod);
				{
					auto p = str_findr(dir_mod, '\\');
					if (p)
					{
						*p = 0;
						len_mod = u32(p - dir_mod);
					}
				}
				if (len_mod == len_cur && str_equ(path, dir_mod)) len_mod = 0; //当工作目录和模块所在目录一致时候, 避免重复

				auto add_dir = [&path, &size](wchar* str, s32 len)
				{
					if (len > 0 && len * 2 + 8 < size)
					{
						_str_copy(path, str);
						path += len;
						*path++ = ';';
						size -= len + 1;

						_str_copy(path, str);
						path += len;
						size -= len;

						len = _str_copy(path, "\\pdb");
						path += len;
						*path++ = ';';
						size -= len + 1;
					}
				};
				if (len_cur) add_dir(dir_cur, len_cur);
				if (len_mod) add_dir(dir_mod, len_mod);
				*path = 0;
			}

			//加载模块
			void _init_load_module_from_TH32(HANDLE proc, DWORD proc_id)
			{
				cl::_Ptr<char, _CloseHandle> snapshot = (char*)CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, proc_id);
				if (snapshot == INVALID_HANDLE_VALUE)
					return;

				//加载模块信息
				MODULEENTRY32W me;
				me.dwSize = sizeof(me);
				if (!Module32FirstW(snapshot, &me)) return;

				do
				{
					SymLoadModuleExW(proc, 0, me.szExePath, me.szModule, (DWORD64)me.modBaseAddr, me.modBaseSize, nullptr, 0);
				} while (Module32NextW(snapshot, &me));
			}

			//加载模块, 其实和_load_module_from_TH32一样, 只是从不同的地方加载
			void _init_load_module_from_PSAPI(HANDLE proc)
			{
				static constexpr s32 MaxSize = 4096;
				static constexpr s32 arr_size = sizeof(HMODULE) * MaxSize;

				auto arr = (HMODULE*)g_init_proc_buf;
				auto file = (wchar*)(arr + MaxSize);
				auto name = file + MaxSize;

				DWORD bytes;
				bool ret = EnumProcessModulesEx(proc, arr, arr_size, &bytes, LIST_MODULES_64BIT);
				cl_assert(bytes <= arr_size);
				cl_assert(ret);

				auto cnt = bytes / sizeof(HMODULE);

				MODULEINFO mi;
				for (size_t i = 0; i < cnt; i++)
				{
					GetModuleInformation(proc, arr[i], &mi, sizeof(mi));
					GetModuleFileNameExW(proc, arr[i], file, MaxSize);
					GetModuleBaseNameW(proc, arr[i], name, MaxSize);

					SymLoadModuleExW(proc, 0, file, name, (DWORD64)mi.lpBaseOfDll, mi.SizeOfImage, nullptr, 0);
				}
			}

			//初始化当前的线程上下文
			inline void _init_ctx(CONTEXT* ctx, const CONTEXT* _ctx)
			{
				if (_ctx == NULL)
				{
					memset(ctx, 0, sizeof(CONTEXT));
					ctx->ContextFlags = CONTEXT_ALL;
#if CL_Bits == CL_Bits_64
					RtlCaptureContext(ctx);
#else 
					//call x  可以分为2步, 即 #push ip #jmp x
					{
						__asm                call lable_asm_x
						__asm  lable_asm_x:  pop eax
						__asm                mov ctx->Eip, eax
						__asm                mov ctx->Ebp, ebp
						__asm                mov ctx->Esp, esp
					}
#endif
				}
				else
					*ctx = *_ctx;
			}

			//初始化堆栈帧信息
			inline void _init_stack_frame(STACKFRAME64& stack, DWORD* img_type, CONTEXT* ctx)
			{
				memset(&stack, 0, sizeof(stack));
				stack.AddrPC.Mode = stack.AddrFrame.Mode = stack.AddrStack.Mode = AddrModeFlat;
#if CL_Bits == CL_Bits_64
				* img_type = IMAGE_FILE_MACHINE_AMD64;
				stack.AddrPC.Offset = ctx->Rip;
				stack.AddrFrame.Offset = ctx->Rsp;
				stack.AddrStack.Offset = ctx->Rsp;
#else
				* img_type = IMAGE_FILE_MACHINE_I386;
				stack.AddrPC.Offset = ctx->Eip;
				stack.AddrFrame.Offset = ctx->Ebp;
				stack.AddrStack.Offset = ctx->Esp;
#endif 
			}
			 
			//根据addr来填充SymbolObj
			//本函数非线程安全, 所以需要加锁
			s32 _get_name(HANDLE proc, wchar* name, s32 name_size, DWORD64 addr)
			{
				LockGuard(g_locker);
				static constexpr s32 buf_size = 1024;
				static char buf[buf_size];
				static_assert(sizeof(SYMBOL_INFOW) + 256 < buf_size, "buffer overflow!");

				auto sym = (SYMBOL_INFOW*)buf;
				sym->SizeOfStruct = sizeof(SYMBOL_INFOW);
				sym->MaxNameLen = 256;

				DWORD64 offset;
				if (!SymFromAddrW(proc, addr, &offset, sym))
					return false;

				return UnDecorateSymbolNameW(sym->Name, name, name_size, UNDNAME_COMPLETE);
			}

			//根据addr来填充SymbolObj
			//本函数非线程安全, 所以需要加锁
			bool _get_file_and_line(HANDLE proc, wchar* file, s32 file_size, s32& _line, DWORD64 addr)
			{
				LockGuard(g_locker);

				IMAGEHLP_LINEW64 line;
				line.SizeOfStruct = sizeof(line);

				DWORD offset;
				if (!SymGetLineFromAddrW64(proc, addr, &offset, &line)) return false;

				_line = line.LineNumber;
				s32 len = str_copy(file, file_size, line.FileName);
				cl_assert(len != 0);
				return true;
			}

			class StackNodeCenter
			{
				static constexpr s32 StackNodeSize = 1024;

				s32 block_cnt_ = 0;
				u8* blocks_[16];
				StackNode* list_ = nullptr; 
	 
				u8* _alloc_more()
				{ 
					static constexpr s32 block_size = 4 * 1024 * 1024;  
					auto block = (u8*)win32_alloc(block_size);

					s32 cnt = block_size / StackNodeSize;
					auto pb = block;
					for (s32 i = 0; i < cnt; i++)
					{
						auto node = (StackNode*)pb;
						pb += StackNodeSize;

						node->next_ = list_;
						list_ = node;
					}

					return block;
				}
			public:
				~StackNodeCenter() 
				{
					for (s32 i = 0; i < block_cnt_; i++) 
						win32_free(blocks_[i]); 
				}

				StackNode* alloc()
				{
					LockGuard(g_locker);
					if (list_ == nullptr)
					{
						cl_assert(block_cnt_ < 16);
						blocks_[block_cnt_++] = _alloc_more(); 
					}
					auto n = list_;
					list_ = list_->next_;

					n->init(StackNodeSize);
					return n;
				}

				void free(StackNode* n)
				{
					LockGuard(g_locker);
					n->next_ = list_;
					list_ = n;
				}

				void free_list(StackNode* head, StackNode* tail)
				{
					LockGuard(g_locker);
					tail->next_ = list_;
					list_ = head;
				}
			};

			class Symbol
			{
				HANDLE proc_ = nullptr;
				DWORD proc_id_ = 0;
				StackNodeCenter nodes_;
			public: 
				Symbol() { } 
				~Symbol() { SymCleanup(proc_); }

				void init(const wchar* self_sym_path)
				{
					LockGuard(g_locker);
					if (proc_) return;

					g_init_proc_buf = (u8*)win32_alloc(4 * 1024 * 1024);
					if (!g_init_proc_buf) return;

					_init_sym_path(self_sym_path);

					proc_ = GetCurrentProcess();//无需关闭
					proc_id_ = GetCurrentProcessId();

					if (!SymInitializeW(proc_, (wchar*)g_init_proc_buf, FALSE))
						return;

					DWORD options = SymGetOptions();
					options |= SYMOPT_DEFERRED_LOADS;//延迟加载符号
					options |= SYMOPT_DISABLE_SYMSRV_AUTODETECT;//禁用sym SRV
					options |= SYMOPT_LOAD_LINES;//加载文件行数
					options |= SYMOPT_OMAP_FIND_NEAREST;//但代码被优化时,加载最近的行
					options |= SYMOPT_FAIL_CRITICAL_ERRORS;//禁止加载调试信息文件失败时弹出对话框
					options |= SYMOPT_NO_PROMPTS;//抑制代理服务器的对话框认证
					options = SymSetOptions(options);

					_init_load_module_from_TH32(proc_, proc_id_);
					_init_load_module_from_PSAPI(proc_);

					win32_free(g_init_proc_buf); g_init_proc_buf = nullptr;
				}

				StackNode* alloc_stack(const wchar* start_fun, const CONTEXT* _ctx = nullptr)
				{
					CONTEXT ctx;
					_init_ctx(&ctx, _ctx);

					STACKFRAME64 frame;
					DWORD img_type;
					_init_stack_frame(frame, &img_type, &ctx);

					StackNode* head = nodes_.alloc();
					head->next_ = nullptr;

					if (start_fun == nullptr) start_fun = L"cl::stack_alloc";
					do
					{
						{
							LockGuard(g_locker);
							if (!StackWalk64(img_type,
											 proc_,
											 GetCurrentThread(),
											 &frame,
											 &ctx,
											 nullptr,
											 SymFunctionTableAccess64,
											 SymGetModuleBase64,
											 nullptr))
								break;
						}

						auto addr = frame.AddrPC.Offset;
						if (addr == 0)
						{
							nodes_.free(head);
							return nullptr;
						}

						if (_get_name(proc_, head->name_, head->size_, addr) == 0)
							continue;

						if (str_find(head->name_, start_fun) != nullptr)
							break;
					} while (true);

					while (true)
					{
						{
							LockGuard(g_locker);
							if (!StackWalk64(img_type,
											 proc_,
											 GetCurrentThread(),
											 &frame,
											 &ctx,
											 nullptr,
											 SymFunctionTableAccess64,
											 SymGetModuleBase64,
											 nullptr))
								break;
						}

						auto addr = frame.AddrPC.Offset;
						if (addr == 0) break;

						auto name_len = _get_name(proc_, head->name_, head->size_, addr);
						if (name_len == 0)
							continue;

						{
							head->size_ -= name_len + 1;
							head->file_ = head->name_ + name_len + 1;
						} 

						if (!_get_file_and_line(proc_, head->file_, head->size_, head->line_, addr)) continue;

						bool is_end = false;
						if (str_find(head->name_, L"WinMain") != nullptr ||
							str_find(head->name_, L"main") != nullptr)
							is_end = true;

						StackNode* node = nodes_.alloc();
						node->next_ = head;
						head = node;

						if (is_end) break;
					}

					auto node = head;
					head = head->next_;
					nodes_.free(node);
					return head;
				}

				void free_stack(StackNode* node)
				{
					auto tail = node;
					while (tail->next_) tail = tail->next_;
					nodes_.free_list(node, tail);
				}
			};

			static u8 g_symbol_buf[sizeof(Symbol)];
			static Symbol* g_symbol = nullptr; 
		}
	}

	stack_api void stack_print(StackNode* node)
	{
#if CL_Version == CL_Version_Debug  
		StrBufW<4096> buf;
		auto p = node;
		while (p)
		{
			buf << p->file_ << '(' << p->line_ << ")\r\n" << p->name_ << "\r\n";
			DPrint() << buf;
			buf.clear();
			p = p->next_;
		}
#endif
	}

	stack_api StackNode* stack_alloc(const wchar* start_fun)
	{ 
		return detail::g_symbol->alloc_stack(start_fun);
	}
	stack_api void stack_free(StackNode* node)
	{ 
		detail::g_symbol->free_stack(node);
	}

	stack_api void stack_init(const wchar* pdb_dir)
	{
		detail::g_symbol = (detail::Symbol*)detail::g_symbol_buf;
		new(detail::g_symbol)detail::Symbol();

		detail::g_symbol->init(pdb_dir);
	}
	stack_api void stack_uninit()
	{
		detail::g_symbol->~Symbol();
		detail::g_symbol = nullptr;
	}
}

#endif

//根据addr来填充SymbolObj
//本函数非线程安全, 所以需要加锁
//bool _get_module(HANDLE proc, SymStack* stack, DWORD64 addr)
//{
//	if (stack->buf_over()) return false;

//	LockGuard(g_sym_locker);

//	IMAGEHLP_MODULE64_V3 mm; //memset(&mm, 0, sizeof(IMAGEHLP_MODULE64_V3)); 
//	do
//	{
//		mm.SizeOfStruct = sizeof(IMAGEHLP_MODULE64_V3);
//		if (SymGetModuleInfo64(proc, addr, (IMAGEHLP_MODULE64*)&mm))
//			break;

//		mm.SizeOfStruct = sizeof(IMAGEHLP_MODULE64_V2);
//		if (SymGetModuleInfo64(proc, addr, (IMAGEHLP_MODULE64*)&mm))
//			break;

//		stack->module_[stack->cnt_] = "";
//		return false;
//	} while (false);

//	return stack->set_module(mm.ModuleName);
//}