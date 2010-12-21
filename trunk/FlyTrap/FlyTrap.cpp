/*! @internal
 * @file FlyTrap.cpp
 * This file contains glue which initializes google breakpad and allows
 * the caller to control how FlyTrap behaves.
 *
 * @brief FlyTrap glue implementation
 * @author David Wimsey
 * $Revision$
 * $Date$
 *
 * @section LICENSE
 *
 * Copyright (c) 2003-2010, David Wimsey
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

// FlyTrap.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "flytrapversion.h"
// these are in librtslocalize, which we don't want to include just yet
#ifdef WIN32
#define tstrcpy wcscpy
#define tstrdup _wcsdup
#endif

#define FLYTRAPSERVERSTARTUPTIMEOUT	20000
#define MESSAGE_BUFFER_LEN	(1<<20)
#define MAX_PATH_LEN	(1<<15)
#define SAFE_MAX_PATH	(32000)
#define RUNDLL32_PATH	L"%SystemRoot%\\System32\\rundll32.exe"
#define FLYTRAP_LIBRARY_BINARY_NAME	L"FlyTrap.dll"
typedef struct __FlyTrapContextData {
	volatile int enabled;
	wchar_t *reportUrl;
	wchar_t *OOPServerExecutablePath;
	ExceptionHandler *handlerPtr;
	wchar_t dumpPath[MAX_PATH_LEN+3];
	CustomClientInfo *ccInfo;
	FLYTRAPPARAM *params;
	int params_len;
} FLYTRAPCONTEXT, *LPFLYTRAPCONTEXT;

typedef struct __FlyTrapServerContextData {
	CrashGenerationServer *server;
	HANDLE processHandle;
	volatile LONG windowsWaittingCount;
	wchar_t *reportUrl;
	wchar_t dumpPath[MAX_PATH_LEN+3];
} FLYTRAPSERVERCONTEXT, *LPFLYTRAPSERVERCONTEXT;

LPFLYTRAPCONTEXT lctx = NULL;
const wchar_t FSkPipeName[] = L"\\\\.\\pipe\\FlyTrap\\%d";
const wchar_t FSkEventName[] = L"Local\\FlyTrapServerReady%d";
extern "C" __declspec (dllexport) void __cdecl FlyTrapRunDLLLaunchServerW(HWND hWnd, HINSTANCE hInstance, wchar_t *lpCmdLine, int nCmdShow)
{
	wchar_t *tt;
	wchar_t *offPtr;
	wchar_t *pidStr;
	wchar_t *pipeName;
	wchar_t *reportUrl;
	wchar_t *dumpPath;

	tt = wcsdup(lpCmdLine);
		
	pidStr = tt;
	offPtr = wcschr(pidStr, L' '); // we look for the first space, we require pipe paths to not use spaces or other command-line unsafe characters
	if(offPtr == NULL) {
		// command line is malformed
		MessageBoxW(hWnd, lpCmdLine, L"Command line error starting FlyTrap server, no pipe name specified on the command line.", MB_OK);
		return;
	}
	offPtr[0] = L'\0';	// this ends off the pipe name

	pipeName = offPtr + 1;
	offPtr = wcschr(pipeName, L' '); // we look for the first space, we require pipe paths to not use spaces or other command-line unsafe characters
	if(offPtr == NULL) {
		// command line is malformed
		MessageBoxW(hWnd, lpCmdLine, L"Command line error starting FlyTrap server, no report url specified.", MB_OK);
		return;
	}
	offPtr[0] = L'\0';	// this ends off the pipe name

	reportUrl = offPtr + 1;
	offPtr = wcschr(reportUrl, L' ');
	if(offPtr == NULL) {
		// command line is malformed
		MessageBoxW(hWnd, lpCmdLine, L"Command line error starting FlyTrap server, no dump path specified.", MB_OK);
		return;
	}
	offPtr[0] = L'\0';

	dumpPath = offPtr + 1;

	offPtr = wcschr(dumpPath, L' ');
	if(offPtr == NULL) {
		// command line is malformed
		MessageBoxW(hWnd, lpCmdLine, L"Command line error starting FlyTrap server, no startup ready event name specified.", MB_OK);
		return;
	}
	offPtr[0] = L'\0';

	wchar_t *startupReadyEventName = offPtr + 1;

	DWORD pid = _wtol(pidStr);
	
	LPFLYTRAPSERVERCONTEXT sCtx = (LPFLYTRAPSERVERCONTEXT)FlyTrapInitServer(dumpPath, reportUrl, pipeName, startupReadyEventName);
	if(sCtx == NULL) {
		MessageBoxW(NULL, pipeName, L"Could not initialized FlyTrap server.", MB_OK);
		return;
	}

	// Open the calling process to wait for it to finish.
	HANDLE pHandle = OpenProcess(SYNCHRONIZE, FALSE, pid);
	// When the wait returns, the original process has exited and so should we
	DWORD res = WaitForSingleObject(pHandle, 1000);
	while( res == 258 || sCtx->windowsWaittingCount > 0 ) {
		Sleep(100);
		res = WaitForSingleObject(pHandle, 100);
	}
	FlyTrapShutdownServer(sCtx);
}

// this is just a wrapper to convert to unicode and invoke the unicode version of this function
extern "C" __declspec (dllexport) void __cdecl FlyTrapRunDLLLaunchServer(HWND hWnd, HINSTANCE hInstance, char *lpCmdLineA, int nCmdShow)
{
	size_t szCmdLine;
	wchar_t *lpCmdLine;

	szCmdLine = strlen(lpCmdLineA) + 1;
	lpCmdLine = (wchar_t*)malloc((szCmdLine + 1)*sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, lpCmdLineA, szCmdLine, lpCmdLine, szCmdLine);
	FlyTrapRunDLLLaunchServerW(hWnd, hInstance, lpCmdLine, nCmdShow);
	free(lpCmdLine);
}

// this function uses VirtualAlloc and VirtualProtect in an effort to secure the context structure against corruption
// it won't always work, but its better than nothing.

bool AddExceptionHandler(LPFLYTRAPCONTEXT clientContext)
{
	if(clientContext == NULL) {
		return(false);
	}
	if(clientContext->handlerPtr != NULL) {
		// a handler has already been installed, don't overlap it.
		return(false);
	}

	wchar_t *tmpStr;
	DWORD sRes;
	wchar_t *fpath;
	wchar_t *dllpath;
	wchar_t pidstr[22];
	wchar_t pipepathstr[50];
	wchar_t eventpathstr[50];
	wchar_t *pMessageBuffer;
	DWORD pid;
	HMODULE sModule;
	int spawnErr;
	int eNo;
	HANDLE serverReadyEvent;

	clientContext->ccInfo = new CustomClientInfo();
	int realCount = 0;
	for(int i = 0; i < clientContext->params_len; i++) {
		if(clientContext->params[i].name != NULL) {
			realCount++;
		}
	}

	clientContext->ccInfo->count = realCount;
	clientContext->ccInfo->entries = new CustomInfoEntry[realCount];
	realCount = 0;
	for(int i = 0; i < clientContext->params_len; i++) {
		if(clientContext->params[i].name != NULL) {
			wcsncpy((wchar_t*)&clientContext->ccInfo->entries[realCount].name, clientContext->params[i].name, min(wcslen(clientContext->params[i].name)+1, clientContext->ccInfo->entries[realCount].kNameMaxLength));
			((wchar_t*)&clientContext->ccInfo->entries[realCount].name)[clientContext->ccInfo->entries[realCount].kNameMaxLength-1] = L'\0';
			wcsncpy((wchar_t*)&clientContext->ccInfo->entries[realCount].value, clientContext->params[i].value, min(wcslen(clientContext->params[i].value)+1, clientContext->ccInfo->entries[realCount].kValueMaxLength));
			((wchar_t*)&clientContext->ccInfo->entries[realCount].value)[clientContext->ccInfo->entries[realCount].kValueMaxLength-1] = L'\0';
			realCount++;
		}
	}
	
	if(clientContext->OOPServerExecutablePath != NULL && wcslen(clientContext->OOPServerExecutablePath) > 0) {
		// try out of process
		if(wcscmp(clientContext->OOPServerExecutablePath, FLYTRAP_LIBRARY_BINARY_NAME)==0) {
			// try to use rundll to launch this DLL again as a server
			tmpStr = ExpandEnvVarsInStr(RUNDLL32_PATH);
			if(tmpStr == NULL) {
				return(false);
			}
			fpath = (wchar_t*)malloc(sizeof(wchar_t)*(MAX_PATH_LEN+5));
			if(fpath == NULL) {
				return(false);
			}
			sRes = GetShortPathNameW(tmpStr, fpath, MAX_PATH_LEN);
			free(tmpStr);

			// Generate the pipe path
			pid = GetCurrentProcessId();
			wsprintf(pidstr, L"%d", pid);
			wsprintf(pipepathstr, FSkPipeName, pid);
			wsprintf(eventpathstr, FSkEventName, pid);

			// Generate the server DLL path
			sModule = GetModuleHandleW(L"FlyTrap");
			if(sModule != NULL) {
				pMessageBuffer = (wchar_t*)malloc((MESSAGE_BUFFER_LEN+5) * sizeof(wchar_t));
				if(pMessageBuffer == NULL) {
					free(fpath);
					return(false);
				}

				GetModuleFileNameW(sModule, pMessageBuffer, MESSAGE_BUFFER_LEN);
				tmpStr = (wchar_t*)malloc((MAX_PATH_LEN+5) * sizeof(wchar_t));
				if(tmpStr == NULL) {
					free(fpath);
					free(pMessageBuffer);
					return(false);
				}

				GetShortPathName(pMessageBuffer, tmpStr, MAX_PATH_LEN);
				free(pMessageBuffer);
				dllpath = (wchar_t*)malloc((wcslen(tmpStr) + strlen(",FlyTrapRunDLLLaunchServer") + 5) * sizeof(wchar_t));
				if(dllpath == NULL) {
					free(fpath);
					free(tmpStr);
					return(false);
				}

				wcscpy(dllpath, tmpStr);
				free(tmpStr);
				wcscat(dllpath, L",FlyTrapRunDLLLaunchServer");
			} else {
				// Our DLL has been renamed, but try the default anyway
				dllpath = wcsdup(FLYTRAP_LIBRARY_BINARY_NAME L",FlyTrapRunDLLLaunchServer");
			}

			// create an event for the server to use to signal ready
			serverReadyEvent = ::CreateEventW(NULL, TRUE, FALSE, eventpathstr);
			if(serverReadyEvent == NULL) {
				MessageBox(NULL, eventpathstr, L"FlyTrap could not create server startup notification event.", MB_OK);
				free(fpath);
				free(dllpath);
			} else {
				spawnErr = _wspawnl(_P_DETACH, fpath, fpath, dllpath, pidstr, pipepathstr, clientContext->reportUrl, clientContext->dumpPath, eventpathstr, NULL);
				eNo = *_errno();
				free(fpath);
				free(dllpath);
				if(spawnErr < 0) {
					MessageBox(NULL, pipepathstr, L"Could not initialized client pipe.", MB_OK);
				} else {
					// OOP Server was 'launched', now lets see if we can talk to it
					// now we have to figure out a way to wait around for something to talk too, it shouldn't take long
					// but we have to wait otherwise we'll try to connect before the server has actually started.
					switch(WaitForSingleObject(serverReadyEvent, FLYTRAPSERVERSTARTUPTIMEOUT)) {
						case WAIT_OBJECT_0:
							// the server has signaled it is ready for client connections
							clientContext->handlerPtr = new ExceptionHandler((wchar_t*)&clientContext->dumpPath, FlyTrapExceptionFilter, FlyTrapInProcessDumpCallback, clientContext, ExceptionHandler::HANDLER_ALL, MiniDumpNormal, pipepathstr, clientContext->ccInfo);//custom_client_info_c);
							CloseHandle(serverReadyEvent);
							return(true);
							break;
						case WAIT_ABANDONED:
							MessageBox(NULL, L"FlyTrap server could not be started, in process error reporting will be used.", L"", MB_OK);
							break;
						case WAIT_TIMEOUT:
							MessageBox(NULL, L"FlyTrap server did not start in time, in process error reporting will be used.", L"", MB_OK);
							break;
						case WAIT_FAILED:
							MessageBox(NULL, L"A system error occured waiting on the the FlyTrap server to become ready, in process error reporting will be used.", L"", MB_OK);
							break;
						default:
							MessageBox(NULL, L"An unexpected error occured waiting on the the FlyTrap server to become ready, in process error reporting will be used.", L"", MB_OK);
							break;
					}
				}
				CloseHandle(serverReadyEvent);
			}
		}
	}

	if(clientContext->handlerPtr == NULL) {
		// no crash CrashGenerationClient exists, so fall back to internal
		// exception handling
		clientContext->handlerPtr = new ExceptionHandler((wchar_t*)&clientContext->dumpPath, FlyTrapExceptionFilter, FlyTrapInProcessDumpCallback, clientContext, ExceptionHandler::HANDLER_ALL);
	}
	if(clientContext->handlerPtr != NULL) {
		return(true);
	} else {
		return(false);
	}
}

FLYTRAP_API int __stdcall FlyTrapInitClient(wchar_t *dumpPath, wchar_t *reportUrl, wchar_t *OOPExePath)
{
	TCHAR *expandedDumpPath = NULL;
	size_t edpSize = 0;

	if(lctx != NULL) {
		// the structure pointer shouldn't be initialized, something is wrong
		return(FLYTRAP_ERROR_NO_CONTEXT);
	}

	// Allocate some memory that we can protect later, we want to do it at the top of the address space
	// to put it as far away from the application as possible, this of course assumes that the heap is
	// at the bottom of the address space
	LPFLYTRAPCONTEXT clientContext = (LPFLYTRAPCONTEXT)VirtualAlloc(NULL, sizeof(FLYTRAPCONTEXT), MEM_COMMIT | MEM_RESERVE | MEM_TOP_DOWN, PAGE_READWRITE);
	if(clientContext == NULL) {
		return(FLYTRAP_ERROR_OUTOFMEMORY);
	}
	// We shouldn't need to do this as VirtualAlloc is supposed to do it
	// But we do it anyway so we know where we stand, crash handlers need special care
	SecureZeroMemory(clientContext, sizeof(FLYTRAPCONTEXT));

	wchar_t *tmpStr;

	// initialize the context with data based on our arguments
	if(dumpPath == NULL) {
		wcscpy((LPWSTR)&clientContext->dumpPath, L".");
	} else {
		ExpandEnvironmentStringsW(dumpPath, (LPTSTR)&clientContext->dumpPath, MAX_PATH_LEN);
		tmpStr = wcsdup((LPWSTR)&clientContext->dumpPath);
		GetShortPathName(tmpStr, (LPWSTR)&clientContext->dumpPath, MAX_PATH_LEN);
		free(tmpStr);
		clientContext->dumpPath[MAX_PATH_LEN] = _T('\0');
		clientContext->dumpPath[MAX_PATH_LEN-1] = _T('\0');
	}
	clientContext->reportUrl = wcsdup(reportUrl);

	// make sure the dump path exists if possible
	_wmkdir(clientContext->dumpPath);

	if(OOPExePath != NULL && wcslen(OOPExePath) > 0) {
		clientContext->OOPServerExecutablePath = wcsdup(OOPExePath);
	}
	// for now we enable at init automatically.  This allows us to catch the earliest possible crash, even in our own code possibly.
	clientContext->params = NULL;
	clientContext->params_len = 0;
	clientContext->enabled = 0;

	lctx = clientContext;
	return(FLYTRAP_ERROR_SUCCESS);
}

#include "client/windows/crash_generation/crash_generation_server.h"

/*!
 *
 */

