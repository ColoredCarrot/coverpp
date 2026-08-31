#ifndef _EXCEPTION_CODES_H
#define _EXCEPTION_CODES_H

// Article about these exception codes:		https://www.softwareverify.com/blog/exceptions-codes-youve-never-heard-of/
// Article about fail fast codes:			https://www.softwareverify.com/blog/fail-fast-codes/
// Current version of this header file:		https://www.softwareverify.com/blogDownloads/exceptionCodes.h

// See also https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/specific-exceptions

// Exception codes that we've found while developing tools at Software Verify (www.softwareverify.com)
// Some of these are documented in ntstatus.h, but depending on your Visual Studio version or other IDE you may not have these defined
// The rest we've found them working with compilers from Microsoft or Embarcadero, or in the .Net Core open source codebase.

// From ntstatus.h, see also https://codemachine.com/downloads/win81/ntstatus.h
//
// The success status codes 0 - 63 are reserved for wait completion status.
// FacilityCodes 0x5 - 0xF have been allocated by various drivers.
//
//
//  Values are 32 bit values laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//

// Put exception codes here for which there is no definition in an official header file from Microsoft etc.

// Visual Studio

#define SVL_THREAD_NAMING_EXCEPTION								0x406D1388	// MS Thread Naming Exception			(see https://blogs.msdn.microsoft.com/stevejs/2005/12/20/naming-threads-in-win32-and-net/)
#define SVL_MSVC_EXCEPTION										0xE06D7363	// C++ Exception Code					(see https://blogs.msdn.microsoft.com/oldnewthing/20100730-00/?p=13273)

// .Net

#define SVL_CLR_NET_FATAL_JIT_EXCEPTION							0x02345678	// CLR (.Net) Fatal JIT Exception Code			(see .Net Core open source code)	*
#define SVL_CLR_NET_CLRDBG_DATA_CHECKSUM_EXCEPTION				0x31415927	// CLR (.Net) Data Checksum Exception Code		(see .Net Core open source code)	*
#define SVL_CLRDBG_NOTIFICATION_EXCEPTION_CODE					0x04242420	// CLR (.Net) First Chance Exception Code
#define SVL_CLR_NET_BOOTUP_COMPLUS_EXCEPTION					0xC0020001	// CLR (.Net) Bootup COM+ Exception Code		(see .Net Core open source code)	*
#define SVL_CLR_NET_EXCEPTION									0xE0434F4D	// CLR (.Net) Exception Code
#define SVL_CLR_NET_COMPLUS_EXCEPTION							0xE0434352	// CLR (.Net) COM+ Exception Code				(see https://stackoverflow.com/questions/6244939/how-do-i-fix-a-net-windows-application-crashing-at-startup-with-exception-code and .Net Core open source code)
#define SVL_CLR_NET_HIJACK_EXCEPTION							0xE0434F4E	// CLR (.Net) HIJACK Exception Code				(see .Net Core open source code)
#define SVL_CLR_NET_CLRDATA_NOTIFY_EXCEPTION					0xE0444143	// CLR (.Net) Notify Exception Code				(see .Net Core open source code)	*
#define SVL_CLR_NET_EXX_EXCEPTION								0xE0455858	// CLR (.Net) EXX Exception Code				(see .Net Core open source code)
#define SVL_CLR_NET_SEH_VERIFICATION_EXCEPTION					0xE0564552	// CLR (.Net) SEH Verification Exception Code	(see .Net Core open source code)	*
#define SVL_CLR_NET_INTERNAL_ASSERT_EXCEPTION					0xE0584D4E	// CLR (.Net) Internal ASSERT Exception Code	(see .Net Core open source code)

// Embarcadero / Borland

#define SVL_DELPHI_EXCEPTION_1									0x0EEDFADE	// Delphi Exception Code				(see https://stackoverflow.com/questions/16602756/strange-0x0eedfade-exception-in-delphi-multi-thread-program)
#define SVL_DELPHI_EXCEPTION_2									0x0EEDFACE	// Delphi Exception Code				(see http://webfiles.icebreak.org/webfiles/CBuilderCD/v4/Runimage/Cbuilder4/Source/RTL/SOURCE/EXCEPT/EXCEPT.C)
#define SVL_CPP_BUILDER_EXCEPTION								0x0EEFFACE	// C++ Builder Exception Code			(see http://webfiles.icebreak.org/webfiles/CBuilderCD/v4/Runimage/Cbuilder4/Source/RTL/SOURCE/EXCEPT/EXCEPT.C)

// you can look up error codes and exceptions here
// https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
// https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/705fb797-2175-4a90-b5a3-3918024b10b8

#ifndef STATUS_VERIFIER_STOP
#define STATUS_VERIFIER_STOP									0xC0000421L
#endif	//STATUS_VERIFIER_STOP

