// FlySwatter.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

// these are in librtslocalize, which we don't want to include just yet
#ifdef WIN32
#define tstrcpy wcscpy
#define tstrdup _wcsdup
#endif

typedef struct __FlySwatterContextData {
	volatile int enabled;
	wchar_t *reportUrl;
	//CrashReportSender *reportSender;
	ExceptionHandler *handlerPtr;
	wchar_t dumpPath[32768];
	FLYSWATTERPARAM *params;
	int params_len;
} FLYSWATTERCONTEXT, *LPFLYSWATTERCONTEXT;

#define FLYSWATTERCONTEXTSIZE ((sizeof(lctx->dumpPath)/sizeof(wchar_t)) - 2)

LPFLYSWATTERCONTEXT lctx = NULL;

// this function uses VirtualAlloc and VirtualProtect in an effort to secure the context structure against corruption
// it won't always work, but its better than nothing.

FLYSWATTER_API int FlySwatterInit(wchar_t *dumpPath, wchar_t *reportUrl, wchar_t *OOPExePath)
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
		wcscpy((LPWSTR)&lctx->dumpPath, L".");
	} else {
		ExpandEnvironmentStrings(dumpPath, (LPTSTR)&lctx->dumpPath, FLYSWATTERCONTEXTSIZE);
	}
	lctx->reportUrl = tstrdup(reportUrl);
	//lctx->reportSender = new CrashReportSender(_T(""));
	lctx->handlerPtr = new ExceptionHandler((LPTSTR)&lctx->dumpPath, FlySwatterExceptionFilter, FlySwatterMiniDumpCallback, lctx, ExceptionHandler::HANDLER_ALL);
	return((lctx->handlerPtr == NULL ? 0 : 1));
}

FLYSWATTER_API int FlySwatterEnable()
{
	if(lctx->handlerPtr == NULL) {
		return(-1);
	}
	return(InterlockedExchange((volatile LONG *)&lctx->enabled, 1));
}

FLYSWATTER_API int FlySwatterDisable()
{
	if(lctx->handlerPtr == NULL) {
		return(-1);
	}
	return(InterlockedExchange((volatile LONG *)&lctx->enabled, 0));
}

FLYSWATTER_API int FlySwatterIsEnabled()
{
	if(lctx->handlerPtr == NULL) {
		return(-1);
	}
	return(lctx->enabled);
}

FLYSWATTER_API void FlySwatterSetParam(const wchar_t *name, const wchar_t *value)
{
	if(name == NULL) {
		// can't do anything with this
		return;
	}

	if(wcslen(name) == 0) {
		// the name is empty, we can't do anything with this either.
		return;
	}

	LPFLYSWATTERPARAM p = NULL;
	LPFLYSWATTERPARAM ep = NULL;

	// First we have to see if the parameter we're setting already exists
	for(int i = 0; i < lctx->params_len; i++) {
		if(lctx->params[i].name == NULL) {
			// this is an empty slot, skip it
			if(ep == NULL) {
				// save the first one so we can use it later if we need to
				ep = &lctx->params[i];
			}
			continue;
		}
		if(wcscmp(lctx->params[i].name, name)==0) {
			// we've found a matching value, clear out the existing data if there is any
			p = &lctx->params[i];
			if(p->value != NULL) {
				free(p->value);
				p->value = NULL;
			}	
			break;
		}
	}

	if(p == NULL) {
		// the name doesn't exist already, add it
		if(lctx->params_len == 0) {
			lctx->params = (LPFLYSWATTERPARAM)VirtualAlloc(NULL, (16 * sizeof(FLYSWATTERPARAM)), MEM_COMMIT | MEM_RESERVE | MEM_TOP_DOWN, PAGE_READWRITE);
			if(lctx->params == NULL) {
				// we couldn't allocate our buffer, just get out of town
				return;
			}
			// We shouldn't need to do this as VirtualAlloc is supposed to do it
			// But we do it anyway so we know where we stand, crash handlers need special care
			SecureZeroMemory(lctx->params, 16 * sizeof(FLYSWATTERPARAM));

			lctx->params = (LPFLYSWATTERPARAM)calloc(16, sizeof(FLYSWATTERPARAM));
			lctx->params_len = 16;
			memset(lctx->params, 0, (16 * sizeof(FLYSWATTERPARAM)));
			p = &lctx->params[0];
			p->name = _wcsdup(name);
		} else {
			if(ep != NULL) {
				// we have an empty one in the existing block we can use, so lets use it
				p = ep;
				ep->name = _wcsdup(name);
				if(p->value != NULL) {
					free(p->value);
					p->value = NULL;
				}
			} else {
				// we have to grow the buffer block
				void *tPtr = (LPFLYSWATTERPARAM)VirtualAlloc(NULL, ((lctx->params_len + 16) * sizeof(FLYSWATTERPARAM)), MEM_COMMIT | MEM_RESERVE | MEM_TOP_DOWN, PAGE_READWRITE);
				if(tPtr == NULL) {
					// we couldn't allocate our buffer, just get out of town
					return;
				}
				// We shouldn't need to do this as VirtualAlloc is supposed to do it
				// But we do it anyway so we know where we stand, crash handlers need special care
				SecureZeroMemory(tPtr, ((lctx->params_len + 16) * sizeof(FLYSWATTERPARAM)));
				MoveMemory(tPtr, lctx->params, (lctx->params_len * sizeof(FLYSWATTERPARAM)));
				VirtualFree(lctx->params, (lctx->params_len * sizeof(FLYSWATTERPARAM)), MEM_RELEASE);
				lctx->params = (LPFLYSWATTERPARAM)tPtr;
				p = &lctx->params[lctx->params_len];
				lctx->params_len += 16;
				p->name = _wcsdup(name);
			}
		}
	}

	if(value != NULL) {
		p->value = wcsdup(value);
	}
}

