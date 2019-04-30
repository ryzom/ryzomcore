// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "stdmisc.h"

#include "nel/misc/types_nl.h"

#ifdef NL_OS_WINDOWS
#	include <direct.h>
#	include <tchar.h>
#	include <imagehlp.h>
#	pragma comment(lib, "imagehlp.lib")
#	ifndef getcwd
#		define getcwd(_a, _b) (_getcwd(_a,_b))
#	endif
#	ifdef NL_OS_WIN64
#		define DWORD_TYPE DWORD64
#	else
#		define DWORD_TYPE DWORD
#	endif // NL_OS_WIN64
#elif defined NL_OS_UNIX
#	include <unistd.h>
#	define IsDebuggerPresent() false
#   ifndef NL_OS_MAC
#	    include <execinfo.h>
#   endif
//#	include <malloc.h>
#	include <errno.h>
#endif

#include "nel/misc/debug.h"

#ifdef HAVE_NELCONFIG_H
#  include "nelconfig.h"
#endif // HAVE_NELCONFIG_H

#include "nel/misc/log.h"
#include "nel/misc/displayer.h"
#include "nel/misc/mem_displayer.h"
#include "nel/misc/command.h"
#include "nel/misc/report.h"
#include "nel/misc/path.h"
#include "nel/misc/variable.h"
#include "nel/misc/system_info.h"
#include "nel/misc/system_utils.h"

#define NL_NO_DEBUG_FILES 1

using namespace std;

// If you don't want to add default displayer, put 0 instead of 1. In this case, you
// have to manage yourself displayer (in final release for example, we have to put 0)
// Alternatively, you can use --without-logging when using configure to set
// it to 0.
#ifndef NEL_DEFAULT_DISPLAYER
#define NEL_DEFAULT_DISPLAYER 1
#endif // NEL_DEFAULT_DISPLAYER

// Put 0 if you don't want to display in file "log.log"
// Alternatively, you can use --without-logging when using configure to set
// it to 0.
#ifndef NEL_LOG_IN_FILE
#define NEL_LOG_IN_FILE 1
#endif // NEL_LOG_IN_FILE

#define DEFAULT_DISPLAYER NEL_DEFAULT_DISPLAYER

#define LOG_IN_FILE NEL_LOG_IN_FILE

// If true, debug system will trap crash even if the application is in debugger
static const bool TrapCrashInDebugger = false;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

//
// Globals
//

bool DisableNLDebug= false;
NLMISC::CVariablePtr<bool> _DisableNLDebug("nel","DisableNLDebug","Disables generation and output of nldebug logs (no code associated with the log generation is executed)",&DisableNLDebug,true);

static std::string LogPath;

//bool DebugNeedAssert = false;
//bool NoAssert = false;


// ***************************************************************************
CImposterLog::CImposterLog(TAccessor accessor)
	: _Accessor(accessor)
{}

CLog* CImposterLog::operator -> ()
{
	if(NLMISC::INelContext::isContextInitialised())
	{
		return (NLMISC::INelContext::getInstance().*_Accessor)();
	}
	return NULL;
}

CImposterLog::operator CLog*()
{
	if(NLMISC::INelContext::isContextInitialised())
	{
		return (NLMISC::INelContext::getInstance().*_Accessor)();
	}
	return NULL;
}

CLog &CImposterLog::operator ()()
{
	return *(operator CLog*());
}


//CLog *ErrorLog = NULL;
CImposterLog	ErrorLog(&INelContext::getErrorLog);
//CLog *WarningLog = NULL;
CImposterLog	WarningLog(&INelContext::getWarningLog);
//CLog *InfoLog = NULL;
CImposterLog	InfoLog(&INelContext::getInfoLog);
//CLog *DebugLog = NULL;
CImposterLog	DebugLog(&INelContext::getDebugLog);
//CLog *AssertLog = NULL;
CImposterLog	AssertLog(&INelContext::getAssertLog);


// ***************************************************************************
CMemDisplayer *DefaultMemDisplayer = NULL;
CMsgBoxDisplayer *DefaultMsgBoxDisplayer = NULL;

static CStdDisplayer *sd = NULL;
static CFileDisplayer *fd = NULL;

static TCrashCallback CrashCallback = NULL;

void setCrashCallback(TCrashCallback crashCallback)
{
	CrashCallback = crashCallback;
}

// Yoyo: allow only the crash report to be emailed once
static bool	CrashAlreadyReported = false;
bool	isCrashAlreadyReported()
{
	return CrashAlreadyReported;
}
void	setCrashAlreadyReported(bool state)
{
	CrashAlreadyReported= state;
}


void setAssert (bool assert)
{
	INelContext::getInstance().setNoAssert(!assert);
}

void nlFatalError (const char *format, ...)
{
	char *str;
	NLMISC_CONVERT_VARGS (str, format, 256/*NLMISC::MaxCStringSize*/);

	INelContext::getInstance().setDebugNeedAssert( NLMISC::DefaultMsgBoxDisplayer == NULL );

	NLMISC::ErrorLog->displayNL (str);

	if (INelContext::getInstance().getDebugNeedAssert())
			NLMISC_BREAKPOINT;

#ifndef NL_OS_WINDOWS

	//	exit(EXIT_FAILURE);
	abort ();
#endif
}

void nlError (const char *format, ...)
{
	char *str;
	NLMISC_CONVERT_VARGS (str, format, 256/*NLMISC::MaxCStringSize*/);

	INelContext::getInstance().setDebugNeedAssert( NLMISC::DefaultMsgBoxDisplayer == NULL );

	NLMISC::ErrorLog->displayNL (str);

	if (INelContext::getInstance().getDebugNeedAssert())
		NLMISC_BREAKPOINT;

#ifndef NL_OS_WINDOWS
//	exit(EXIT_FAILURE);
	abort ();
#endif
}

// the default behavior is to display all in standard output and to a file named "log.log";

static void initDebug2 (bool logInFile)
{
#if DEFAULT_DISPLAYER

	// put the standard displayer everywhere

//#ifdef NL_DEBUG
	INelContext::getInstance().getDebugLog()->addDisplayer (sd);
//#endif // NL_DEBUG
	INelContext::getInstance().getInfoLog()->addDisplayer (sd);
	INelContext::getInstance().getWarningLog()->addDisplayer (sd);
	INelContext::getInstance().getAssertLog()->addDisplayer (sd);
	INelContext::getInstance().getErrorLog()->addDisplayer (sd);

	// put the memory displayer everywhere

	// use the memory displayer and bypass all filter (even for the debug mode)
	INelContext::getInstance().getDebugLog()->addDisplayer (DefaultMemDisplayer, true);
	INelContext::getInstance().getInfoLog()->addDisplayer (DefaultMemDisplayer, true);
	INelContext::getInstance().getWarningLog()->addDisplayer (DefaultMemDisplayer, true);
	INelContext::getInstance().getAssertLog()->addDisplayer (DefaultMemDisplayer, true);
	INelContext::getInstance().getErrorLog()->addDisplayer (DefaultMemDisplayer, true);

	// put the file displayer only if wanted

#if LOG_IN_FILE
	if (logInFile)
	{
//#ifdef NL_DEBUG
		INelContext::getInstance().getDebugLog()->addDisplayer (fd);
//#endif // NL_DEBUG
		INelContext::getInstance().getInfoLog()->addDisplayer (fd);
		INelContext::getInstance().getWarningLog()->addDisplayer (fd);
		INelContext::getInstance().getAssertLog()->addDisplayer (fd);
		INelContext::getInstance().getErrorLog()->addDisplayer (fd);
	}
#endif // LOG_IN_FILE

	// put the message box only in release for error

	if (DefaultMsgBoxDisplayer)
	{
		INelContext::getInstance().getAssertLog()->addDisplayer (DefaultMsgBoxDisplayer);
		INelContext::getInstance().getErrorLog()->addDisplayer (DefaultMsgBoxDisplayer);
	}

#endif // DEFAULT_DISPLAYER
}