#ifndef STATUS_APPLICATION_HANG
#define STATUS_APPLICATION_HANG									0xCFFFFFFFL
#endif	//STATUS_APPLICATION_HANG

#ifndef STATUS_FATAL_MEMORY_EXHAUSTION
#define STATUS_FATAL_MEMORY_EXHAUSTION							0xC00001ADL
#endif	//STATUS_FATAL_MEMORY_EXHAUSTION

#ifndef STATUS_FAIL_FAST_EXCEPTION
#define STATUS_FAIL_FAST_EXCEPTION								0xC0000602L
#endif	//STATUS_FAIL_FAST_EXCEPTION

#ifndef STATUS_HEAP_CORRUPTION
#define	STATUS_HEAP_CORRUPTION									0xC0000374L
#endif	//STATUS_HEAP_CORRUPTION

#ifndef STATUS_INVALID_EXCEPTION_HANDLER
#define STATUS_INVALID_EXCEPTION_HANDLER						0xC00001A5L
#endif	//STATUS_INVALID_EXCEPTION_HANDLER

#ifndef STATUS_UNWIND
#define STATUS_UNWIND											0xC0000027L
#endif	//STATUS_UNWIND

#ifndef STATUS_BAD_STACK
#define STATUS_BAD_STACK										0xC0000028L
#endif	//STATUS_BAD_STACK

#ifndef STATUS_INVALID_UNWIND_TARGET
#define STATUS_INVALID_UNWIND_TARGET							0xC0000029L
#endif	//STATUS_INVALID_UNWIND_TARGET

#ifndef STATUS_FATAL_USER_CALLBACK_EXCEPTION
#define STATUS_FATAL_USER_CALLBACK_EXCEPTION					0xC000041DL
#endif	//STATUS_FATAL_USER_CALLBACK_EXCEPTION

#ifndef STATUS_INVALID_IMAGE_FORMAT
#define STATUS_INVALID_IMAGE_FORMAT								0xC000007BL
#endif	//STATUS_INVALID_IMAGE_FORMAT

#ifndef STATUS_DEBUGGER_INACTIVE
#define STATUS_DEBUGGER_INACTIVE								0xC0000354L
#endif	//STATUS_DEBUGGER_INACTIVE

#ifndef CERT_E_EXPIRED
#define CERT_E_EXPIRED											0x800B0101L
#endif	//CERT_E_EXPIRED

#ifndef STATUS_WX86_SINGLE_STEP
#define STATUS_WX86_SINGLE_STEP									0x4000001EL
#endif	//STATUS_WX86_SINGLE_STEP

#ifndef STATUS_WX86_BREAKPOINT
#define STATUS_WX86_BREAKPOINT									0x4000001FL
#endif	//STATUS_WX86_BREAKPOINT

#ifndef DBG_REPLY_LATER
#define DBG_REPLY_LATER											0x40010001L
#endif	//DBG_REPLY_LATER

#ifndef DBG_UNABLE_TO_PROVIDE_HANDLE
#define DBG_UNABLE_TO_PROVIDE_HANDLE							0x40010002L
#endif	//DBG_UNABLE_TO_PROVIDE_HANDLE

#ifndef DBG_TERMINATE_THREAD
#define DBG_TERMINATE_THREAD									0x40010003L
#endif	//DBG_TERMINATE_THREAD

#ifndef DBG_TERMINATE_PROCESS
#define DBG_TERMINATE_PROCESS									0x40010004L
#endif	//DBG_TERMINATE_PROCESS

#ifndef DBG_CONTROL_C
#define DBG_CONTROL_C											0x40010005L
#endif	//DBG_CONTROL_C

#ifndef DBG_PRINTEXCEPTION_C
#define	DBG_PRINTEXCEPTION_C									0x40010006L
#endif	//DBG_PRINTEXCEPTION_C

#ifndef DBG_RIPEXCEPTION
#define DBG_RIPEXCEPTION										0x40010007L
#endif	//DBG_RIPEXCEPTION

#ifndef DBG_CONTROL_BREAK
#define DBG_CONTROL_BREAK										0x40010008L
#endif	//DBG_CONTROL_BREAK

#ifndef DBG_COMMAND_EXCEPTION
#define DBG_COMMAND_EXCEPTION									0x40010009L
#endif	//DBG_COMMAND_EXCEPTION

#ifndef DBG_PRINTEXCEPTION_WIDE_C
#define	DBG_PRINTEXCEPTION_WIDE_C								0x4001000AL
#endif	//DBG_PRINTEXCEPTION_WIDE_C

#ifndef DBG_EXCEPTION_NOT_HANDLED
#define	DBG_EXCEPTION_NOT_HANDLED								0x80010001L
#endif	//DBG_EXCEPTION_NOT_HANDLED