FLYTRAP_API int __stdcall FlyTrapShutdownClient(void *clientContext)
{
	if(clientContext == NULL){
		return(false);
	} else {
		VirtualFree(clientContext, sizeof(FLYTRAPCONTEXT), MEM_RELEASE);
		free(clientContext);
		return(true);
	}
}

FLYTRAP_API int __stdcall FlyTrapShutdownServer(void *serverContext)
{
	LPFLYTRAPSERVERCONTEXT sCtx = (LPFLYTRAPSERVERCONTEXT)serverContext;
	if(sCtx != NULL) {
		if(sCtx->server != NULL) {
			delete sCtx->server;
		}
		VirtualFree(sCtx, sizeof(FLYTRAPSERVERCONTEXT), MEM_RELEASE);
		return(true);
	}
	return(false);
}

FLYTRAP_API void * __stdcall FlyTrapInitServer(wchar_t *dumpPath, wchar_t *reportUrl, wchar_t *pipeName, wchar_t *serverReadyEventName)
{
//	TCHAR *expandedDumpPath = NULL;
//	size_t edpSize = 0;

	// Allocate some memory that we can protect later, we want to do it at the top of the address space
	// to put it as far away from the application as possible, this of course assumes that the heap is
	// at the bottom of the address space
	LPFLYTRAPSERVERCONTEXT serverContext = (LPFLYTRAPSERVERCONTEXT)VirtualAlloc(NULL, sizeof(FLYTRAPSERVERCONTEXT), MEM_COMMIT | MEM_RESERVE | MEM_TOP_DOWN, PAGE_READWRITE);
	if(serverContext == NULL) {
		return(NULL);
	}
	// We shouldn't need to do this as VirtualAlloc is supposed to do it
	// But we do it anyway so we know where we stand, crash handlers need special care
	SecureZeroMemory(serverContext, sizeof(FLYTRAPSERVERCONTEXT));

	serverContext->reportUrl = wcsdup(reportUrl);

	wstring rPipePath (pipeName);
	wstring rDumpPath (dumpPath);
	serverContext->server = new CrashGenerationServer(rPipePath, NULL, NULL, NULL, (google_breakpad::CrashGenerationServer::OnClientDumpRequestCallback)&FlyTrapOutOfProcessDumpCallback, serverContext, NULL, NULL, true, &rDumpPath);
	if(serverContext->server->Start() != true) {
		delete serverContext->server;
		free(serverContext);
		return(NULL);
	}

	HANDLE serverReadyEvent = ::OpenEventW(EVENT_ALL_ACCESS, FALSE, serverReadyEventName);

	if(serverReadyEvent == NULL) {
		DWORD errNo = GetLastError();
		MessageBox(NULL, serverReadyEventName, L"FlyTrap could not open server startup notification event.", MB_OK);
	} else {
		SetEvent(serverReadyEvent);
		CloseHandle(serverReadyEvent);
	}
	return(serverContext);	
}