// ***************************************************************************
// Method called when an assert arise

void _assertex_stop_0(bool &ignoreNextTime, sint line, const char *file, const char *funcName, const char *exp)
{
	INelContext::getInstance().setDebugNeedAssert( false );
	NLMISC::createDebug ();
	if (NLMISC::DefaultMsgBoxDisplayer)
		NLMISC::DefaultMsgBoxDisplayer->IgnoreNextTime = ignoreNextTime;
	else if(!INelContext::getInstance().getNoAssert())
		INelContext::getInstance().setDebugNeedAssert(true);
	NLMISC::AssertLog->setPosition (line, file, funcName);
	if(exp)		NLMISC::AssertLog->displayNL ("\"%s\" ", exp);
	else		NLMISC::AssertLog->displayNL ("STOP");
}

bool _assertex_stop_1(bool &ignoreNextTime)
{
	if (NLMISC::DefaultMsgBoxDisplayer)
		ignoreNextTime = NLMISC::DefaultMsgBoxDisplayer->IgnoreNextTime;
	return INelContext::getInstance().getDebugNeedAssert();
}

bool _assert_stop(bool &ignoreNextTime, sint line, const char *file, const char *funcName, const char *exp)
{
	_assertex_stop_0(ignoreNextTime, line, file, funcName, exp);
	return _assertex_stop_1(ignoreNextTime);
}


#ifdef NL_OS_WINDOWS

/*
// ***************************************************************************
static DWORD __stdcall GetModuleBase(HANDLE hProcess, DWORD dwReturnAddress)
{
	IMAGEHLP_MODULE moduleInfo;

	if (SymGetModuleInfo(hProcess, dwReturnAddress, &moduleInfo))
		return moduleInfo.BaseOfImage;
	else
	{
		MEMORY_BASIC_INFORMATION memoryBasicInfo;

		if (::VirtualQueryEx(hProcess, (LPVOID) dwReturnAddress,
			&memoryBasicInfo, sizeof(memoryBasicInfo)))
		{
			DWORD cch = 0;
			wchar_t szFile[MAX_PATH] = { 0 };

			cch = GetModuleFileNameW((HINSTANCE)memoryBasicInfo.AllocationBase,
								 szFile, MAX_PATH);

			if (cch && (lstrcmpA(szFile, "DBFN")== 0))
			{
				if (!SymLoadModule(hProcess,
				   NULL, "MN",
				   NULL, (DWORD) memoryBasicInfo.AllocationBase, 0))
				{
					DWORD dwError = GetLastError();
//					nlinfo("Error: %d", dwError);
				}
		}
		else
		{
		 if (!SymLoadModule(hProcess,
			   NULL, ((cch) ? szFile : NULL),
			   NULL, (DWORD) memoryBasicInfo.AllocationBase, 0))
			{
				DWORD dwError = GetLastError();
//				nlinfo("Error: %d", dwError);
			 }

		}

		 return (DWORD) memoryBasicInfo.AllocationBase;
	  }
//		else
//			nlinfo("Error is %d", GetLastError());
	}

	return 0;
}

LPVOID __stdcall FunctionTableAccess (HANDLE hProcess, DWORD AddrBase)
{
	AddrBase = 0x40291f;
	DWORD addr = SymGetModuleBase (hProcess, AddrBase);
	HRESULT hr = GetLastError ();

	IMAGEHLP_MODULE moduleInfo;
	moduleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE);
	SymGetModuleInfo(hProcess, addr, &moduleInfo);
	hr = GetLastError ();
	SymLoadModule(hProcess, NULL, NULL, NULL, 0, 0);
	hr = GetLastError ();

	LPVOID temp = SymFunctionTableAccess (hProcess, AddrBase);
	hr = GetLastError ();
	return temp;
}
*/

/* can't include dbghelp.h */
typedef struct _NEL_MINIDUMP_EXCEPTION_INFORMATION {  DWORD ThreadId;  PEXCEPTION_POINTERS ExceptionPointers;  BOOL ClientPointers;
} NEL_MINIDUMP_EXCEPTION_INFORMATION, *PNEL_MINIDUMP_EXCEPTION_INFORMATION;
typedef enum _NEL_MINIDUMP_TYPE
{
  MiniDumpNormal = 0x00000000,
  MiniDumpWithDataSegs = 0x00000001,
  MiniDumpWithFullMemory = 0x00000002,
  MiniDumpWithHandleData = 0x00000004,
  MiniDumpFilterMemory = 0x00000010,
  MiniDumpWithUnloaded = 0x00000020,
  MiniDumpWithIndirectlyReferencedMemory = 0x00000040,
  MiniDumpFilterModulePaths = 0x00000080,
  MiniDumpWithProcessThreadData = 0x00000100,
  MiniDumpWithPrivateReadWriteMemory = 0x00000200,
  MiniDumpWithoutOptionalData = 0x00000400,
  MiniDumpWithFullMemoryInfo = 0x00000800,
  MiniDumpWithThreadInfo = 0x00001000,
  MiniDumpWithCodeSegs = 0x00002000
} NEL_MINIDUMP_TYPE;

static void DumpMiniDump(PEXCEPTION_POINTERS excpInfo)
{
	HANDLE file = CreateFileA (NL_CRASH_DUMP_FILE, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file)
	{
		HMODULE hm = LoadLibraryA ("dbghelp.dll");
		if (hm)
		{
			BOOL (WINAPI* MiniDumpWriteDump)(
			  HANDLE hProcess,
			  DWORD ProcessId,
			  HANDLE hFile,
			  NEL_MINIDUMP_TYPE DumpType,
			  PNEL_MINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
			  PNEL_MINIDUMP_EXCEPTION_INFORMATION UserStreamParam,
			  PNEL_MINIDUMP_EXCEPTION_INFORMATION CallbackParam
			) = NULL;
			*(FARPROC*)&MiniDumpWriteDump = GetProcAddress(hm, "MiniDumpWriteDump");
			if (MiniDumpWriteDump)
			{
				// OutputDebugString(_T("writing minidump\r\n"));
				NEL_MINIDUMP_EXCEPTION_INFORMATION eInfo;
				eInfo.ThreadId = GetCurrentThreadId();
				eInfo.ExceptionPointers = excpInfo;
				eInfo.ClientPointers = FALSE;

				// note:  MiniDumpWithIndirectlyReferencedMemory does not work on Win98
				MiniDumpWriteDump(
					GetCurrentProcess(),
					GetCurrentProcessId(),
					file,
					MiniDumpNormal,
					excpInfo ? &eInfo : NULL,
					NULL,
					NULL);
			}
			else
			{
				nlwarning ("Can't get proc MiniDumpWriteDump in dbghelp.dll");
			}
		}
		else
		{
			nlwarning ("Can't load dbghelp.dll");
		}
		CloseHandle (file);
	}
	else
		nlwarning ("Can't create mini dump file");
}