#ifndef STATUS_FATAL_APP_EXIT
#define STATUS_FATAL_APP_EXIT									0x40000015L
#endif	//STATUS_FATAL_APP_EXIT

#ifndef STATUS_STACK_BUFFER_OVERRUN
#define STATUS_STACK_BUFFER_OVERRUN								0xC0000409L
#endif	//STATUS_STACK_BUFFER_OVERRUN

#ifndef STATUS_INVALID_CRUNTIME_PARAMETER
#define STATUS_INVALID_CRUNTIME_PARAMETER						0xC0000417L
#endif	//STATUS_INVALID_CRUNTIME_PARAMETER

#ifndef STATUS_ASSERTION_FAILURE
#define STATUS_ASSERTION_FAILURE								0xC0000420L
#endif	//STATUS_ASSERTION_FAILURE

// codes found during testing, but no official documentation
// see also https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/18d8fbe8-a967-4f1c-ae50-99ca8e491d2d

#ifndef STATUS_MODULE_NOT_FOUND													// 0x0000007E ERROR_MOD_NOT_FOUND
#define STATUS_MODULE_NOT_FOUND									0x8007007EL		// this is a Software Verify definition and name for this exception code as we can't find an official name
#endif	//STATUS_MODULE_NOT_FOUND

#ifndef STATUS_PROCEDURE_NOT_FOUND												// 0x0000007F ERROR_PROC_NOT_FOUND
#define STATUS_PROCEDURE_NOT_FOUND								0xC06D007FL		// this is a Software Verify definition and name for this exception code as we can't find an official name
#endif	//#ifndef STATUS_PROCEDURE_NOT_FOUND									// https://github.com/google/googletest/issues/1277

#ifndef STATUS_BAD_EXE_FORMAT													// 0x000000C1 ERROR_BAD_EXE_FORMAT
#define STATUS_BAD_EXE_FORMAT									0x800700C1L		// this is a Software Verify definition and name for this exception code as we can't find an official name
#endif	//STATUS_BAD_EXE_FORMAT													// https://answers.microsoft.com/en-us/windows/forum/all/windows-10-not-updating-error-0x800700c1/7b31d836-6a3c-4848-aa64-a6bab7d9862d

#ifndef STATUS_SOFTWARE_RESTRICTION_POLICY										// 0x000004EC ERROR_ACCESS_DISABLED_BY_POLICY
#define STATUS_SOFTWARE_RESTRICTION_POLICY						0x800704ECL		// this is a Software Verify definition and name for this exception code as we can't find an official name
#endif	//STATUS_SOFTWARE_RESTRICTION_POLICY

#ifndef STATUS_RECURSION_TOO_DEEP_STACK_OVERFLOW								// 0x000003E9 STATUS_STACK_OVERFLOW
#define STATUS_RECURSION_TOO_DEEP_STACK_OVERFLOW				0x800703E9L		// this is a Software Verify definition and name for this exception code as we can't find an official name
#endif	//STATUS_RECURSION_TOO_DEEP_STACK_OVERFLOW

#ifndef STATUS_DOT_NET_CORE_RUNTIME_COMPATIBLE_FRAMEWORK						// 0x80008081
#define STATUS_DOT_NET_CORE_RUNTIME_COMPATIBLE_FRAMEWORK		0x80008081L		// this is a Software Verify definition and name for this exception code as we can't find an official name
#endif	//STATUS_DOT_NET_CORE_RUNTIME_COMPATIBLE_FRAMEWORK

#ifndef STATUS_DOT_NET_CORE_RUNTIME_LIB_LOAD_FAILURE							// 0x80008082 COREHOSTLIBLOADFAILURE
#define STATUS_DOT_NET_CORE_RUNTIME_LIB_LOAD_FAILURE			0x80008082L		// this is a Software Verify definition and name for this exception code as we can't find an official name
#endif	//STATUS_DOT_NET_CORE_RUNTIME_LIB_LOAD_FAILURE

#ifndef STATUS_DOT_NET_CORE_CANNOT_FIND_RUNTIME_TARGET							// 0x80008083
#define STATUS_DOT_NET_CORE_CANNOT_FIND_RUNTIME_TARGET			0x80008083L		// this is a Software Verify definition and name for this exception code as we can't find an official name
#endif	//STATUS_DOT_NET_CORE_CANNOT_FIND_RUNTIME_TARGET

#ifndef STATUS_DOT_NET_CORE_RUNTIME_VERSION_NOT_INSTALLED						// 0x80008096
#define STATUS_DOT_NET_CORE_RUNTIME_VERSION_NOT_INSTALLED		0x80008096L		// this is a Software Verify definition and name for this exception code as we can't find an official name
#endif	//STATUS_DOT_NET_CORE_RUNTIME_VERSION_NOT_INSTALLED

#endif	//_EXCEPTION_CODES_H
