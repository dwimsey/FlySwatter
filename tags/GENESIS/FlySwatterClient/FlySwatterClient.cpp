// FlySwatter.cpp : Defines the exported functions for the DLL application.
//

#ifndef FLYSWATTER_API
//#if defined(_WIN32_WCE)
	#ifdef FLYSWATTER_EXPORTS
		#define FLYSWATTER_API __declspec(dllexport)
	#else
		#define FLYSWATTER_API __declspec(dllimport)
	#endif
//#else
//	#define FLYSWATTER_API
//#endif
#endif // !FLYSWATTER_API

#include "stdafx.h"

#include "FlySwatterClient.h"

#include "client/windows/handler/exception_handler.h"
#include "client/windows/sender/crash_report_sender.h"
using namespace google_breakpad;

// these are in librtslocalize, which we don't want to include just yet
#ifdef WIN32
#define tstrcpy wcscpy
#define tstrdup _wcsdup
#endif

extern "C" {
	FLYSWATTER_API int FlySwatterInit(TCHAR *dump_path, TCHAR *reportUrl);
	FLYSWATTER_API int FlySwatterEnable();
	FLYSWATTER_API int FlySwatterDisable();
	FLYSWATTER_API int FlySwatterIsEnabled();

	bool FlySwatterExceptionFilter(void *ctx, EXCEPTION_POINTERS *exceptionInfo, MDRawAssertionInfo *assertionInfo);
	bool FlySwatterMiniDumpCallback(const wchar_t* dumpPath, const wchar_t* dumpId, void* ctx, EXCEPTION_POINTERS* exceptionInfo, MDRawAssertionInfo* assertionInfo, bool dumpSucceeded);
}

typedef struct __FlySwatterContextData {
	int enabled;
	wchar_t *reportUrl;
	CrashReportSender *reportSender;
	ExceptionHandler *handlerPtr;
	TCHAR dumpPath[32768];
} FLYSWATTERCONTEXT, *LPFLYSWATTERCONTEXT;

#define FLYSWATTERCONTEXTSIZE ((sizeof(lctx->dumpPath)/sizeof(TCHAR)) - 2)

FLYSWATTERCONTEXT ctx = {0, NULL, NULL, NULL};
LPFLYSWATTERCONTEXT lctx = NULL;

// this function uses VirtualAlloc and VirtualProtect in an effort to secure the context structure against corruption
// it won't always work, but its better than nothing.

FLYSWATTER_API int FlySwatterInit(TCHAR *dumpPath, TCHAR *reportUrl)
{
	TCHAR *expandedDumpPath = NULL;
	size_t edpSize = 0;

	if(lctx != NULL) {
		// the structure pointer shouldn't be initialized, something is wrong
		return(-1);
	}

	// Allocate some memory that we can protect later, we want to do it at the top of the address space
	// to put it as far away from the application as possible, this of course assumes that the heap is
	// at the bottom of the address space
	lctx = (LPFLYSWATTERCONTEXT)VirtualAlloc(NULL, sizeof(FLYSWATTERCONTEXT), MEM_COMMIT | MEM_RESERVE | MEM_TOP_DOWN, PAGE_READWRITE);
	if(lctx == NULL) {
		return(-2);
	}
	// We shouldn't need to do this as VirtualAlloc is supposed to do it
	// But we do it anyway so we know where we stand, crash handlers need special care
	SecureZeroMemory(lctx, sizeof(FLYSWATTERCONTEXT));

	// initialize the context with data based on our arguments
	if(dumpPath == NULL) {
		tstrcpy((LPTSTR)&lctx->dumpPath, _T("."));
	} else {
		ExpandEnvironmentStrings(dumpPath, (LPTSTR)&lctx->dumpPath, FLYSWATTERCONTEXTSIZE);
	}
	lctx->reportUrl = tstrdup(reportUrl);
	lctx->reportSender = new CrashReportSender(_T(""));
	lctx->handlerPtr = new ExceptionHandler((LPTSTR)&lctx->dumpPath, FlySwatterExceptionFilter, FlySwatterMiniDumpCallback, &ctx, ExceptionHandler::HANDLER_ALL);
	// store the return value in a temporary variable as we're about to lock it down and reading the context will
	// cause an exception
	int rVal;
	DWORD bb;

	rVal = (lctx->handlerPtr == NULL ? 0 : 1);

	// Lock the page off
	rVal = VirtualProtect(lctx, FLYSWATTERCONTEXTSIZE, PAGE_NOACCESS, &bb);
	if(rVal == 0) {
		rVal = GetLastError();
	}
	return(rVal);
}
/*
FLYSWATTER_API int FlySwatterInitOrig(TCHAR *dumpPath, TCHAR *reportUrl)
{
	if(ctx.handlerPtr != NULL) {
		return(-1);
	}
	if(dumpPath == NULL) {
		dumpPath = _T(".");
	}

	// initialize the context
	ctx.enabled = 0;
	ctx.reportUrl = tstrdup(reportUrl);
	ctx.reportSender = new CrashReportSender(NULL);
	ctx.handlerPtr = new ExceptionHandler(dumpPath, FlySwatterExceptionFilter, FlySwatterMiniDumpCallback, &ctx, ExceptionHandler::HANDLER_ALL);

	return((ctx.handlerPtr == NULL ? 0 : 1));
}
*/