class EDebug : public ETrapDebug
{
public:

	EDebug() { _Reason = "Nothing about EDebug"; }

	virtual ~EDebug() throw() {}

	EDebug(EXCEPTION_POINTERS * pexp) : m_pexp(pexp) { nlassert(pexp != 0); createWhat(); }
	EDebug(const EDebug& se) : m_pexp(se.m_pexp) { createWhat(); }

	void createWhat ()
	{
		string shortExc, longExc, subject;
		string addr, ext;
		ULONG_PTR skipNFirst = 0;
		_Reason.clear();

		if (m_pexp == NULL)
		{
			_Reason = "Unknown exception, don't have context.";
		}
		else
		{
			switch (m_pexp->ExceptionRecord->ExceptionCode)
			{
			case EXCEPTION_ACCESS_VIOLATION          : shortExc="Access Violation"; longExc="The thread attempted to read from or write to a virtual address for which it does not have the appropriate access";
				ext = ", thread attempts to ";
				ext += m_pexp->ExceptionRecord->ExceptionInformation[0]?"write":"read";
				if (m_pexp->ExceptionRecord->ExceptionInformation[1])
					ext += toString(" at 0x%X",m_pexp->ExceptionRecord->ExceptionInformation[1]);
				else
					ext += " at <NULL>";
				break;
			case EXCEPTION_DATATYPE_MISALIGNMENT     : shortExc="Datatype Misalignment"; longExc="The thread attempted to read or write data that is misaligned on hardware that does not provide alignment. For example, 16-bit values must be aligned on 2-byte boundaries, 32-bit values on 4-byte boundaries, and so on"; break;
			case EXCEPTION_BREAKPOINT                : shortExc="Breakpoint"; longExc="A breakpoint was encountered"; break;
			case EXCEPTION_SINGLE_STEP               : shortExc="Single Step"; longExc="A trace trap or other single-instruction mechanism signaled that one instruction has been executed"; break;
			case EXCEPTION_ARRAY_BOUNDS_EXCEEDED     : shortExc="Array Bounds Exceeded"; longExc="The thread attempted to access an array element that is out of bounds, and the underlying hardware supports bounds checking"; break;
			case EXCEPTION_FLT_DENORMAL_OPERAND      : shortExc="Float Denormal Operand"; longExc="One of the operands in a floating-point operation is denormal. A denormal value is one that is too small to represent as a standard floating-point value"; break;
			case EXCEPTION_FLT_DIVIDE_BY_ZERO        : shortExc="Float Divide By Zero"; longExc="The thread attempted to divide a floating-point value by a floating-point divisor of zero"; break;
			case EXCEPTION_FLT_INEXACT_RESULT        : shortExc="Float Inexact Result"; longExc="The result of a floating-point operation cannot be represented exactly as a decimal fraction"; break;
			case EXCEPTION_FLT_INVALID_OPERATION     : shortExc="Float Invalid Operation"; longExc="This exception represents any floating-point exception not included in this list"; break;
			case EXCEPTION_FLT_OVERFLOW              : shortExc="Float Overflow"; longExc="The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type"; break;
			case EXCEPTION_FLT_STACK_CHECK           : shortExc="Float Stack Check"; longExc="The stack overflowed or underflowed as the result of a floating-point operation"; break;
			case EXCEPTION_FLT_UNDERFLOW             : shortExc="Float Underflow"; longExc="The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type"; break;
			case EXCEPTION_INT_DIVIDE_BY_ZERO        : shortExc="Integer Divide By Zero"; longExc="The thread attempted to divide an integer value by an integer divisor of zero"; break;
			case EXCEPTION_INT_OVERFLOW              : shortExc="Integer Overflow"; longExc="The result of an integer operation caused a carry out of the most significant bit of the result"; break;
			case EXCEPTION_PRIV_INSTRUCTION          : shortExc="Privileged Instruction"; longExc="The thread attempted to execute an instruction whose operation is not allowed in the current machine mode"; break;
			case EXCEPTION_IN_PAGE_ERROR             : shortExc="In Page Error"; longExc="The thread tried to access a page that was not present, and the system was unable to load the page. -ie. the program or memory mapped file couldn't be paged in because it isn't accessable any more. Device drivers can return this exception if something went wrong with the read (i.e hardware problems)"; break;
			case EXCEPTION_ILLEGAL_INSTRUCTION       : shortExc="Illegal Instruction"; longExc="The thread tried to execute an invalid instruction -such as MMX opcodes on a non MMX system. Branching to an invalid location can cause this -something stack corruption often causes"; break;
			case EXCEPTION_NONCONTINUABLE_EXCEPTION  : shortExc="Noncontinuable Exception"; longExc="The thread attempted to continue execution after a noncontinuable exception occurred"; break;
			case EXCEPTION_STACK_OVERFLOW            : shortExc="Stack Overflow"; longExc="Stack overflow. Can occur during errant recursion, or when a function creates a particularly large array on the stack"; break;
			case EXCEPTION_INVALID_DISPOSITION       : shortExc="Invalid Disposition"; longExc="Whatever number the exception filter returned, it wasn't a value the OS knows about"; break;
			case EXCEPTION_GUARD_PAGE                : shortExc="Guard Page"; longExc="Memory Allocated as PAGE_GUARD by VirtualAlloc() has been accessed"; break;
			case EXCEPTION_INVALID_HANDLE            : shortExc="Invalid Handle"; longExc.clear(); break;
			case CONTROL_C_EXIT                      : shortExc="Control-C"; longExc="Lets the debugger know the user hit Ctrl-C. Seemingly for console apps only"; break;
			case STATUS_NO_MEMORY                    : shortExc="No Memory"; longExc="Called by HeapAlloc() if you specify HEAP_GENERATE_EXCEPTIONS and there is no memory or heap corruption";
				ext = ", unable to allocate ";
				ext += toString ("%d bytes", m_pexp->ExceptionRecord->ExceptionInformation [0]);
				break;
			case STATUS_WAIT_0                       : shortExc="Wait 0"; longExc.clear(); break;
			case STATUS_ABANDONED_WAIT_0             : shortExc="Abandoned Wait 0"; longExc.clear(); break;
			case STATUS_USER_APC                     : shortExc="User APC"; longExc="A user APC was delivered to the current thread before the specified Timeout interval expired"; break;
			case STATUS_TIMEOUT                      : shortExc="Timeout"; longExc.clear(); break;
			case STATUS_PENDING                      : shortExc="Pending"; longExc.clear(); break;
			case STATUS_SEGMENT_NOTIFICATION         : shortExc="Segment Notification"; longExc.clear(); break;
			case STATUS_FLOAT_MULTIPLE_FAULTS        : shortExc="Float Multiple Faults"; longExc.clear(); break;
			case STATUS_FLOAT_MULTIPLE_TRAPS         : shortExc="Float Multiple Traps"; longExc.clear(); break;
#ifdef NL_COMP_VC6
			case STATUS_ILLEGAL_VLM_REFERENCE        : shortExc="Illegal VLM Reference"; longExc.clear(); break;
#endif
			case 0xE06D7363                          : shortExc="Microsoft C++ Exception"; longExc="Microsoft C++ Exception"; break;	// cpp exception
			case 0xACE0ACE                           : shortExc.clear(); longExc.clear();
				if (m_pexp->ExceptionRecord->NumberParameters == 1)
					skipNFirst = m_pexp->ExceptionRecord->ExceptionInformation [0];
				break;	// just want the stack
			default                                  : shortExc="Unknown Exception"; longExc="Unknown Exception "+toString("0x%X", m_pexp->ExceptionRecord->ExceptionCode); break;
			};

			if(m_pexp->ExceptionRecord != NULL)
			{
				if (m_pexp->ExceptionRecord->ExceptionAddress)
					addr = toString(" at 0x%X", m_pexp->ExceptionRecord->ExceptionAddress);
				else
					addr = " at <NULL>";
			}

			string progname;
			if(!shortExc.empty() || !longExc.empty())
			{
				wchar_t name[1024];
				GetModuleFileNameW (NULL, name, 1023);
				progname = CFile::getFilename(wideToUtf8(name));
				progname += " ";
			}

			subject = progname + shortExc + addr;

			if (_Reason.empty())
			{
				if (!shortExc.empty()) _Reason += shortExc + " exception generated" + addr + ext + ".\n";
				if (!longExc.empty()) _Reason += longExc + ".\n";
			}

			// display the stack
			addStackAndLogToReason (skipNFirst);

			if(!shortExc.empty() || !longExc.empty())
			{
				// yoyo: allow only to send the crash report once. Because users usually click ignore,
				// which create noise into list of bugs (once a player crash, it will surely continues to do it).
				report(progname + shortExc, subject, _Reason, NL_CRASH_DUMP_FILE, true, !isCrashAlreadyReported(), ReportAbort);
				// TODO: Does this need to be synchronous? Why does this not handle the report result?

				// no more sent mail for crash
				setCrashAlreadyReported(true);
			}
		}
	}