FLYTRAP_API int __stdcall FlyTrapEnable()
{
	if(lctx == NULL) {
		return(-1);
	}
	if(lctx->handlerPtr == NULL) {
		if(AddExceptionHandler(lctx)==0) {
			return(-2);
		}
	}
	return(InterlockedExchange((volatile LONG *)&lctx->enabled, 1));
}

FLYTRAP_API int __stdcall FlyTrapDisable()
{
	if(lctx == NULL) {
		return(-1);
	}
	if(lctx->handlerPtr == NULL) {
		return(-2);
	}
	return(InterlockedExchange((volatile LONG *)&lctx->enabled, 0));
}

FLYTRAP_API int __stdcall FlyTrapIsEnabled()
{
	if(lctx == NULL) {
		return(-1);
	}
	if(lctx->handlerPtr == NULL) {
		return(-2);
	}
	return(lctx->enabled);
}

FLYTRAP_API void __stdcall FlyTrapSetParam(const wchar_t *name, const wchar_t *value)
{
	if(lctx == NULL) {
		return;
	}
	if(name == NULL) {
		// can't do anything with this
		return;
	}

	if(wcslen(name) == 0) {
		// the name is empty, we can't do anything with this either.
		return;
	}

	LPFLYTRAPPARAM p = NULL;
	LPFLYTRAPPARAM ep = NULL;

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
			lctx->params = (LPFLYTRAPPARAM)VirtualAlloc(NULL, (16 * sizeof(FLYTRAPPARAM)), MEM_COMMIT | MEM_RESERVE | MEM_TOP_DOWN, PAGE_READWRITE);
			if(lctx->params == NULL) {
				// we couldn't allocate our buffer, just get out of town
				return;
			}
			// We shouldn't need to do this as VirtualAlloc is supposed to do it
			// But we do it anyway so we know where we stand, crash handlers need special care
			SecureZeroMemory(lctx->params, 16 * sizeof(FLYTRAPPARAM));

			lctx->params = (LPFLYTRAPPARAM)calloc(16, sizeof(FLYTRAPPARAM));
			lctx->params_len = 16;
			memset(lctx->params, 0, (16 * sizeof(FLYTRAPPARAM)));
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
				void *tPtr = (LPFLYTRAPPARAM)VirtualAlloc(NULL, ((lctx->params_len + 16) * sizeof(FLYTRAPPARAM)), MEM_COMMIT | MEM_RESERVE | MEM_TOP_DOWN, PAGE_READWRITE);
				if(tPtr == NULL) {
					// we couldn't allocate our buffer, just get out of town
					return;
				}
				// We shouldn't need to do this as VirtualAlloc is supposed to do it
				// But we do it anyway so we know where we stand, crash handlers need special care
				SecureZeroMemory(tPtr, ((lctx->params_len + 16) * sizeof(FLYTRAPPARAM)));
				MoveMemory(tPtr, lctx->params, (lctx->params_len * sizeof(FLYTRAPPARAM)));
				VirtualFree(lctx->params, (lctx->params_len * sizeof(FLYTRAPPARAM)), MEM_RELEASE);
				lctx->params = (LPFLYTRAPPARAM)tPtr;
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

const wchar_t *FlyTrapGetParamEx(LPFLYTRAPPARAM params, int params_len, const wchar_t *name)
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

FLYTRAP_API const wchar_t * __stdcall FlyTrapGetParam(const wchar_t *name)
{
	if(lctx == NULL) {
		return(NULL);
	}
	return(FlyTrapGetParamEx(lctx->params, lctx->params_len, name));
}

static int flytrap_in_exception = 0;
FLYTRAP_API void __stdcall FlyTrapTriggerReport()
{
	if(lctx == NULL) {
		return;
	}
	if(lctx->handlerPtr == NULL) {
		return;
	}
	lctx->handlerPtr->WriteMinidump();
	// reset the global in exception marker so we handle exceptions in the future
	flytrap_in_exception = 0;
}

bool FlyTrapExceptionFilter(void *ctx, EXCEPTION_POINTERS *exceptionInfo, MDRawAssertionInfo *assertionInfo)
{
	// This filter determines if we have anything to do with catching the exception.  If we return true, the dump
	// will be written and the dump callback called.  If we return false, the exception will be passed up the
	// handler chain
	if(ctx == NULL) {
		// this is a bad sign, a misconfiguration or a very corrupted stack, abort
		MessageBox(NULL, L"An error occurred in the application which has caused it to crash.  An error report could not be generated.", L"An application error has occurred.", MB_OK);
		return(false);
	}

	if(((LPFLYTRAPCONTEXT)ctx)->enabled == 0) {
		return(false);
	}

	if(flytrap_in_exception > 0) {
		// we don't filter our own exceptions at this time
		return(false);
	}
	// we always call FlyTrap for now when we're enabled
	flytrap_in_exception = 1;
	return(true);
}

typedef struct __FlyTrapOutOfProcessCallbackData {
	LPFLYTRAPSERVERCONTEXT serverContext;
	const wchar_t *dump_path;
	unsigned int pCount;
	LPFLYTRAPPARAM params;
	int report_mode;
} FTOOPCDATA, *LPFTOOPCDATA;

DWORD WINAPI FlyTrapOutOfProcessDumpCallbackThread(LPVOID iValue)
{
	LPFTOOPCDATA ftOopcData = (LPFTOOPCDATA)iValue;
	if(ftOopcData != NULL) {
		wchar_t *dpath = NULL;
		wchar_t *dump_id = NULL;
		if(ftOopcData->dump_path != NULL) {
			dpath = wcsdup(ftOopcData->dump_path);
			dump_id = wcsrchr(dpath, '/');
			wchar_t *did_end = NULL;
#ifdef WIN32
			did_end = wcsrchr(dpath, '\\');
			if(did_end > dump_id) {
				dump_id = did_end;
			}
#endif
			if(dump_id != NULL) {
				did_end = wcschr(dump_id, '.');
				if(did_end != NULL) {
					did_end[0] = L'\0';
					dump_id++;
				} else {
					dump_id = NULL;
				}
			}
		}
		if(dump_id == NULL) {
			// generate our own dump id
			if(dpath != NULL) {
				free(dpath);
			}
			dpath = wcsdup(L"NoDumpIdFound");
			dump_id = dpath;
		}

		FlyTrapCrashAlert(NULL, ftOopcData->serverContext->reportUrl, ftOopcData->report_mode, dump_id, ftOopcData->dump_path, ftOopcData->params, ftOopcData->pCount);
		if(ftOopcData->dump_path != NULL) {
			// Unlink the dump file as it has been sent
			_wunlink(ftOopcData->dump_path);
			// free the memory for the dump file path
			free((void*)ftOopcData->dump_path);
		}

		if(dpath != NULL) {
			free(dpath);
		}

		// if we have parameters, we need to free them before we return as we're the final owner
		if(ftOopcData->params != NULL) {
			// loop through the parameters and free them
			for(unsigned int i = 0; i < ftOopcData->pCount; i++) {
				// free the parameter name, if it exists
				if(ftOopcData->params[i].name != NULL) {
					free(ftOopcData->params[i].name);
				}
				// free the parameter value, if it exists
				if(ftOopcData->params[i].value != NULL) {
					free(ftOopcData->params[i].value);
				}
			}
			// free the parameter pointer container
			free(ftOopcData->params);
		}
		// Update the server context to show that we're done and going away now
		if(ftOopcData->serverContext != NULL) {
			InterlockedDecrement(&ftOopcData->serverContext->windowsWaittingCount);
		}
		// free up the thread setup structure
		free(ftOopcData);
	}
	return(0);
}

void FlyTrapOutOfProcessDumpCallback(void *dump_context, ClientInfo *client_info, const std::wstring *dump_path)
{
	LPFLYTRAPPARAM params = NULL;
	LPFLYTRAPSERVERCONTEXT serverContext = (LPFLYTRAPSERVERCONTEXT)dump_context;

	// this will be free'd by the thread we start
	LPFTOOPCDATA ftOopcData = (LPFTOOPCDATA)malloc(sizeof(FTOOPCDATA));
	if(ftOopcData == NULL) {
		/// @TODO Handle this error in some way to let someone know it happened
		return;
	}
	// Zero initialize the structure
	memset(ftOopcData, 0, sizeof(FTOOPCDATA));

	unsigned int pCount = 0;
	if(client_info != NULL) {
		EXCEPTION_POINTERS **exInfo = client_info->ex_info();
		if(exInfo == NULL) {
			// something isn't write, lets report it as a crash to be safe
			ftOopcData->report_mode = FLYTRAP_REPORTMODE_CRASH;
		} else {
			// determine if the exception pointers point to an exception or if the request was user triggered
			ftOopcData->report_mode = ((exInfo == NULL) ? FLYTRAP_REPORTMODE_USERTRIGGER : FLYTRAP_REPORTMODE_CRASH);
		}

		CustomClientInfo ccInfo = client_info->GetCustomInfo();
		if(ccInfo.count > 0) {
			pCount = ccInfo.count;
			// this will be free'd by the thread we start
			params = (LPFLYTRAPPARAM)malloc(sizeof(FLYTRAPPARAM) * pCount);
			if(params == NULL) {
				/// @HACK Handle this error in some way to let someone know it happened

				// free the thread parameters structure
				free(ftOopcData);
				return;
			}
			for(unsigned int i = 0; i < pCount; i++) {
				if(ccInfo.entries[i].name != NULL) {
					params[i].name = wcsdup(ccInfo.entries[i].name);
				} else {
					params[i].name = NULL;
				}
				if(ccInfo.entries[i].value != NULL) {
					params[i].value = wcsdup(ccInfo.entries[i].value);
				} else {
					params[i].value = NULL;
				}
			}
		}
	}

	ftOopcData->params = params;
	ftOopcData->pCount = pCount;
	ftOopcData->serverContext = serverContext;
	// this will be free'd by the thread we start
	if(dump_path != NULL) {
		ftOopcData->dump_path = wcsdup(dump_path->c_str());
		if(ftOopcData->dump_path == NULL) {
			/// @TODO Handle this error in some way to let someone know it happened

			// free the parameters we've duplicated
			if(ftOopcData->params != NULL) {
				for(unsigned int i = 0; i < ftOopcData->pCount; i++) {
					// free the parameter name, if it exists
					if(ftOopcData->params[i].name != NULL) {
						free(ftOopcData->params[i].name);
					}
					// free the parameter value, if it exists
					if(ftOopcData->params[i].value != NULL) {
						free(ftOopcData->params[i].value);
					}
				}
				// free the parameter pointer container
				free(ftOopcData->params);
			}
			// free the thread parameters structure
			free(ftOopcData);
			return;
		}
	} else {
		// no path to set, set parameter to NULL
		ftOopcData->dump_path = NULL;
	}

	HANDLE ftcbThread;
	DWORD ftcbThreadId;

	// Add our reference in the server context so we aren't forgotten
	InterlockedIncrement(&ftOopcData->serverContext->windowsWaittingCount);
	ftcbThread = CreateThread(NULL, 0, FlyTrapOutOfProcessDumpCallbackThread, ftOopcData, 0, &ftcbThreadId);
	if(ftcbThread == NULL) {
		// the thread wasn't started, we'll need to cleanup ourselves
		// this window is certainly no longer waiting
		InterlockedDecrement(&ftOopcData->serverContext->windowsWaittingCount);
		if(ftOopcData->params != NULL) {
			// free the parameter copies
			for(unsigned int i = 0; i < ftOopcData->pCount; i++) {
				// free the parameter name, if it exists
				if(ftOopcData->params[i].name != NULL) {
					free(ftOopcData->params[i].name);
				}
				// free the parameter value, if it exists
				if(ftOopcData->params[i].value != NULL) {
					free(ftOopcData->params[i].value);
				}
			}
			// free the parameter pointer container
			free(ftOopcData->params);
		}
		if(ftOopcData->dump_path != NULL) {
			// free the dump file path string memory
			free((void*)ftOopcData->dump_path);
		}
		// free the thread parameters structure
		free(ftOopcData);
		return;
	}
	CloseHandle(ftcbThread);
}

bool FlyTrapInProcessDumpCallback(const wchar_t *dumpPath, const wchar_t *dumpId, void *mctx, EXCEPTION_POINTERS *exceptionInfo, MDRawAssertionInfo *assertionInfo, bool dumpSucceeded)
{
	LPFLYTRAPCONTEXT ctx = (LPFLYTRAPCONTEXT)mctx;
	if(dumpSucceeded == false) {
		// send the crash report even if a dump could not be created
		// return(false);
	}

	if((dumpPath == NULL) && (dumpId == NULL) && (dumpSucceeded == true)) {
		// this was handled out of process, we don't need to do anything here
		return(true);
	}

	wchar_t miniDumpFilename[1025];
	if(dumpSucceeded == true) {
		if(dumpPath == NULL || dumpId == NULL) {
			FlyTrapCrashAlert(NULL, ctx->reportUrl, (exceptionInfo == NULL ? FLYTRAP_REPORTMODE_USERTRIGGER : FLYTRAP_REPORTMODE_CRASH), dumpId, NULL, ctx->params, ctx->params_len);
		} else {
			wsprintf(miniDumpFilename, L"%s\\%s.dmp", dumpPath, dumpId);
			miniDumpFilename[1023] = L'\0';
			FlyTrapCrashAlert(NULL, ctx->reportUrl, (exceptionInfo == NULL ? FLYTRAP_REPORTMODE_USERTRIGGER : FLYTRAP_REPORTMODE_CRASH), dumpId, miniDumpFilename, ctx->params, ctx->params_len);
			// the minidump should already be gone, but we're going to remove it again anyway to be safe
			_wunlink(miniDumpFilename);
		}
	} else {
		FlyTrapCrashAlert(NULL, ctx->reportUrl, (exceptionInfo == NULL ? FLYTRAP_REPORTMODE_USERTRIGGER : FLYTRAP_REPORTMODE_CRASH), dumpId, NULL, ctx->params, ctx->params_len);
	}

	return(true);
}


// misc functions
HKEY GetRootKeyHandleFromPathStr(const wchar_t *path)
{
	if(wcsnicmp(path, L"HKEY_CLASSES_ROOT\\", 16)==0) {
		return(HKEY_CLASSES_ROOT);
	} else if(wcsnicmp(path, L"HKEY_CURRENT_USER\\", 18)==0) {
		return(HKEY_CURRENT_USER);
	} else if(wcsnicmp(path, L"HKEY_LOCAL_MACHINE\\", 19)==0) {
		return(HKEY_LOCAL_MACHINE);
	} else if(wcsnicmp(path, L"HKEY_CURRENT_CONFIG\\", 20)==0) {
		return(HKEY_CURRENT_CONFIG);
//	} else if(wcsnicmp(fnameBuf, L"HKEY_USERS\\", 5)==0) {
//		return(HKEY_USERS);
	} else {
		return(0);
	}
}


wchar_t *DumpRegistryKey(wchar_t *regPath)
{
	HKEY bKey;
	int i;
	FILETIME ft;
	wchar_t *subKeyNamePath;
	wchar_t *dumpOut;
	wchar_t *tmpPtr, *ttmpPtr;
	wchar_t nameBuf[257];
	DWORD nameBufSiz;
	DWORD vNameSiz = 16384;
	wchar_t *vName = (wchar_t*)calloc(vNameSiz + 1, sizeof(wchar_t));
	DWORD dataSiz;
	LPBYTE data;
	DWORD type;
	LRESULT r;
	HKEY baseKey = GetRootKeyHandleFromPathStr(regPath);
	wchar_t *baseSubKey = wcschr(regPath, L'\\');
	baseSubKey++;
	dumpOut = mprintf(L"\r\n[%s]\r\n", regPath);

	if(baseKey == 0) {
		// the path isn't properly formed, it doesn't contain a known base key
		tmpPtr = mprintf(L"%s!!Error: Keyname is invalid, base keyname is unknown\r\n", dumpOut);
		free(dumpOut);
		dumpOut = tmpPtr;
		return(dumpOut);
	}

	r = RegOpenKeyW(baseKey, baseSubKey, &bKey);
	if(r != ERROR_SUCCESS) {
		tmpPtr = mprintf(L"%s!!Error: Could not open key: %d\r\n", dumpOut, r);
		free(dumpOut);
		dumpOut = tmpPtr;
		return(dumpOut);
	}

	vName = (wchar_t*)calloc(vNameSiz + 1, sizeof(wchar_t));
	if(vName == NULL) {
		tmpPtr = mprintf(L"%s!!Error: Could allocate memory for name buffer.", dumpOut);
		free(dumpOut);
		dumpOut = tmpPtr;
		return(dumpOut);
	}
	for(i = 0; ; i++) {
		vNameSiz = 16384;
		dataSiz = 0;
		r = RegEnumValueW(bKey, i, vName, &vNameSiz, NULL, &type, NULL, &dataSiz);
		if(r != ERROR_SUCCESS) {
			if(r != ERROR_NO_MORE_ITEMS) {
				// TODO: Do something with this error
				tmpPtr = mprintf(L"%s!!Error: Could not enumerate value: %d: Index: %d\r\n", dumpOut, r, i);
				free(dumpOut);
				dumpOut = tmpPtr;
			}
			break;
		}
		dataSiz++;
		// this really should be char instead of wchar_t, but this gives us plenty of overrun space
		data = (LPBYTE)calloc(dataSiz + 1, sizeof(wchar_t));
		if(data == NULL) {
			// couldn't allocate the memory for the data stored, log this condition and skip to the next
			tmpPtr = mprintf(L"%s%s=hex(%d):!!Error: Could allocate memory for data buffer, %d bytes requested.", dumpOut, vName, type, (dataSiz + 1 * sizeof(wchar_t)));
			free(dumpOut);
			dumpOut = tmpPtr;
			break;
		}
		vNameSiz++;
		r = RegEnumValueW(bKey, i, vName, &vNameSiz, NULL, &type, data, &dataSiz);
		if(r != ERROR_SUCCESS) {
			tmpPtr = mprintf(L"%s!!Error: Could not enumerate value: %d: Index: %d Name: %s\r\n", dumpOut, r, i, vName);
			free(dumpOut);
			dumpOut = tmpPtr;
			free(data);
			continue;
		}
		if(vName[0] == L'\0') {
			// a blank name means its the default key, use @ when exporting
			vName[0] = L'@';
			vName[1] = L'\0';
		}
		switch(type) {
			case REG_SZ:
			case REG_EXPAND_SZ:
				((wchar_t*)data)[dataSiz] = L'\0';
				r = 1;
				// determine if the string can be written to a file without special encoding
				for(unsigned int ii = 0; ii < ((dataSiz-1)/sizeof(wchar_t)); ii++) {
					if(iswprint(((wchar_t*)data)[ii])==0) {
						r = 0;
						break;
					}
				}
				if(r == 1) {
					tmpPtr = mprintf(L"%s%s=\"%s\"\r\n", dumpOut, vName, (wchar_t*)data);
					free(dumpOut);
					dumpOut = tmpPtr;
					break;
				}
			case REG_MULTI_SZ:
			case REG_BINARY:
				// the text has some non-printable values in it, we'll store it as hex
				if(type == REG_BINARY) {
					tmpPtr = mprintf(L"%s%s=hex:", dumpOut, vName);
				} else {
					tmpPtr = mprintf(L"%s%s=hex(%d):", dumpOut, vName, type);
				}
				free(dumpOut);
				dumpOut = tmpPtr;
				tmpPtr = (wchar_t*)calloc((dataSiz * 3)+2, sizeof(wchar_t));
				if(tmpPtr == NULL) {
					// couldn't allocate space for hex encoded output buffer
					tmpPtr = mprintf(L"%s!!Error: Could not allocate buffer for hex encoded output: %s Size: %d", dumpOut, vName, ((dataSiz * 3)+2 * sizeof(wchar_t)));
					free(dumpOut);
					dumpOut = tmpPtr;
					break;
				}
				for(unsigned int ii = 0; ii < dataSiz; ii++) {
					wsprintf((LPWSTR)&tmpPtr[ii*3], L"%2.2x,", data[ii]);
				}
				// this removes the trailing comma
				if(dataSiz>0) {
					tmpPtr[(dataSiz*3)-1] = L'\0';
				} else {
					tmpPtr[0] = L'\0';
				}
				ttmpPtr = mprintf(L"%s%s\r\n", dumpOut, tmpPtr);
				free(dumpOut);
				free(tmpPtr);
				dumpOut = ttmpPtr;
				break;
			case REG_QWORD:
				tmpPtr = mprintf(L"%s%s=qword:%ll\r\n", dumpOut, vName, (*((DWORD*)data)));
				free(dumpOut);
				dumpOut = tmpPtr;
				break;
			case REG_DWORD:
				tmpPtr = mprintf(L"%s%s=dword:%8.8X\r\n", dumpOut, vName, (*((DWORD*)data)));
				free(dumpOut);
				dumpOut = tmpPtr;
				break;
		}
		free(data);
	}
	free(vName);

	for(i = 0; ; i++) {
		nameBufSiz = 256;
		r = RegEnumKeyExW(bKey, i, nameBuf, &nameBufSiz, NULL, NULL, NULL, &ft);
		if(r != ERROR_SUCCESS) {
			if(r != ERROR_NO_MORE_ITEMS) {
				// TODO: Do something with this error
				tmpPtr = mprintf(L"%s!!Error: Could not enumerate subkeys: %d: Index: %d\r\n", dumpOut, r, i);
				free(dumpOut);
				dumpOut = tmpPtr;
			}
			break;
		}
		subKeyNamePath = mprintf(L"%s\\%s", regPath, nameBuf);
		tmpPtr = DumpRegistryKey(subKeyNamePath);
		free(subKeyNamePath);
		if(tmpPtr != NULL) {
			wchar_t *tt;
			tt = mprintf(L"%s%s", dumpOut, tmpPtr);
			free(tmpPtr);
			free(dumpOut);
			dumpOut = tt;
		}
	}
	RegCloseKey(bKey);
	return(dumpOut);
}

// used to delete the map created by CreateParamMap
void DeleteParamMap(map<wstring, wstring> *oldMap)
{
	if(oldMap == NULL) {
		return;
	}
	delete(oldMap);
}

// This value must be such that if multipled by 4, then divided by 3 the result is a whole number.
// The value of 3072 (3k) results in a 4096 (4k) block with no dangling characters when base64blockencode
// is called.
#define FLYTRAP_FILEREAD_BUFSIZE	3072

// creates a map<wstring, wstring> suitable for submission using the breakpad CrashReportSender.SendReport method
map<wstring, wstring> *CreateParamMap(const LPFLYTRAPPARAM params, const int params_len, const wchar_t *dumpId, const wchar_t *reportUrl)
{
	map<wstring, wstring> *paramsStr = new map<wstring, wstring>;
	if(paramsStr == NULL) {
		return(NULL);
	}
	(*paramsStr)[FLYTRAP_PARAM_FLYTRAPVERSION] = _T(FLYTRAP_STRPRODUCTVERSION);
	(*paramsStr)[FLYTRAP_PARAM_FLYTRAPBUILDFLAGS] = _T(FLYTRAP_STRSPECIALBUILD);
	if(dumpId == NULL) {
		(*paramsStr)[FLYTRAP_PARAM_CRASHID] = L"";
	} else {
		(*paramsStr)[FLYTRAP_PARAM_CRASHID] = dumpId;
	}
	if(reportUrl == NULL) {
		(*paramsStr)[FLYTRAP_PARAM_REPORTURL] = L"";
	} else {
		(*paramsStr)[FLYTRAP_PARAM_REPORTURL] = reportUrl;
	}

	FILE *fp;
	int iCount;
	wchar_t *offset;
	wchar_t nameBuf[64];
	unsigned char *inBuf = NULL;	// this is a char (not wchar_t) because its used for storing bytes of binary data
	unsigned char *tmpBuf = NULL;	// this is a char (not wchar_t) because its used for storing bytes of binary data
	unsigned char *b64OutBuf = NULL;
	wchar_t *outBuf = NULL;
	wchar_t *fnameBuf;
	wchar_t *tfnameBuf;
	wchar_t *tb;
	size_t fSize = 0;
	int fr;
	wchar_t *expandedFName = NULL;
	size_t obSize;
	wchar_t *baseOutBuf;
	int fen;
	int i;

	for(i = 0; i < params_len; i++) {
		if(params[i].name == NULL) {
			// blank entry, just skip it.  We could probably break out of the loop but we won't do that since
			// we may have a way to delete entries in the future
			continue;
		} else if(params[i].name[0] == L'\0') {
			// blank entry, just skip it.  We could probably break out of the loop but we won't do that since
			// we may have a way to delete entries in the future
			continue;
		} else if(wcscmp(params[i].name, FLYTRAP_PARAM_ATTACHFILES_PARAM L"s") == 0) {
			inBuf = (unsigned char*)calloc(FLYTRAP_FILEREAD_BUFSIZE, sizeof(char));
			if(inBuf == NULL) {
				// out of memory!
				// @TODO Do something about this error
				break;
			}
			tmpBuf = (unsigned char*)calloc((((FLYTRAP_FILEREAD_BUFSIZE*4)/3)+2), sizeof(char));
			if(tmpBuf == NULL) {
				// out of memory!
				// @TODO Do something about this error
				free(inBuf);
				break;
			}

			// Add the files specified
			if(params[i].value != NULL) {
				if(wcslen(params[i].value)>0) {
					iCount = 0;
					fnameBuf = wcsdup(params[i].value);
					tfnameBuf = fnameBuf;	// so we can free the original fnameBuf pointer later

					offset = wcschr(fnameBuf, L';');
					while(offset != NULL) {
						offset[0] = L'\0';
						offset++;

						// fnameBuf now represents the current filename we're trying to load
						// load the file and generate outBuf
						if(wcslen(fnameBuf) > 0) {
							expandedFName = ExpandEnvVarsInStr(fnameBuf);
							fp = _wfopen(expandedFName, L"rb");
							if(fp == NULL) {
								outBuf = mprintf(L"%s;-2;File could not be opened for reading: %d", expandedFName, _errno());
								free(expandedFName);
							} else {
								fseek(fp, 0, SEEK_END);
								size_t fOffset = ftell(fp);
								if(fOffset == 0) {
									// empty file, shortcut case
									outBuf = mprintf(L"%s;0;", expandedFName);
									free(expandedFName);
									fclose(fp);
								} else {
									fseek(fp, 0, SEEK_SET);
									obSize = (((fOffset * 4)/3) + ( ((fOffset%4) > 0) ? 1 : 0 ));
									fOffset = 0;
									b64OutBuf = (unsigned char*)calloc(obSize + 2, sizeof(char));
									if(b64OutBuf == NULL) {
										// @TODO Do something to handle this error!
									}
									baseOutBuf = (wchar_t*)b64OutBuf;
									while(!feof(fp)) {
										// loop through the file reading in the data and adding it to the base64 encoded output
										fr = fread(inBuf, sizeof(char), FLYTRAP_FILEREAD_BUFSIZE, fp);
										fen = ferror(fp);
										if(fen != 0) {
											free(baseOutBuf);
											outBuf = mprintf(L"Could not read from file: %d@%d", _errno(), fOffset);
											break;
										}
										fOffset += fr;
										if(fr == 0) {
											// this shouldn't ever happen, but just in case
											break;
										}
										b64OutBuf += base64blockencode(inBuf, b64OutBuf, fr);
										fSize += fr;
									}
									fclose(fp);
									tb = baseOutBuf;

									outBuf = mprintf(L"%s;%d;%S", expandedFName, fSize, tb);
									free(expandedFName);
									free(tb);
								}
							}
						} else {
							outBuf = wcsdup(L"File name missing;-1;");
						}
						wsprintf(nameBuf, FLYTRAP_PARAM_ATTACHFILES_PARAM L"_%d", iCount);
						// set the map value to our base64 encoded file data
						(*paramsStr)[nameBuf] = outBuf;
						// outBuf is no longer needed
						free(outBuf);
						fnameBuf = offset;
						offset = wcschr(fnameBuf, L';');
						iCount++;
					}
					if(fnameBuf != NULL) {
						// catch the remaining filename in the buffer
						// fnameBuf now represents the current filename we're trying to load
						// load the file and generate outBuf
						// fnameBuf now represents the current filename we're trying to load
						// load the file and generate outBuf
						if(wcslen(fnameBuf) > 0) {
							expandedFName = ExpandEnvVarsInStr(fnameBuf);
							fp = _wfopen(expandedFName, L"rb");
							if(fp == NULL) {
								outBuf = mprintf(L"%s;-2;File could not be opened for reading: %d", expandedFName, _errno());
								free(expandedFName);
							} else {
								fseek(fp, 0, SEEK_END);
								size_t fOffset = ftell(fp);
								if(fOffset == 0) {
									// empty file, shortcut case
									outBuf = mprintf(L"%s;0;", expandedFName);
									free(expandedFName);
									fclose(fp);
								} else {
									fseek(fp, 0, SEEK_SET);
									obSize = (((fOffset * 4)/3) + ( ((fOffset%4) > 0) ? 1 : 0 ));
									fOffset = 0;
									b64OutBuf = (unsigned char*)calloc(obSize + 2, sizeof(char));
									if(b64OutBuf == NULL) {
										// @TODO Do something to handle this error!
									}
									baseOutBuf = (wchar_t*)b64OutBuf;
									while(!feof(fp)) {
										// loop through the file reading in the data and adding it to the base64 encoded output
										fr = fread(inBuf, sizeof(char), FLYTRAP_FILEREAD_BUFSIZE, fp);
										fen = ferror(fp);
										if(fen != 0) {
											free(baseOutBuf);
											outBuf = mprintf(L"Could not read from file: %d@%d", _errno(), fOffset);
											break;
										}
										fOffset += fr;
										if(fr == 0) {
											// this shouldn't ever happen, but just in case
											break;
										}
										b64OutBuf += base64blockencode(inBuf, b64OutBuf, fr);
										fSize += fr;
									}
									fclose(fp);
									tb = baseOutBuf;

									outBuf = mprintf(L"%s;%d;%S", expandedFName, fSize, tb);
									free(expandedFName);
									free(tb);
								}
							}
						} else {
							outBuf = wcsdup(L"File name missing;-1;");
						}
						wsprintf(nameBuf, FLYTRAP_PARAM_ATTACHFILES_PARAM L"_%d", iCount);
						// set the map value to our base64 encoded file data
						(*paramsStr)[nameBuf] = outBuf;
						// outBuf is no longer needed
						free(outBuf);
						fnameBuf = offset;
						iCount++;
					}
					free(tfnameBuf);
				}
			}
			free(inBuf);
			free(tmpBuf);
		} else if(wcscmp(params[i].name, FLYTRAP_PARAM_ATTACHREGKEY_PARAM L"s") == 0) {
			// Dump the specified registry keys and include them
			if(params[i].value != NULL) {
				if(wcslen(params[i].value)>0) {
					iCount = 0;
					fnameBuf = wcsdup(params[i].value);
					tfnameBuf = fnameBuf;	// so we can free the original fnameBuf pointer later

					offset = wcschr(fnameBuf, L';');
					while(offset != NULL) {
						offset[0] = L'\0';
						offset++;
						// fnameBuf now represents the current registry path we're trying to export
						if(wcslen(fnameBuf) < 5) {
							outBuf = wcsdup(L"Registry path too short;-1;");
						} else {
							tb = DumpRegistryKey(fnameBuf);
							if(tb != NULL) {
								outBuf = mprintf(L"%s;%s", fnameBuf, tb);
								free(tb);
							} else {
								outBuf = mprintf(L"%s;Could not open registry key", fnameBuf);
							}
						}
						wsprintf(nameBuf, FLYTRAP_PARAM_ATTACHREGKEY_PARAM L"_%d", iCount);
						// set the map value to our base64 encoded data
						(*paramsStr)[nameBuf] = outBuf;
						// outBuf is no longer needed
						free(outBuf);
						fnameBuf = offset;
						offset = wcschr(fnameBuf, L';');
						iCount++;
					}
					if(fnameBuf != NULL) {
						// catch the remaining filename in the buffer
						// fnameBuf now represents the current registry path we're trying to export
						if(wcslen(fnameBuf) < 5) {
							outBuf = wcsdup(L"Registry path too short;-1;");
						} else {
							tb = DumpRegistryKey(fnameBuf);
							if(tb != NULL) {
								outBuf = mprintf(L"%s;%s", fnameBuf, tb);
								free(tb);
							} else {
								outBuf = mprintf(L"%s;Could not open registry key", fnameBuf);
							}
						}
						wsprintf(nameBuf, FLYTRAP_PARAM_ATTACHREGKEY_PARAM L"_%d", iCount);
						// set the map value to our base64 encoded data
						(*paramsStr)[nameBuf] = outBuf;
						// outBuf is no longer needed
						free(outBuf);
						iCount++;
					}
					free(tfnameBuf);
				}
			}
		}

		(*paramsStr)[params[i].name] = params[i].value;
	}
	return(paramsStr);
}

// given an input string inStrPtr, returns a string which is identical to inStrPtr except that it
// has all environment variables expanded, inStrPtr is freed and a new pointer is returned
wchar_t *ExpandEnvVarsInStr(const wchar_t *inStrPtr)
{
	DWORD sn = ExpandEnvironmentStringsW(inStrPtr, NULL, 0);
	wchar_t *outStrPtr = (wchar_t*)calloc(sn + 2, sizeof(wchar_t));
	ExpandEnvironmentStringsW(inStrPtr, outStrPtr, sn + 1);
	// ensure the string is always safely NULL terminated
	outStrPtr[sn+1] = L'\0';
	return(outStrPtr);
}

TCHAR *mprintf(const TCHAR *format, ...)
{
	va_list args;
    int len;
    TCHAR *buffer;

    // retrieve the variable arguments
    va_start(args, format);
    
    len = _vsctprintf(format, args) + 1;// _vscprintf doesn't count terminating '\0'
    
    buffer = (TCHAR*)calloc(len + 1, sizeof(TCHAR));

	// we do this manually because the macro VC provides in tchar.h changes between vc versions in a broken way
#ifdef UNICODE
	vswprintf(buffer, (size_t)len, format, args); // C4996
#else
	vsprintf(buffer, (size_t)len, format, args); // C4996
#endif
    // Note: vsprintf is deprecated; consider using vsprintf_s instead
	return(buffer);
}