FLYSWATTER_API int FlySwatterEnable()
{
	int ov;
	FlySwatterMiniDumpCallback(_T("C:\\blah.txt"), _T("test"), lctx, NULL, NULL, true);
	if(lctx->handlerPtr == NULL) {
		return(-1);
	}
	ov = lctx->enabled;
	lctx->enabled = 1;
	return(ov);
}

FLYSWATTER_API int FlySwatterDisable()
{
	int ov;
	if(ctx.handlerPtr == NULL) {
		return(-1);
	}
	ov = ctx.enabled;
	ctx.enabled = 0;
	return(ov);
}

FLYSWATTER_API int FlySwatterIsEnabled()
{
	if(ctx.handlerPtr == NULL) {
		return(-1);
	}
	if(ctx.enabled == 0) {
		return(0);
	} else {
		return(1);
	}
}

bool FlySwatterExceptionFilter(void *ctx, EXCEPTION_POINTERS *exceptionInfo, MDRawAssertionInfo *assertionInfo)
{
	if(ctx == NULL) {
		// this is a bad sign, a misconfiguration or a very corrupted stack, abort
		return(false);
	}

	if(((LPFLYSWATTERCONTEXT)ctx)->enabled == 0) {
		return(false);
	}
	// we always call FlySwatter for now when we're enabled
	return(true);
}

bool FlySwatterMiniDumpCallback(const wchar_t *dumpPath, const wchar_t *dumpId, void *ctx, EXCEPTION_POINTERS *exceptionInfo, MDRawAssertionInfo *assertionInfo, bool dumpSucceeded)
{
	if(dumpSucceeded == false) {
		return(false);
	}

	// We've got a crash dump and we're enabled
	// Compile the data to be sent

	 // Sends the specified minidump file, along with the map of
  // name value pairs, as a multipart POST request to the given URL.
  // Parameter names must contain only printable ASCII characters,
  // and may not contain a quote (") character.
  // Only HTTP(S) URLs are currently supported.  The return value indicates
  // the result of the operation (see above for possible results).
  // If report_code is non-NULL and the report is sent successfully (that is,
  // the return value is RESULT_SUCCEEDED), a code uniquely identifying the
  // report will be returned in report_code.
  // (Otherwise, report_code will be unchanged.)

///	  ReportResult SendCrashReport(const wstring &url,
///                               const map<wstring, wstring> &parameters,
///                               const wstring &dump_file_name,
///                               wstring *report_code);

	map<wstring, wstring> params;
	wstring fileName = _T("C:\\test.txt");
	wstring reportCode;
	params[_T("AppName")] = _T("Name of the App");

	// unlock the context so we can use it
	int rVal;
	DWORD bb;
	ReportResult rs;
	rVal = VirtualProtect(lctx, FLYSWATTERCONTEXTSIZE, PAGE_READONLY, &bb);
	if(rVal == 0) {
		rVal = GetLastError();
	}
	
	/// send the report
	rs = ((LPFLYSWATTERCONTEXT)ctx)->reportSender->SendCrashReport(lctx->reportUrl, params, fileName, &reportCode);
	
	// no need to relock it, we're going to die now anyway
//	int rVal;
//	DWORD bb;
//	rVal = VirtualProtect(lctx, FLYSWATTERCONTEXTSIZE, PAGE_NOACCESS, &bb);
//	if(rVal == 0) {
//		rVal = GetLastError();
//	}
//	rVal = (lctx->handlerPtr == NULL ? 0 : 1);

	return(true);
}