	// display the callstack
	void addStackAndLogToReason (ULONG_PTR /* skipNFirst */ = 0)
	{
#ifdef NL_OS_WINDOWS
		// ace hack
/*		skipNFirst = 0;

		DWORD symOptions = SymGetOptions();
		symOptions |= SYMOPT_LOAD_LINES;
		symOptions &= ~SYMOPT_UNDNAME;
		SymSetOptions (symOptions);

		nlverify (SymInitialize(getProcessHandle(), NULL, FALSE) == TRUE);

		STACKFRAME callStack;
		::ZeroMemory (&callStack, sizeof(callStack));
		callStack.AddrPC.Mode      = AddrModeFlat;
		callStack.AddrPC.Offset    = m_pexp->ContextRecord->Eip;
		callStack.AddrStack.Mode   = AddrModeFlat;
		callStack.AddrStack.Offset = m_pexp->ContextRecord->Esp;
		callStack.AddrFrame.Mode   = AddrModeFlat;
		callStack.AddrFrame.Offset = m_pexp->ContextRecord->Ebp;

		_Reason += "\nCallstack:\n";
		_Reason += "-------------------------------\n";
		for (sint32 i = 0; ; i++)
		{
			SetLastError(0);
			BOOL res = StackWalk (IMAGE_FILE_MACHINE_I386, getProcessHandle(), GetCurrentThread(), &callStack,
				m_pexp->ContextRecord, NULL, FunctionTableAccess, GetModuleBase, NULL);

			if (res == FALSE || callStack.AddrFrame.Offset == 0)
				break;

			string symInfo, srcInfo;

			if (i >= skipNFirst)
			{
				srcInfo = getSourceInfo (callStack.AddrPC.Offset);
				symInfo = getFuncInfo (callStack.AddrPC.Offset, callStack.AddrFrame.Offset);
				_Reason += srcInfo + ": " + symInfo + "\n";
			}
		}
		SymCleanup(getProcessHandle());
		*/
#elif !defined(NL_OS_MAC)
		// Make place for stack frames and function names
		const uint MaxFrame=64;
		void *trace[MaxFrame];
		char **messages = (char **)NULL;
		int i, trace_size = 0;

		trace_size = backtrace(trace, MaxFrame);
		messages = backtrace_symbols(trace, trace_size);
		result += "Callstack:\n";
		_Reason += "-------------------------------\n";
		for (i=0; i<trace_size; ++i)
			_Reason += toString("%i : %s\n", i, messages[i]);
		// free the messages
		free(messages);
#endif

// 		_Reason += "-------------------------------\n";
// 		_Reason += "\n";
// 		if(DefaultMemDisplayer)
// 		{
// 			_Reason += "Log with no filter:\n";
// 			_Reason += "-------------------------------\n";
// 			DefaultMemDisplayer->write (_Reason);
// 		}
// 		else
// 		{
// 			_Reason += "No log\n";
// 		}
//		_Reason += "-------------------------------\n";

		// add specific information about the application
// 		if(CrashCallback)
// 		{
// 			_Reason += "User Crash Callback:\n";
// 			_Reason += "-------------------------------\n";
// 			static bool looping = false;
// 			if(looping)
// 			{
// 				_Reason += "******* WARNING: crashed in the user crash callback *******\n";
// 				looping = false;
// 			}
// 			else
// 			{
// 				looping = true;
// 				_Reason += CrashCallback();
// 				looping = false;
// 			}
// 			_Reason += "-------------------------------\n";
// 		}
	}

	string getSourceInfo (DWORD_TYPE addr)
	{
		string str;

		IMAGEHLP_LINE  line;
		::ZeroMemory (&line, sizeof (line));
		line.SizeOfStruct = sizeof(line);

		// ACE: removed the next code because "SymGetLineFromAddr" is not available on windows 98
		bool ok = false;
		DWORD displacement = 0 ;
		DWORD resdisp = 0;

//
		/*
		// "Debugging Applications" John Robbins
		// The problem is that the symbol engine finds only those source
		// line addresses (after the first lookup) that fall exactly on
		// a zero displacement. I'll walk backward 100 bytes to
		// find the line and return the proper displacement.
		bool ok = true;
		DWORD displacement = 0 ;
		DWORD resdisp;

		while (!SymGetLineFromAddr (getProcessHandle(), addr - displacement, (DWORD*)&resdisp, &line))
		{
			if (100 == ++displacement)
			{
				ok = false;
				break;
			}
		}
		*/
//

		// "Debugging Applications" John Robbins
		// I found the line, and the source line information is correct, so
		// change the displacement if I had to search backward to find the source line.
		if (displacement)
			resdisp = displacement;

		if (ok)
		{
			str = line.FileName;
			str += "(" + toString ((uint32)line.LineNumber) + ")";
			str += toString(": 0x%X", addr);
		}
		else
		{
			IMAGEHLP_MODULE module;
			::ZeroMemory (&module, sizeof(module));
			module.SizeOfStruct = sizeof(module);

			if (SymGetModuleInfo (getProcessHandle(), addr, &module))
			{
				str = module.ModuleName;
			}
			else
			{
				str = "<NoModule>";
			}

			str += toString("!0x%p", (void*)addr);
		}

//

		/*DWORD disp;
		if (SymGetLineFromAddr (getProcessHandle(), addr, &disp, &line))
		{
			str = line.FileName;
			str += "(" + toString (line.LineNumber) + ")";
		}
		else
		{*/
			IMAGEHLP_MODULE module;
			::ZeroMemory (&module, sizeof(module));
			module.SizeOfStruct = sizeof(module);

			if (SymGetModuleInfo (getProcessHandle(), addr, &module))
			{
				str = module.ModuleName;
			}
			else
			{
				str = "<NoModule>";
			}

			str += toString("!0x%p", (void*)addr);
		//}
		str +=" DEBUG:"+toString("0x%p", addr);

//

		return str;
	}