const wchar_t *FlySwatterGetParamEx(LPFLYSWATTERPARAM params, int params_len, const wchar_t *name)
{
	if(name == NULL) {
		// can't do anything with this
		return(NULL);
	}

	if(wcslen(name) == 0) {
		// the name is empty, we can't do anything with this either.
		return(NULL);
	}

	// First we have to see if the parameter we're setting already exists
	for(int i = 0; i < params_len; i++) {
		if(params[i].name == NULL) {
			// this is an empty slot, skip it
			continue;
		}
		if(wcscmp(params[i].name, name)==0) {
			// we've found a matching value, clear out the existing data if there is any
			return(params[i].value);
		}
	}
	return(NULL);
}

FLYSWATTER_API const wchar_t *FlySwatterGetParam(const wchar_t *name)
{
	return(FlySwatterGetParamEx(lctx->params, lctx->params_len, name));
}

bool FlySwatterExceptionFilter(void *ctx, EXCEPTION_POINTERS *exceptionInfo, MDRawAssertionInfo *assertionInfo)
{
	if(ctx == NULL) {
		// this is a bad sign, a misconfiguration or a very corrupted stack, abort
		MessageBox(NULL, L"An error occurred in the application which has caused it to crash.  An error report could not be generated.", L"An application error has occurred.", MB_OK);
		return(false);
	}

	if(((LPFLYSWATTERCONTEXT)ctx)->enabled == 0) {
		return(false);
	}

	// we always call FlySwatter for now when we're enabled
	return(true);
}
	
bool FlySwatterMiniDumpCallback(const wchar_t *dumpPath, const wchar_t *dumpId, void *mctx, EXCEPTION_POINTERS *exceptionInfo, MDRawAssertionInfo *assertionInfo, bool dumpSucceeded)
{
	LPFLYSWATTERCONTEXT ctx = (LPFLYSWATTERCONTEXT)mctx;
	if(dumpSucceeded == false) {
		return(false);
	}

	wchar_t miniDumpFilename[1025];
	_sntprintf(miniDumpFilename, 1023, L"%s\\%s.dmp", dumpPath, dumpId);
	miniDumpFilename[1023] = L'\0';
	FlySwatterCrashAlert(ctx->reportUrl, miniDumpFilename, ctx->params, ctx->params_len);
	// the minidump should already be gone, but we're going to remove it again anyway to be safe
	_wunlink(miniDumpFilename);

// no need to relock it, we're going to die now anyway
//	int rVal;
//	DWORD bb;
//	rVal = VirtualProtect(lctx, FLYSWATTERCONTEXTSIZE, PAGE_NOACCESS, &bb);
//	if(rVal == 0) {
//		rVal = GetLastError();
//	}
//	rVal = (lctx->handlerPtr == NULL ? 0 : 1);

	exit(255);
	return(true);
}
