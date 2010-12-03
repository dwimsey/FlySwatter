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

#include "client/windows/handler/exception_handler.h"
#include "client/windows/sender/crash_report_sender.h"
#include "client/windows/crash_generation/client_info.h"

using namespace google_breakpad;

#define FLYTRAP_IMPLICIT_DECLARATIONS 1
#include "FlyTrap.h"

extern "C" {


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