	HANDLE getProcessHandle()
	{
		return CSystemInfo::isNT()?GetCurrentProcess():(HANDLE)(uintptr_t)GetCurrentProcessId();
	}

	// return true if found
	bool findAndErase(string &str, const char *token, const char *replace = NULL)
	{
		string::size_type pos;
		if ((pos = str.find(token)) != string::npos)
		{
			str.erase (pos,strlen(token));
			if (replace != NULL)
				str.insert (pos, replace);
			return true;
		}
		else
			return false;
	}

	// remove space and const stuffs
	// rawType contains the type without anything (to compare with known type)
	// displayType contains the type without std:: and stl ugly things
	void cleanType(string &rawType, string &displayType)
	{
		while (findAndErase(rawType, "std::")) ;
		while (findAndErase(displayType, "std::")) ;

		while (findAndErase(rawType, "_STL::")) ;
		while (findAndErase(displayType, "_STL::")) ;

		while (findAndErase(rawType, "const")) ;

		while (findAndErase(rawType, " ")) ;

		while (findAndErase(rawType, "&")) ;

		// rename ugly stl type

		while (findAndErase(rawType, "classbasic_string<char,classchar_traits<char>,classallocator<char>>", "string")) ;
		while (findAndErase(displayType, "class basic_string<char,class char_traits<char>,class allocator<char> >", "string")) ;
		while (findAndErase(rawType, "classvector<char,class char_traits<char>,class allocator<char> >", "string")) ;
	}

	string getFuncInfo (uintptr_t funcAddr, uintptr_t stackAddr)
	{
		string str ("NoSymbol");

		DWORD symSize = 10000;
		PIMAGEHLP_SYMBOL  sym = (PIMAGEHLP_SYMBOL) GlobalAlloc (GMEM_FIXED, symSize);
		::ZeroMemory (sym, symSize);
		sym->SizeOfStruct = symSize;
		sym->MaxNameLength = symSize - sizeof(IMAGEHLP_SYMBOL);

		DWORD_TYPE disp = 0;
		if (SymGetSymFromAddr (getProcessHandle(), funcAddr, &disp, sym) == FALSE)
		{
			return str;
		}

		CHAR undecSymbol[1024];
		if (UnDecorateSymbolName (sym->Name, undecSymbol, 1024, UNDNAME_COMPLETE | UNDNAME_NO_THISTYPE | UNDNAME_NO_SPECIAL_SYMS | UNDNAME_NO_MEMBER_TYPE | UNDNAME_NO_MS_KEYWORDS | UNDNAME_NO_ACCESS_SPECIFIERS ) > 0)
		{
			str = undecSymbol;
		}
		else if (SymUnDName (sym, undecSymbol, 1024) == TRUE)
		{
			str = undecSymbol;
		}

		// replace param with the value of the stack for this param

		string parse = str;
		str.clear();
		uint pos2 = 0;
		sint stop = 0;

		string type;

		string::size_type i = parse.find ("(");

		// copy the function name
		str = parse.substr(0, i);

//		nlinfo ("not parsed '%s'", parse.c_str());

		// if there s parameter, parse them
		if(i!=string::npos)
		{
			// copy the '('
			str += parse[i];
			for (i++; i < parse.size (); i++)
			{
				if (parse[i] == '<')
					 stop++;
				if (parse[i] == '>')
					 stop--;

				if (stop==0 && (parse[i] == ',' || parse[i] == ')'))
				{
					uintptr_t *addr = (uintptr_t*)(stackAddr) + 2 + pos2++;

					string displayType = type;
					cleanType (type, displayType);

					char tmp[1024];
					if(type == "void")
					{
						tmp[0]='\0';
					}
					else if(type == "int")
					{
						if (!IsBadReadPtr(addr,sizeof(int)))
							sprintf (tmp, "%p", (void *)(*addr));
					}
					else if (type == "char")
					{
						if (!IsBadReadPtr(addr,sizeof(char)))
							if (nlisprint(*addr))
							{
								sprintf (tmp, "'%c'", (char)((*addr) & 0xFF));
							}
							else
							{
								sprintf (tmp, "%p", (void *)(*addr));
							}
					}
					else if (type == "char*")
					{
						if (!IsBadReadPtr(addr,sizeof(char*)) && *addr != 0)
						{
							if (!IsBadStringPtrA((char*)*addr,32))
							{
								uint pos = 0;
								tmp[pos++] = '\"';
								for (uint j = 0; j < 32; j++)
								{
									char c = ((char *)*addr)[j];
									if (c == '\0')
										break;
									else if (c == '\n')
									{
										tmp[pos++] = '\\';
										tmp[pos++] = 'n';
									}
									else if (c == '\r')
									{
										tmp[pos++] = '\\';
										tmp[pos++] = 'r';
									}
									else if (c == '\t')
									{
										tmp[pos++] = '\\';
										tmp[pos++] = 't';
									}
									else
										tmp[pos++] = c;
								}
								tmp[pos++] = '\"';
								tmp[pos++] = '\0';
							}
						}
					}
					else if (type == "string") // we assume a string is always passed by reference (i.e. addr is a string**)
					{
						if (!IsBadReadPtr(addr,sizeof(string*)))
						{
							if (*addr != 0)
							{
								if (!IsBadReadPtr((void*)*addr,sizeof(string)))
									sprintf (tmp, "\"%s\"", ((string*)*addr)->c_str());
							}
						}
					}
					else
					{
						if (!IsBadReadPtr(addr,sizeof(uintptr_t*)))
						{
							if(*addr == 0)
								sprintf (tmp, "<NULL>");
							else
								sprintf (tmp, "0x%p", (void *)*addr);
						}
					}

					str += displayType;
					if(tmp[0]!='\0')
					{
						str += "=";
						str += tmp;
					}
					str += parse[i];
					type.clear();
				}
				else
				{
					type += parse[i];
				}
			}
			GlobalFree (sym);
			if (disp != 0)
			{
				str += " + ";
				str += toString ((uintptr_t)disp);
				str += " bytes";
			}
		}

//		nlinfo ("after parsing '%s'", str.c_str());

		return str;
	}

private:
	EXCEPTION_POINTERS * m_pexp;
};

// workaround of VCPP synchronous exception and se translator
bool global_force_exception_flag = false;
#define WORKAROUND_VCPP_SYNCHRONOUS_EXCEPTION  if (global_force_exception_flag) force_exception_frame();
void force_exception_frame(...) {std::cout.flush();}

