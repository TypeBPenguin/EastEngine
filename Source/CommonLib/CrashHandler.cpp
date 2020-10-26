#include "stdafx.h"
#include "CrashHandler.h"

#include "StringUtil.h"
#include "FileUtil.h"

#include <tchar.h>
#include <csignal>
#include <DbgHelp.h>

namespace est
{
	namespace CrashHandler
	{
		enum
		{
			eCrashDumpMemory = 5 * 1024 * 1024,
		};

		static std::unique_ptr<char[]> s_crashDumpBuffer = nullptr;
		std::wstring s_miniDumpPath;

		void SetProcessExceptionHandlers();
		void SetThreadExceptionHandlers();

		bool Initialize(const wchar_t* path)
		{
			if (s_crashDumpBuffer == nullptr)
			{
				s_crashDumpBuffer = std::make_unique<char[]>(eCrashDumpMemory);

				SetProcessExceptionHandlers();
				SetThreadExceptionHandlers();
			}

			s_miniDumpPath = path;

			return true;
		}

		void Release()
		{
			s_crashDumpBuffer.reset();
		}

		void ForceCrash()
		{
			int* p = nullptr;
			*p = 0;
		}

		void SecureMemory()
		{
			Release();
		}

		// Collects current process state.
		void GetExceptionPointers(DWORD dwExceptionCode, EXCEPTION_POINTERS** ppExceptionPointers)
		{
			// The following code was taken from VC++ 8.0 CRT (invarg.c: line 104)

			EXCEPTION_RECORD ExceptionRecord;
			CONTEXT ContextRecord;
			ZeroMemory(&ContextRecord, sizeof(CONTEXT));

#ifdef _X86_
			__asm
			{
				mov dword ptr[ContextRecord.Eax], eax
				mov dword ptr[ContextRecord.Ecx], ecx
				mov dword ptr[ContextRecord.Edx], edx
				mov dword ptr[ContextRecord.Ebx], ebx
				mov dword ptr[ContextRecord.Esi], esi
				mov dword ptr[ContextRecord.Edi], edi
				mov word ptr[ContextRecord.SegSs], ss
				mov word ptr[ContextRecord.SegCs], cs
				mov word ptr[ContextRecord.SegDs], ds
				mov word ptr[ContextRecord.SegEs], es
				mov word ptr[ContextRecord.SegFs], fs
				mov word ptr[ContextRecord.SegGs], gs
				pushfd
				pop[ContextRecord.EFlags]
			}

			ContextRecord.ContextFlags = CONTEXT_CONTROL;
#pragma warning(push)
#pragma warning(disable:4311)
			ContextRecord.Eip = (ULONG)_ReturnAddress();
			ContextRecord.Esp = (ULONG)_AddressOfReturnAddress();
#pragma warning(pop)
			ContextRecord.Ebp = *((ULONG *)_AddressOfReturnAddress() - 1);

#elif defined (_IA64_) || defined (_AMD64_)

			/* Need to fill up the Context in IA64 and AMD64. */
			RtlCaptureContext(&ContextRecord);

#else  /* defined (_IA64_) || defined (_AMD64_) */

			ZeroMemory(&ContextRecord, sizeof(ContextRecord));

#endif  /* defined (_IA64_) || defined (_AMD64_) */

			ZeroMemory(&ExceptionRecord, sizeof(EXCEPTION_RECORD));

			ExceptionRecord.ExceptionCode = dwExceptionCode;
			ExceptionRecord.ExceptionAddress = _ReturnAddress();

			EXCEPTION_RECORD* pExceptionRecord = new EXCEPTION_RECORD;
			memcpy(pExceptionRecord, &ExceptionRecord, sizeof(EXCEPTION_RECORD));

			CONTEXT* pContextRecord = new CONTEXT;
			memcpy(pContextRecord, &ContextRecord, sizeof(CONTEXT));

			*ppExceptionPointers = new EXCEPTION_POINTERS;
			(*ppExceptionPointers)->ExceptionRecord = pExceptionRecord;
			(*ppExceptionPointers)->ContextRecord = pContextRecord;
		}

