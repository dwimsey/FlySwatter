// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _CRT_SECURE_NO_WARNINGS
// we don't want this warnings, they really provide little usefulness and we're aware of how to code properly for them
#define _CRT_SECURE_NO_WARNINGS 1
#endif

#ifndef _CRT_NONSTDC_NO_DEPRECATE
// we don't care about these warnings either, we prefer compatibility with other compilers and OSes
#define _CRT_NONSTDC_NO_DEPRECATE 1
#endif

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>


#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <atlbase.h>
#include <atlstr.h>

#define FLYTRAP_VERSION_MAJOR 0
#define FLYTRAP_VERSION_MINOR 1
#define FLYTRAP_VERSION_PATCH 2
#define FLYTRAP_VERSION_BUILD 3
#define FLYTRAP_VERSION_STRING "0.1.2.3\0\0"

#ifndef FLYTRAP_API
//#if defined(_WIN32_WCE)
	#ifdef FLYTRAP_EXPORTS
//		#define FLYTRAP_API __declspec(dllexport)
		#define FLYTRAP_API
	#else
		#define FLYTRAP_API __declspec(dllimport)
	#endif
//#else
//	#define FLYTRAP_API
//#endif
#endif // !FLYTRAP_API

#include "client/windows/handler/exception_handler.h"
#include "client/windows/sender/crash_report_sender.h"
#include "client/windows/crash_generation/client_info.h"

using namespace google_breakpad;

#include "FlyTrap.h"

extern "C" {
	FLYTRAP_API int __stdcall FlyTrapInitClient(wchar_t *dump_path, wchar_t *reportUrl, wchar_t *OOPExePath);
	FLYTRAP_API int __stdcall FlyTrapShutdownClient(void *clientContext);
	FLYTRAP_API void * __stdcall FlyTrapInitServer(wchar_t *dumpPath, wchar_t *reportUrl, wchar_t *pipeName, wchar_t *serverReadyEventName);
	FLYTRAP_API int __stdcall FlyTrapShutdownServer(void *serverContext);
	FLYTRAP_API int __stdcall FlyTrapEnable();
	FLYTRAP_API int __stdcall FlyTrapDisable();
	FLYTRAP_API int __stdcall FlyTrapIsEnabled();
	FLYTRAP_API void __stdcall FlyTrapSetParam(const wchar_t *name, const wchar_t *value);
	FLYTRAP_API const wchar_t * __stdcall FlyTrapGetParam(const wchar_t *name);
	FLYTRAP_API void __stdcall FlyTrapTriggerReport(void);

	// This are used internally and need to be available to all bits
	int FlyTrapCrashAlert(void *parentWindowHandle, const wchar_t *reportUrl, const wchar_t *miniDumpFilename, const LPFLYTRAPPARAM params, const int params_len);
	bool FlyTrapExceptionFilter(void *ctx, EXCEPTION_POINTERS *exceptionInfo, MDRawAssertionInfo *assertionInfo);
	void FlyTrapOutOfProcessDumpCallback(void *dump_context, ClientInfo *client_info, const std::wstring *dump_path);
	bool FlyTrapInProcessDumpCallback(const wchar_t* dumpPath, const wchar_t* dumpId, void* ctx, EXCEPTION_POINTERS* exceptionInfo, MDRawAssertionInfo* assertionInfo, bool dumpSucceeded);

	unsigned char *base64encode(unsigned char *inBuf, size_t inSize, int lineSize);
	char *b64append(const char *inbuf, const char* format);

	void DeleteParamMap(map<wstring, wstring> *oldMap);
	map<wstring, wstring> *CreateParamMap(const LPFLYTRAPPARAM params, const int params_len, const wchar_t *dumpId, const wchar_t *reportUrl);
	wchar_t *ExpandEnvVarsInStr(const wchar_t *inStrPtr);
	wchar_t *mprintf(const wchar_t *format, ...);
}