static void exceptionTranslator(unsigned, EXCEPTION_POINTERS *pexp)
{
#ifndef NL_NO_DEBUG_FILES
	CFile::createEmptyFile(getLogDirectory() + "exception_catched");
#endif
	if (pexp->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT)
	{
#ifndef NL_NO_DEBUG_FILES
		CFile::createEmptyFile(getLogDirectory() + "breakpointed");
#endif
		return;
	}
#if FINAL_VERSION
	// In final version, throw EDebug to display a smart dialog box with callstack & log when crashing
#	pragma message ( "Smart crash enabled" )
	DumpMiniDump(pexp);
	throw EDebug (pexp);
#else
	// In debug version, let the program crash and use a debugger (clicking "Cancel")
	// Ace: 'if' not activated because we can't debug if enabled: keeping only 0xACE0ACE for nlstop...
	//if (!TrapCrashInDebugger && IsDebuggerPresent ())
	{
		if (pexp->ExceptionRecord->ExceptionCode == 0xACE0ACE)
			throw EDebug (pexp);
		else
			return;
	}
	/*else
	{
		if (pexp->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT)
			return;
		else
			throw EDebug (pexp);
	}*/
#endif
}

#endif // NL_OS_WINDOWS

void getCallStack(std::string &result, sint skipNFirst)
{
#ifdef NL_OS_WINDOWS
	try
	{
		WORKAROUND_VCPP_SYNCHRONOUS_EXCEPTION // force to install a exception frame

		DWORD_PTR array[1];
		array[0] = skipNFirst;
		RaiseException (0xACE0ACE, 0, 1, array);
	}
	catch (const EDebug &e)
	{
		result += e.what();
	}
#elif !defined(NL_OS_MAC)
	// Make place for stack frames and function names
	const uint MaxFrame=64;
	void *trace[MaxFrame];
	char **messages = (char **)NULL;
	int i, trace_size = 0;

	// on mac, require at least os 10.5
	trace_size = backtrace(trace, MaxFrame);
	messages = backtrace_symbols(trace, trace_size);
	result += "Dumping call stack :\n";
	for (i=0; i<trace_size; ++i)
		result += toString("%i : %s\n", i, messages[i]);
	// free the messages
	free(messages);
#endif
}


void getCallStackAndLog (string &result, sint /* skipNFirst */)
{
	//getCallStack(result, skipNFirst);
//#ifdef NL_OS_WINDOWS
//	try
//	{
//		WORKAROUND_VCPP_SYNCHRONOUS_EXCEPTION // force to install a exception frame
//
//		DWORD array[1];
//		array[0] = skipNFirst;
//		RaiseException (0xACE0ACE, 0, 1, array);
//	}
//	catch (const EDebug &e)
//	{
//		result += e.what();
//	}
//#else
//
//	// Make place for stack frames and function names
//	const uint MaxFrame=64;
//	void *trace[MaxFrame];
//	char **messages = (char **)NULL;
//	int i, trace_size = 0;
//
//	trace_size = backtrace(trace, MaxFrame);
//	messages = backtrace_symbols(trace, trace_size);
//	result += "Dumping call stack :\n";
//	for (i=0; i<trace_size; ++i)
//		result += toString("%i : %s\n", i, messages[i]);
//	// free the messages
//	free(messages);
//#endif
//
	result += "-------------------------------\n";
	result += "\n";
	if(DefaultMemDisplayer)
	{
		result += "Log with no filter:\n";
		result += "-------------------------------\n";
		DefaultMemDisplayer->write (result);
	}
	else
	{
		result += "No log\n";
	}
	result += "-------------------------------\n";

	// add specific information about the application
	if(CrashCallback)
	{
		result += "User Crash Callback:\n";
		result += "-------------------------------\n";
		static bool looping = false;
		if(looping)
		{
			result += "******* WARNING: crashed in the user crash callback *******\n";
			looping = false;
		}
		else
		{
			looping = true;
			result += CrashCallback();
			looping = false;
		}
		result += "-------------------------------\n";
	}
}

void changeLogDirectory(const std::string &dir)
{
	if (fd == NULL)return;
	LogPath = CPath::standardizePath(dir);
	string p = LogPath + "log.log";
	fd->setParam(p);
}

std::string getLogDirectory()
{
	return LogPath;
}

// You should not call this, unless you know what you're trying to do (it kills debug/log)!
// Destroys debug environment, to clear up the memleak log.
// NeL context must be deleted immediately after debug destroyed,
// or there will be various issues when static destructors call nldebug etc...
void destroyDebug()
{
	delete sd; sd = NULL;
	delete DefaultMsgBoxDisplayer; DefaultMsgBoxDisplayer = NULL;
	delete fd; fd = NULL;
	delete DefaultMemDisplayer; DefaultMemDisplayer = NULL;
	if (INelContext::isContextInitialised())
	{
		CLog *log;
		INelContext &context = INelContext::getInstance();
		log = context.getErrorLog(); context.setErrorLog(NULL); delete log; log = NULL;
		log = context.getWarningLog(); context.setWarningLog(NULL); delete log; log = NULL;
		log = context.getInfoLog(); context.setInfoLog(NULL); delete log; log = NULL;
		log = context.getDebugLog(); context.setDebugLog(NULL); delete log; log = NULL;
		log = context.getAssertLog(); context.setAssertLog(NULL); delete log; log = NULL;
		INelContext::getInstance().setAlreadyCreateSharedAmongThreads(false);
	}
}

void createDebug (const char *logPath, bool logInFile, bool eraseLastLog)
{
	// Do some basic compiler time check on type size
	nlctassert(sizeof(char) == 1);

//	static bool alreadyCreateSharedAmongThreads = false;
//	if ( !alreadyCreateSharedAmongThreads )
	if (!INelContext::getInstance().getAlreadyCreateSharedAmongThreads())
	{
		// Debug Info for mutexes
#ifdef MUTEX_DEBUG
		initAcquireTimeMap();
#endif

#ifndef NL_COMP_MINGW
#	ifdef NL_OS_WINDOWS
//		if (!IsDebuggerPresent ())
		{
			// Use an environment variable to share the value among the EXE and its child DLLs
			// (otherwise there would be one distinct bool by module, and the last
			// _set_se_translator would overwrite the previous ones)
			const char *SE_TRANSLATOR_IN_MAIN_MODULE = "NEL_SE_TRANS";
			char envBuf [2];
			if ( GetEnvironmentVariableA( SE_TRANSLATOR_IN_MAIN_MODULE, envBuf, 2 ) == 0)
			{
				_set_se_translator(exceptionTranslator);
				SetEnvironmentVariableA( SE_TRANSLATOR_IN_MAIN_MODULE, "1" );
			}
		}
#	endif // NL_OS_WINDOWS
#endif //!NL_COMP_MINGW

		INelContext::getInstance().setErrorLog(new CLog (CLog::LOG_ERROR));
		INelContext::getInstance().setWarningLog(new CLog (CLog::LOG_WARNING));
		INelContext::getInstance().setInfoLog(new CLog (CLog::LOG_INFO));
		INelContext::getInstance().setDebugLog(new CLog (CLog::LOG_DEBUG));
		INelContext::getInstance().setAssertLog(new CLog (CLog::LOG_ASSERT));

		sd = new CStdDisplayer ("DEFAULT_SD");

#ifdef NL_OS_WINDOWS
		if (TrapCrashInDebugger || !IsDebuggerPresent ())
#endif
		{
			DefaultMsgBoxDisplayer = new CMsgBoxDisplayer ("DEFAULT_MBD");
		}

#if LOG_IN_FILE
		if (logInFile)
		{
			string fn;
			if (logPath != NULL)
			{
				LogPath = CPath::standardizePath(logPath);
				fn += LogPath;
			}
			else
			{
// we want the log.log to be in the current directory
//				char	tmpPath[1024];
//				fn += getcwd(tmpPath, 1024);
//				fn += "/";
			}
			fn += "log.log";
#if FINAL_VERSION
			fd = new CFileDisplayer (fn, true, "DEFAULT_FD");
#else // FINAL_VERSION
			fd = new CFileDisplayer (fn, eraseLastLog, "DEFAULT_FD");
#endif // FINAL_VERSION
		}
#endif // LOG_IN_FILE
		DefaultMemDisplayer = new CMemDisplayer ("DEFAULT_MD");

		if (NLMISC::CSystemUtils::detectWindowedApplication())
			INelContext::getInstance().setWindowedApplication(true);

		initDebug2(logInFile);

		INelContext::getInstance().setAlreadyCreateSharedAmongThreads(true);
//		alreadyCreateSharedAmongThreads = true;
	}
}