		// This method creates minidump of the process
		void CreateMiniDump(EXCEPTION_POINTERS* pExcPtrs)
		{
			SecureMemory();

			// Load dbghelp.dll
			HMODULE hDbgHelp = LoadLibrary(_T("dbghelp.dll"));
			if (hDbgHelp == nullptr)
			{
				// Error - couldn't load dbghelp.dll
				return;
			}

			std::time_t curTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

			std::tm tmTime;
			localtime_s(&tmTime, &curTime);
			
			std::wstring dumpPath = string::Format(L"%s%s_[%d%02d%02d-%02d%02d%02d]_CrashMiniDump.dmp", 
				s_miniDumpPath.c_str(), 
				file::GetProgramFileName(),
				tmTime.tm_year + 1900,
				tmTime.tm_mon + 1, 
				tmTime.tm_mday, 
				tmTime.tm_hour, 
				tmTime.tm_min, 
				tmTime.tm_sec
				);

			CreateDirectory(s_miniDumpPath.c_str(), nullptr);

			// Create the minidump file
			HANDLE hFile = CreateFile(
				dumpPath.c_str(),
				GENERIC_WRITE,
				0,
				nullptr,
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				nullptr);

			if (hFile == INVALID_HANDLE_VALUE)
			{
				// Couldn't create file
				return;
			}

			// Write minidump to the file
			MINIDUMP_EXCEPTION_INFORMATION mei;
			mei.ThreadId = GetCurrentThreadId();
			mei.ExceptionPointers = pExcPtrs;
			mei.ClientPointers = FALSE;

			MINIDUMP_CALLBACK_INFORMATION mci;
			mci.CallbackRoutine = nullptr;
			mci.CallbackParam = nullptr;

			typedef BOOL(WINAPI *LPMINIDUMPWRITEDUMP)(
				HANDLE hProcess,
				DWORD ProcessId,
				HANDLE hFile,
				MINIDUMP_TYPE DumpType,
				CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
				CONST PMINIDUMP_USER_STREAM_INFORMATION UserEncoderParam,
				CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

			LPMINIDUMPWRITEDUMP pfnMiniDumpWriteDump = reinterpret_cast<LPMINIDUMPWRITEDUMP>(GetProcAddress(hDbgHelp, "MiniDumpWriteDump"));
			if (pfnMiniDumpWriteDump == nullptr)
			{
				// Bad MiniDumpWriteDump function
				return;
			}

			HANDLE hProcess = GetCurrentProcess();
			DWORD dwProcessId = GetCurrentProcessId();

			BOOL bWriteDump = pfnMiniDumpWriteDump(
				hProcess,
				dwProcessId,
				hFile,
				MiniDumpNormal,
				&mei,
				nullptr,
				&mci);

			if (bWriteDump == FALSE)
			{
				// Error writing dump.
				return;
			}

			// Close file
			CloseHandle(hFile);

			// Unload dbghelp.dll
			FreeLibrary(hDbgHelp);
		}

		/* Exception handler functions. */
		LONG WINAPI SehHandler(PEXCEPTION_POINTERS pExceptionPtrs)
		{
			// Write minidump file
			CreateMiniDump(pExceptionPtrs);

			// Terminate process
			TerminateProcess(GetCurrentProcess(), 1);

			// Unreacheable code  
			return EXCEPTION_EXECUTE_HANDLER;
		}

		void __cdecl TerminateHandler()
		{
			// Abnormal program termination (terminate() function was called)

			// Retrieve exception information
			EXCEPTION_POINTERS* pExceptionPtrs = nullptr;
			GetExceptionPointers(0, &pExceptionPtrs);

			// Write minidump file
			CreateMiniDump(pExceptionPtrs);

			// Terminate process
			TerminateProcess(GetCurrentProcess(), 1);
		}

		void __cdecl UnexpectedHandler()
		{
			// Unexpected error (unexpected() function was called)

			// Retrieve exception information
			EXCEPTION_POINTERS* pExceptionPtrs = nullptr;
			GetExceptionPointers(0, &pExceptionPtrs);

			// Write minidump file
			CreateMiniDump(pExceptionPtrs);

			// Terminate process
			TerminateProcess(GetCurrentProcess(), 1);
		}

		void __cdecl PureCallHandler()
		{
			// Pure virtual function call

			// Retrieve exception information
			EXCEPTION_POINTERS* pExceptionPtrs = nullptr;
			GetExceptionPointers(0, &pExceptionPtrs);

			// Write minidump file
			CreateMiniDump(pExceptionPtrs);

			// Terminate process
			TerminateProcess(GetCurrentProcess(), 1);
		}

		void __cdecl InvalidParameterHandler(const wchar_t* expression,
			const wchar_t* function, const wchar_t* file,
			unsigned int line, uintptr_t pReserved)
		{
			// Invalid parameter exception

			// Retrieve exception information
			EXCEPTION_POINTERS* pExceptionPtrs = nullptr;
			GetExceptionPointers(0, &pExceptionPtrs);

			// Write minidump file
			CreateMiniDump(pExceptionPtrs);

			// Terminate process
			TerminateProcess(GetCurrentProcess(), 1);
		}

		int __cdecl NewHandler(size_t)
		{
			// 'new' operator memory allocation exception

			// Retrieve exception information
			EXCEPTION_POINTERS* pExceptionPtrs = nullptr;
			GetExceptionPointers(0, &pExceptionPtrs);

			// Write minidump file
			CreateMiniDump(pExceptionPtrs);

			// Terminate process
			TerminateProcess(GetCurrentProcess(), 1);

			// Unreacheable code
			return 0;
		}

		void SigabrtHandler(int)
		{
			// Caught SIGABRT C++ signal

			// Retrieve exception information
			EXCEPTION_POINTERS* pExceptionPtrs = nullptr;
			GetExceptionPointers(0, &pExceptionPtrs);

			// Write minidump file
			CreateMiniDump(pExceptionPtrs);

			// Terminate process
			TerminateProcess(GetCurrentProcess(), 1);
		}

		void SigfpeHandler(int /*code*/, int subcode)
		{
			// Floating point exception (SIGFPE)

			EXCEPTION_POINTERS* pExceptionPtrs = static_cast<PEXCEPTION_POINTERS>(_pxcptinfoptrs);

			// Write minidump file
			CreateMiniDump(pExceptionPtrs);

			// Terminate process
			TerminateProcess(GetCurrentProcess(), 1);
		}

		void SigintHandler(int)
		{
			// Interruption (SIGINT)

			// Retrieve exception information
			EXCEPTION_POINTERS* pExceptionPtrs = nullptr;
			GetExceptionPointers(0, &pExceptionPtrs);

			// Write minidump file
			CreateMiniDump(pExceptionPtrs);

			// Terminate process
			TerminateProcess(GetCurrentProcess(), 1);
		}

		void SigillHandler(int)
		{
			// Illegal instruction (SIGILL)

			// Retrieve exception information
			EXCEPTION_POINTERS* pExceptionPtrs = nullptr;
			GetExceptionPointers(0, &pExceptionPtrs);

			// Write minidump file
			CreateMiniDump(pExceptionPtrs);

			// Terminate process
			TerminateProcess(GetCurrentProcess(), 1);
		}

		void SigsegvHandler(int)
		{
			// Invalid storage access (SIGSEGV)

			PEXCEPTION_POINTERS pExceptionPtrs = static_cast<PEXCEPTION_POINTERS>(_pxcptinfoptrs);

			// Write minidump file
			CreateMiniDump(pExceptionPtrs);

			// Terminate process
			TerminateProcess(GetCurrentProcess(), 1);
		}

		void SigtermHandler(int)
		{
			// Termination request (SIGTERM)

			// Retrieve exception information
			EXCEPTION_POINTERS* pExceptionPtrs = nullptr;
			GetExceptionPointers(0, &pExceptionPtrs);

			// Write minidump file
			CreateMiniDump(pExceptionPtrs);

			// Terminate process
			TerminateProcess(GetCurrentProcess(), 1);
		}

		// Sets exception handlers that work on per-process basis
		void SetProcessExceptionHandlers()
		{
			// Install top-level SEH handler
			SetUnhandledExceptionFilter(SehHandler);

			// Catch pure virtual function calls.
			// Because there is one _purecall_handler for the whole process, 
			// calling this function immediately impacts all threads. The last 
			// caller on any thread sets the handler. 
			// http://msdn.microsoft.com/en-us/library/t296ys27.aspx
			_set_purecall_handler(PureCallHandler);

			// Catch new operator memory allocation exceptions
			_set_new_handler(NewHandler);

			// Catch invalid parameter exceptions.
			_set_invalid_parameter_handler(InvalidParameterHandler);

			// Set up C++ signal handlers

			_set_abort_behavior(_CALL_REPORTFAULT, _CALL_REPORTFAULT);

			// Catch an abnormal program termination
			signal(SIGABRT, SigabrtHandler);

			// Catch illegal instruction handler
			signal(SIGINT, SigintHandler);

			// Catch a termination request
			signal(SIGTERM, SigtermHandler);
		}

		// Installs C++ exception handlers that function on per-thread basis
		void SetThreadExceptionHandlers()
		{
			// Catch terminate() calls. 
			// In a multithreaded environment, terminate functions are maintained 
			// separately for each thread. Each new thread needs to install its own 
			// terminate function. Thus, each thread is in charge of its own termination handling.
			// http://msdn.microsoft.com/en-us/library/t6fk7h29.aspx
			set_terminate(TerminateHandler);

			// Catch unexpected() calls.
			// In a multithreaded environment, unexpected functions are maintained 
			// separately for each thread. Each new thread needs to install its own 
			// unexpected function. Thus, each thread is in charge of its own unexpected handling.
			// http://msdn.microsoft.com/en-us/library/h46t5b69.aspx  
			set_unexpected(UnexpectedHandler);

			// Catch a floating point error
			typedef void(*sigh)(int);
			signal(SIGFPE, (sigh)SigfpeHandler);

			// Catch an illegal instruction
			signal(SIGILL, SigillHandler);

			// Catch illegal storage access errors
			signal(SIGSEGV, SigsegvHandler);
		}
	}
}