/*
 * Beep (Windows only, no effect elsewhere)
 */
void beep( uint freq, uint duration )
{
#ifdef NL_OS_WINDOWS
	Beep( freq, duration );
#endif
}


//
// Instance counter
//

NLMISC_SAFE_SINGLETON_IMPL(CInstanceCounterManager);

CInstanceCounterLocalManager *CInstanceCounterLocalManager::_Instance = NULL;

TInstanceCounterData::TInstanceCounterData(const char *className)
:	_InstanceCounter(0),
	_DeltaCounter(0),
	_ClassName(className),
	_Touched(false)
{
	CInstanceCounterLocalManager::getInstance().registerInstanceCounter(this);
}

TInstanceCounterData::~TInstanceCounterData()
{
	CInstanceCounterLocalManager::getInstance().unregisterInstanceCounter(this);
}


void CInstanceCounterManager::registerInstaceCounterLocalManager(CInstanceCounterLocalManager *localMgr)
{
	_InstanceCounterMgrs.insert(localMgr);
}

void CInstanceCounterManager::unregisterInstaceCounterLocalManager(CInstanceCounterLocalManager *localMgr)
{
	_InstanceCounterMgrs.erase(localMgr);
}


std::string CInstanceCounterManager::displayCounters() const
{
	map<string, TInstanceCounterData> counters;

	{
		// gather counter information
		std::set<CInstanceCounterLocalManager*>::const_iterator first2(_InstanceCounterMgrs.begin()), last2(_InstanceCounterMgrs.end());
		for (; first2 != last2; ++first2)
		{
			// iterate over managers
			const CInstanceCounterLocalManager *mgr = *first2;
			{
				std::set<TInstanceCounterData*>::const_iterator first(mgr->_InstanceCounters.begin()), last(mgr->_InstanceCounters.end());
				for (; first != last; ++first)
				{
					const TInstanceCounterData *icd = *first;

					if (!icd->_Touched)
						continue;

					if( counters.find(icd->_ClassName) == counters.end())
					{
						// insert a new item
						counters.insert(make_pair(string(icd->_ClassName), TInstanceCounterData(*icd)));
					}
					else
					{
						// accumulate the counter with the existing counter
						TInstanceCounterData &icddest = counters.find(icd->_ClassName)->second;


						icddest._DeltaCounter += icd->_DeltaCounter;
						icddest._InstanceCounter += icd->_InstanceCounter;
					}

				}
			}

		}
	}

	string ret = toString("Listing %u Instance counters :\n", counters.size());
	map<string, TInstanceCounterData>::iterator first(counters.begin()), last(counters.end());
	for (; first != last; ++first)
	{
		TInstanceCounterData &icd = first->second;
		ret += toString("  Class '%-20s', \t%10d instances, \t%10d delta\n",
			icd._ClassName,
			icd._InstanceCounter,
			icd._InstanceCounter - icd._DeltaCounter);
	}

	return ret;
}

void CInstanceCounterManager::resetDeltaCounter()
{
	std::set<CInstanceCounterLocalManager*>::iterator first2(_InstanceCounterMgrs.begin()), last2(_InstanceCounterMgrs.end());
	for (; first2 != last2; ++first2)
	{
		// iterate over managers
		CInstanceCounterLocalManager *mgr = *first2;
		{
			std::set<TInstanceCounterData*>::iterator first(mgr->_InstanceCounters.begin()), last(mgr->_InstanceCounters.end());
			for (; first != last; ++first)
			{
				TInstanceCounterData *icd = *first;

				icd->_DeltaCounter = icd->_InstanceCounter;
			}
		}
	}
}

uint32 CInstanceCounterManager::getInstanceCounter(const std::string &className) const
{
	uint32 result = 0;
	std::set<CInstanceCounterLocalManager*>::const_iterator first2(_InstanceCounterMgrs.begin()), last2(_InstanceCounterMgrs.end());
	for (; first2 != last2; ++first2)
	{
		// iterate over managers
		const CInstanceCounterLocalManager *mgr = *first2;
		{
			std::set<TInstanceCounterData*>::const_iterator first(mgr->_InstanceCounters.begin()), last(mgr->_InstanceCounters.end());
			for (; first != last; ++first)
			{
				const TInstanceCounterData *icd = *first;

				if (icd->_ClassName == className)
				{
					result += icd->_InstanceCounter;
				}
			}
		}
	}

	return result;
}

sint32 CInstanceCounterManager::getInstanceCounterDelta(const std::string &className) const
{
	sint32 result = 0;
	std::set<CInstanceCounterLocalManager*>::const_iterator first2(_InstanceCounterMgrs.begin()), last2(_InstanceCounterMgrs.end());
	for (; first2 != last2; ++first2)
	{
		// iterate over managers
		const CInstanceCounterLocalManager *mgr = *first2;
		{
			std::set<TInstanceCounterData*>::const_iterator first(mgr->_InstanceCounters.begin()), last(mgr->_InstanceCounters.end());
			for (; first != last; ++first)
			{
				const TInstanceCounterData *icd = *first;

				if (icd->_ClassName == className)
				{
					result += icd->_InstanceCounter - icd->_DeltaCounter;
				}
			}
		}
	}

	return result;
}

void CInstanceCounterLocalManager::unregisterInstanceCounter(TInstanceCounterData *counter)
{
	_InstanceCounters.erase(counter);

	if (_InstanceCounters.empty())
	{
		// no more need for the singleton
		releaseInstance();
	}
}


/// Return the last error code generated by a system call
int getLastError()
{
#ifdef NL_OS_WINDOWS
	return GetLastError();
#else
	return errno;
#endif
}

/// Return a readable text according to the error code submited
std::string formatErrorMessage(int errorCode)
{
#ifdef NL_OS_WINDOWS
	LPWSTR lpMsgBuf = NULL;
	DWORD len = FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPWSTR)(&lpMsgBuf),
		0,
		NULL
	);

	// empty buffer, an error occurred
	if (len == 0) return toString("FormatMessage returned error %d", getLastError());

	// convert wchar_t* to std::string
	string ret = wideToUtf8(lpMsgBuf);

	// Free the buffer.
	LocalFree(lpMsgBuf);

	return ret;
#else
	return strerror(errorCode);
#endif
}



//
// Commands
//

NLMISC_CATEGORISED_COMMAND(nel, displayInstanceCounter, "display the instance counters", "[<filter>]")
{
	string className;
	if (args.size() == 1)
		className = args[0];
	if (args.size() > 1)
		return false;

	string list = CInstanceCounterManager::getInstance().displayCounters();

	vector<string> lines;
	explode(list, string("\n"), lines);


	for (uint i=0; i<lines.size(); ++i)
	{
		if (!className.empty())
		{
			if (lines[i].find(className) == string::npos)
				continue;
		}

		log.displayNL(lines[i].c_str());
	}

	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, resetInstanceCounterDelta, "reset the delta value for instance counter", "")
{
	CInstanceCounterManager::getInstance().resetDeltaCounter();

	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, displayMemlog, "displays the last N line of the log in memory", "[<NbLines>]")
{
	uint nbLines;

	if (args.empty()) nbLines = 100;
	else if (args.size() == 1) NLMISC::fromString(args[0], nbLines);
	else return false;

	if (DefaultMemDisplayer == NULL) return false;

	deque<string>::const_iterator it;

	const deque<string> &str = DefaultMemDisplayer->lockStrings ();

	if (nbLines >= str.size())
		it = str.begin();
	else
		it = str.end() - nbLines;

	DefaultMemDisplayer->write (&log);

	DefaultMemDisplayer->unlockStrings ();

	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, resetFilters, "disable all filters on Nel loggers", "[debug|info|warning|error|assert]")
{
	if(args.empty())
	{
		DebugLog->resetFilters();
		InfoLog->resetFilters();
		WarningLog->resetFilters();
		ErrorLog->resetFilters();
		AssertLog->resetFilters();
	}
	else if (args.size() == 1)
	{
		if (args[0] == "debug") DebugLog->resetFilters();
		else if (args[0] == "info") InfoLog->resetFilters();
		else if (args[0] == "warning") WarningLog->resetFilters();
		else if (args[0] == "error") ErrorLog->resetFilters();
		else if (args[0] == "assert") AssertLog->resetFilters();
	}
	else
	{
		return false;
	}

	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, addPositiveFilterDebug, "add a positive filter on DebugLog", "<filterstr>")
{
	if(args.size() != 1) return false;
	DebugLog->addPositiveFilter( args[0].c_str() );
	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, addNegativeFilterDebug, "add a negative filter on DebugLog", "<filterstr>")
{
	if(args.size() != 1) return false;
	DebugLog->addNegativeFilter( args[0].c_str() );
	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, removeFilterDebug, "remove a filter on DebugLog", "[<filterstr>]")
{
	if(args.empty())
		DebugLog->removeFilter();
	else if(args.size() == 1)
		DebugLog->removeFilter( args[0].c_str() );
	else return false;
	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, displayFilterDebug, "display filter on DebugLog", "")
{
	if(!args.empty()) return false;
	DebugLog->displayFilter(log);
	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, addPositiveFilterInfo, "add a positive filter on InfoLog", "<filterstr>")
{
	if(args.size() != 1) return false;
	InfoLog->addPositiveFilter( args[0].c_str() );
	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, addNegativeFilterInfo, "add a negative filter on InfoLog", "<filterstr>")
{
	if(args.size() != 1) return false;
	InfoLog->addNegativeFilter( args[0].c_str() );
	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, removeFilterInfo, "remove a filter on InfoLog", "[<filterstr>]")
{
	if(args.empty())
		InfoLog->removeFilter();
	else if(args.size() == 1)
		InfoLog->removeFilter( args[0].c_str() );
	else return false;
	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, displayFilterInfo, "display filter on InfoLog", "[d|i|w|e]")
{
	if(args.size() > 1) return false;
	if ( args.size() == 1 )
	{
		if ( strcmp( args[0].c_str(), "d" ) == 0 )
			InfoLog->displayFilter(*DebugLog);
		else if ( strcmp( args[0].c_str(), "i" ) == 0 )
			InfoLog->displayFilter(*InfoLog);
		else if ( strcmp( args[0].c_str(), "w" ) == 0 )
			InfoLog->displayFilter(*WarningLog);
		else if ( strcmp( args[0].c_str(), "e" ) == 0 )
			InfoLog->displayFilter(*ErrorLog);
		else
			return false;
	}
	else
	{
		InfoLog->displayFilter(log);
	}
	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, addPositiveFilterWarning, "add a positive filter on WarningLog", "<filterstr>")
{
	if(args.size() != 1) return false;
	WarningLog->addPositiveFilter( args[0].c_str() );
	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, addNegativeFilterWarning, "add a negative filter on WarningLog", "<filterstr>")
{
	if(args.size() != 1) return false;
	WarningLog->addNegativeFilter( args[0].c_str() );
	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, removeFilterWarning, "remove a filter on WarningLog", "[<filterstr>]")
{
	if(args.empty())
		WarningLog->removeFilter();
	else if(args.size() == 1)
		WarningLog->removeFilter( args[0].c_str() );
	else return false;
	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, displayFilterWarning, "display filter on WarningLog", "")
{
	if(!args.empty()) return false;
	WarningLog->displayFilter(log);
	return true;
}


#if !FINAL_VERSION

// commands to generate different "crash"

NLMISC_CATEGORISED_COMMAND(nel, assert, "generate a failed nlassert()", "")
{
	if(args.size() != 0) return false;
	nlassertex (false, ("Assert generated by the assert command"));
	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, stop, "generate a nlstop()", "")
{
	if(args.size() != 0) return false;
	nlstopex (("Stop generated by the stop command"));
	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, abort, "generate a abort()", "")
{
	if(args.size() != 0) return false;
	abort();
	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, divbyzero, "generate a divide by zero", "")
{
	if(args.size() != 0) return false;
	float a=10,b=0;
	a /= b;
	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, writeaccess, "write a uint8 value in an invalid address", "[<adr> [<value>]]")
{
	uint8 val = 123;
	uint8 *adr = (uint8*)0;
	if(args.size() >= 1)
	{
#ifdef HAVE_X86_64
		uint64 addr64;
		NLMISC::fromString(args[0], addr64);
		adr = (uint8*)addr64;
#else
		uint32 addr32;
		NLMISC::fromString(args[0], addr32);
		adr = (uint8*)addr32;
#endif
	}
	if(args.size() >= 2) NLMISC::fromString(args[1], val);
	*adr = val;
	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, readaccess, "read a uint8 value in an invalid address", "[<adr>]")
{
	uint8 val;
	uint8 *adr = (uint8*)0;
	if(args.size() == 1)
	{
#ifdef HAVE_X86_64
		uint64 addr64;
		NLMISC::fromString(args[0], addr64);
		adr = (uint8*)addr64;
#else
		uint32 addr32;
		NLMISC::fromString(args[0], addr32);
		adr = (uint8*)addr32;
#endif
	}
	val = *adr;
	log.displayNL("value is %hu", (uint16)val);
	return true;
}

#endif // FINAL_VERSION

} // NLMISC
