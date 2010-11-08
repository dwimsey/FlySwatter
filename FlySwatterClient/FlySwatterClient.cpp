// Copyright 2005-2007, Research Triangle Software, Inc.  All rights reserved.
/*!	@file main.c
 * This file contains the bulk of the code used support for encrypted archives created
 * using the RTS cryptography library
 *
 * @brief RTS crypto library main implementation
 * @author David Wimsey <dwimsey@rtsz.com>
 * @version $Revision: 1.130 $
 * @date 2005-2007
 * @todo Zero byte files should never hit the compression engine, as they just add space rather than save it
 * @todo add metadata file type which will never be directly decrypted/decompressed, for storing future archive info
 */

// FlySwatter.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

// these are in librtslocalize, which we don't want to include just yet
#ifdef WIN32
#define tstrcpy wcscpy
#define tstrdup _wcsdup
#endif

#define MINIDUMP_MAX_PATH_SIZE (1<15)-2
#define MESSAGE_BUFFER_LEN	(1<<20)
#define MAX_PATH_LEN	32768
#define SAFE_MAX_PATH	(32000)

typedef struct __FlySwatterContextData {
	volatile int enabled;
	wchar_t *reportUrl;
	//CrashReportSender *reportSender;
	ExceptionHandler *handlerPtr;
	wchar_t dumpPath[MINIDUMP_MAX_PATH_SIZE+3];
	FLYSWATTERPARAM *params;
	int params_len;
} FLYSWATTERCONTEXT, *LPFLYSWATTERCONTEXT;

typedef struct __FlySwatterServerContextData {
	CrashGenerationServer *serverPtr;
} FLYSWATTERSERVERCONTEXT, *LPFLYSWATTERSERVERCONTEXT;

LPFLYSWATTERCONTEXT lctx = NULL;
const wchar_t FSkPipeName[] = L"\\\\.\\pipe\\FlySwatter\\%d";

extern "C" __declspec (dllexport) void __cdecl FlySwatterRunDLLLaunchServerW(HWND hWnd, HINSTANCE hInstance, wchar_t *lpCmdLine, int nCmdShow)
{
	int r;
	wchar_t *tt;
	wchar_t *offPtr;
	wchar_t *pidStr;
	wchar_t *pipeName;
	wchar_t *reportUrl;
	wchar_t *dumpPath;

	int i = MESSAGE_BUFFER_LEN;
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

	DWORD pid = _wtol(pidStr);

	LPFLYSWATTERSERVERCONTEXT sCtx;
	sCtx = FlySwatterInitServer(pipeName, reportUrl, dumpPath);
	if(sCtx != NULL) {
//		wsprintf(pMessageBuffer, L""
		MessageBoxW(NULL, pipeName, L"Could not initialized server pipe.", MB_OK);
		return;
	}
	// Open the calling process to wait for it to finish.
	HANDLE pHandle = OpenProcess(SYNCHRONIZE, FALSE, pid);
	// When the wait returns, the original process has exited and so should we
	WaitForSingleObject(pHandle, INFINITE);

	FlySwatterShutdownServer(sCtx);
}

// this is just a wrapper to convert to unicode and invoke the unicode version of this function
extern "C" __declspec (dllexport) void __cdecl FlySwatterRunDLLLaunchServer(HWND hWnd, HINSTANCE hInstance, char *lpCmdLineA, int nCmdShow)
{
	size_t szCmdLine;
	wchar_t *lpCmdLine;

	szCmdLine = strlen(lpCmdLineA) + 1;
	lpCmdLine = (wchar_t*)malloc((szCmdLine + 1)*sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, lpCmdLineA, szCmdLine, lpCmdLine, szCmdLine);
	FlySwatterRunDLLLaunchServerW(hWnd, hInstance, lpCmdLine, nCmdShow);
	free(lpCmdLine);
}

// this function uses VirtualAlloc and VirtualProtect in an effort to secure the context structure against corruption
// it won't always work, but its better than nothing.
 
FLYSWATTER_API int __stdcall FlySwatterInitClient(wchar_t *dumpPath, wchar_t *reportUrl, wchar_t *OOPExePath)
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
		ExpandEnvironmentStringsW(dumpPath, (LPTSTR)&lctx->dumpPath, MINIDUMP_MAX_PATH_SIZE);
		lctx->dumpPath[MINIDUMP_MAX_PATH_SIZE] = _T('\0');
		lctx->dumpPath[MINIDUMP_MAX_PATH_SIZE-1] = _T('\0');
	}
	lctx->reportUrl = wcsdup(reportUrl);
	//lctx->reportSender = new CrashReportSender(_T(""));
	// make sure the dump path exists if possible
	_wmkdir(lctx->dumpPath);
	if(OOPExePath != NULL && wcslen(OOPExePath) > 0) {
		// try out of process
		if(wcscmp(OOPExePath, L"FlySwatter.dll")==0) {
			// try to use rundll to launch this DLL again as a server
			wchar_t *fpath = ExpandEnvVarsInStr(L"%SystemRoot%\\System32\\rundll32.exe");
			wchar_t *dllpath = ExpandEnvVarsInStr(L"%SystemRoot%\\System32\\rundll32.exe");

			// Generate the pipe path
			wchar_t pidstr[128];
			DWORD pid = GetCurrentProcessId();
			wsprintf(pidstr, L"%d", pid);
			wchar_t pathstr[1024];
			wsprintf(pathstr, FSkPipeName, pid);

			// Generate the server DLL path
			HMODULE sModule = GetModuleHandleW(L"FlySwatter");
			if(sModule != NULL) {
				//GetModuleFileNameW(sModule, pMessageBuffer, MESSAGE_BUFFER_LEN);
				/// @TODO The spawn call below needs to dynamically find the path to
				// ourself to pass to rundll32.exe
				//wsprintf(pMessageBuffer, L"");
			} else {
				// Our DLL has been renamed, but try the default anyway
				//wcscpy(pMessageBuffer, "FlySwatter.dll");
			}
			int r = _wspawnl(_P_DETACH, fpath, fpath, L"C:\\src\\FlySwatter\\DebugStaticCRT\\FlySwatter.dll,FlySwatterRunDLLLaunchServer", pidstr, pathstr, reportUrl, dumpPath, NULL);
			int eNo = *_errno();
			free(fpath);
			free(dllpath);
			if(r > -1) {
					MessageBox(NULL, pathstr, L"Could not initialized client pipe.", MB_OK);

				// OOP Server was 'launched', now lets see if we can talk to it

				// WaitForSingleObject(h, 2000);
				lctx->handlerPtr = new ExceptionHandler((wchar_t*)&lctx->dumpPath, FlySwatterExceptionFilter, FlySwatterInProcessDumpCallback, lctx, ExceptionHandler::HANDLER_ALL, MiniDumpNormal, pathstr, NULL);//custom_client_info_c);
			}
		}
	}

	if(lctx->handlerPtr == NULL) {
		// no crash CrashGenerationClient exists, so fall back to internal
		// exception handling
		lctx->handlerPtr = new ExceptionHandler((wchar_t*)&lctx->dumpPath, FlySwatterExceptionFilter, FlySwatterInProcessDumpCallback, lctx, ExceptionHandler::HANDLER_ALL);
	}
	// for now we enable at init automatically.  This allows us to catch the earliest possible crash, even in our own code possibly.
	lctx->enabled = 1;
	return((lctx->handlerPtr == NULL ? 0 : 1));
}

#include "client/windows/crash_generation/crash_generation_server.h"

/*!
 *
 */

FLYSWATTER_API bool __stdcall FlySwatterShutdownServer(wchar_t *pipeName, wchar_t *reportUrl, wchar_t *dumpPath)
{
	return(false);
}

FLYSWATTER_API void * __stdcall FlySwatterInitServer(wchar_t *pipeName, wchar_t *reportUrl, wchar_t *dumpPath)
{
//	TCHAR *expandedDumpPath = NULL;
//	size_t edpSize = 0;

	// Allocate some memory that we can protect later, we want to do it at the top of the address space
	// to put it as far away from the application as possible, this of course assumes that the heap is
	// at the bottom of the address space
	LPFLYSWATTERSERVERCONTEXT serverContext = (LPFLYSWATTERSERVERCONTEXT)VirtualAlloc(NULL, sizeof(FLYSWATTERSERVERCONTEXT), MEM_COMMIT | MEM_RESERVE | MEM_TOP_DOWN, PAGE_READWRITE);
	if(serverContext == NULL) {
		return(NULL);
	}
	// We shouldn't need to do this as VirtualAlloc is supposed to do it
	// But we do it anyway so we know where we stand, crash handlers need special care
	SecureZeroMemory(serverContext, sizeof(FLYSWATTERCONTEXT));

	wstring rURL(reportUrl);
	wstring rPipePath (pipeName);
	wstring rDumpPath (dumpPath);
	serverContext->server = new CrashGenerationServer(rPipePath, NULL, NULL, NULL, (google_breakpad::CrashGenerationServer::OnClientDumpRequestCallback)&FlySwatterOutOfProcessDumpCallback, NULL, NULL, NULL, true, &rDumpPath);
	if(serverContext->server->Start() == true) {
		return(serverContext);
	}

	return(serverContext);	
}

FLYSWATTER_API int __stdcall FlySwatterEnable()
{
	if(lctx == NULL) {
		return(-1);
	}
	if(lctx->handlerPtr == NULL) {
		return(-2);
	}
	return(InterlockedExchange((volatile LONG *)&lctx->enabled, 1));
}

FLYSWATTER_API int __stdcall FlySwatterDisable()
{
	if(lctx == NULL) {
		return(-1);
	}
	if(lctx->handlerPtr == NULL) {
		return(-2);
	}
	return(InterlockedExchange((volatile LONG *)&lctx->enabled, 0));
}

FLYSWATTER_API int __stdcall FlySwatterIsEnabled()
{
	if(lctx == NULL) {
		return(-1);
	}
	if(lctx->handlerPtr == NULL) {
		return(-2);
	}
	return(lctx->enabled);
}

FLYSWATTER_API void __stdcall FlySwatterSetParam(const wchar_t *name, const wchar_t *value)
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

FLYSWATTER_API const wchar_t * __stdcall FlySwatterGetParam(const wchar_t *name)
{
	if(lctx == NULL) {
		return(NULL);
	}
	return(FlySwatterGetParamEx(lctx->params, lctx->params_len, name));
}

bool FlySwatterExceptionFilter(void *ctx, EXCEPTION_POINTERS *exceptionInfo, MDRawAssertionInfo *assertionInfo)
{
#ifdef _DEBUG
	MessageBox(NULL, L"Attach!", L"Attach!", MB_OK);
#endif
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

void FlySwatterOutOfProcessDumpCallback(void *dump_context, ClientInfo *client_info, const std::wstring *dump_path)
{
	MessageBox(NULL, L"OOPAttach!", L"OOPAttach!", MB_OK);
}

bool FlySwatterInProcessDumpCallback(const wchar_t *dumpPath, const wchar_t *dumpId, void *mctx, EXCEPTION_POINTERS *exceptionInfo, MDRawAssertionInfo *assertionInfo, bool dumpSucceeded)
{
	LPFLYSWATTERCONTEXT ctx = (LPFLYSWATTERCONTEXT)mctx;
	if(dumpSucceeded == false) {
		// send the crash report even if a dump could not be created
		// return(false);
	}

	wchar_t miniDumpFilename[1025];
	if(dumpSucceeded == true) {
		if(dumpPath == NULL) {
			dumpPath = L"";
		}
		if(dumpId == NULL) {
			dumpId = L"";
		}
		_sntprintf(miniDumpFilename, 1023, L"%s\\%s.dmp", dumpPath, dumpId);
		miniDumpFilename[1023] = L'\0';
		FlySwatterCrashAlert(ctx->reportUrl, miniDumpFilename, ctx->params, ctx->params_len);
		// the minidump should already be gone, but we're going to remove it again anyway to be safe
		_wunlink(miniDumpFilename);
	} else {
		FlySwatterCrashAlert(ctx->reportUrl, NULL, ctx->params, ctx->params_len);
	}
// no need to relock it, we're going to die now anyway
//	int rVal;
//	DWORD bb;
//	rVal = VirtualProtect(lctx, sizeof(FLYSWATTERCONTEXT), PAGE_NOACCESS, &bb);
//	if(rVal == 0) {
//		rVal = GetLastError();
//	}
//	rVal = ((lctx->handlerPtr == NULL) ? 0 : 1);

#ifdef WIN32
	_exit(255);
#else
	exit(255);
#endif
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
		r = RegEnumValue(bKey, i, vName, &vNameSiz, NULL, &type, NULL, &dataSiz);
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
			tmpPtr = mprintf(L"%s\"%s\"=hex(%d):!!Error: Could allocate memory for data buffer, %d bytes requested.", dumpOut, vName, type, (dataSiz + 1 * sizeof(wchar_t)));
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
					tmpPtr = mprintf(L"%s\"%s\"=\"%s\"\r\n", dumpOut, vName, (wchar_t*)data);
					free(dumpOut);
					dumpOut = tmpPtr;
					break;
				}
			case REG_MULTI_SZ:
			case REG_EXPAND_SZ:
			case REG_BINARY:
				// the text has some non-printable values in it, we'll store it as hex
				if(type == REG_BINARY) {
					tmpPtr = mprintf(L"%s\"%s\"=hex:", dumpOut, vName);
				} else {
					tmpPtr = mprintf(L"%s\"%s\"=hex(%d):", dumpOut, vName, type);
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
				tmpPtr = mprintf(L"%s\"%s\"=qword:%ll\r\n", dumpOut, vName, (*((DWORD*)data)));
				free(dumpOut);
				dumpOut = tmpPtr;
				break;
			case REG_DWORD:
				tmpPtr = mprintf(L"%s\"%s\"=dword:%8.8X\r\n", dumpOut, vName, (*((DWORD*)data)));
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

// creates a map<wstring, wstring> suitable for submission using the breakpad CrashReportSender.SendReport method
map<wstring, wstring> *CreateParamMap(const LPFLYSWATTERPARAM params, const int params_len, const wchar_t *dumpId, const wchar_t *reportUrl)
{
	map<wstring, wstring> *paramsStr = new map<wstring, wstring>;
	if(paramsStr == NULL) {
		return(NULL);
	}
	(*paramsStr)[L"FlySwatterVersion"] = _T(FLYSWATTER_VERSION_STRING);
	(*paramsStr)[L"FlySwatterCrashId"] = dumpId;
	(*paramsStr)[L"FlySwatterReportURL"] = reportUrl;

	for(int i = 0; i < params_len; i++) {
		if(params[i].name == NULL) {
			// blank entry, just skip it.  We could probably break out of the loop but we won't do that since
			// we may have a way to delete entries in the future
			continue;
		}
		if(params[i].name[0] == L'\0') {
			// blank entry, just skip it.  We could probably break out of the loop but we won't do that since
			// we may have a way to delete entries in the future
			continue;
		}
		if(wcsncmp(params[i].name, L"FlySwatter_CrashAlertDialog_", wcslen(L"FlySwatter_CrashAlertDialog_")) == 0) {
			// Parameters that start with FlySwatter_CrashAlertDialog_ are for use by the dialog display only
			continue;
		}
		(*paramsStr)[params[i].name] = params[i].value;

		FILE *fp;
		int iCount;
		wchar_t *offset;
		wchar_t nameBuf[64];
		unsigned char inBuf[512];	// this is a char (not wchar_t) because its used for storing bytes of binary data
		wchar_t *outBuf = NULL;
		wchar_t *fnameBuf;
		wchar_t *tfnameBuf;
		unsigned char *encodedStr;
		int encSize;
		wchar_t *wencodedStr;
		wchar_t *tb;
		int fSize = 0;
		int fr;

		if(wcscmp(params[i].name, L"FlySwatter_AttachFiles") == 0) {
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
							fp = _wfopen(fnameBuf, L"rb");
							if(fp == NULL) {
								outBuf = mprintf(L"%s;%d;%s", fnameBuf, -2, L"File could not be opened for reading: %d", _errno());
							} else {
								outBuf = wcsdup(L"");
								while(!feof(fp)) {
									// loop through the file reading in the data and adding it to the base64 encoded output
									fr = fread(&inBuf, 1, 512, fp);
									int fen = ferror(fp);
									if(fen != 0) {
										tb = outBuf;
										outBuf = mprintf(L"%sCould not read from file: %d", tb, _errno());
										free(tb);
										break;
									}
									if(fr == 0) {
										// this shouldn't ever happen, but just in case
										break;
									}
									fSize += fr;
									encodedStr = base64encode(inBuf, fr, -1);
									encSize = strlen((const char *)encodedStr) + 1;
									wencodedStr = (wchar_t *)calloc(encSize + 1, sizeof(wchar_t));
									mbstowcs(wencodedStr, (const char *)encodedStr, encSize + 1);
									free(encodedStr);
									tb = outBuf;
									outBuf = mprintf(L"%s%s", tb, wencodedStr);
									free(tb);
									free(wencodedStr);
								}
								fclose(fp);
								tb = outBuf;
								outBuf = mprintf(L"%s;%d;%s", fnameBuf, fSize, tb);
								free(tb);
							}
						} else {
							outBuf = wcsdup(L"File name missing;-1;");
						}
						wsprintf(nameBuf, L"FlySwatter_AttachFiles_%d", iCount);
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
						if(wcslen(fnameBuf) > 0) {
							fp = _wfopen(fnameBuf, L"rb");
							if(fp == NULL) {
								outBuf = mprintf(L"%s;-2;%s", fnameBuf, L"File could not be opened for reading: %d.", _errno());
							} else {
								outBuf = wcsdup(L"");
								while(!feof(fp)) {
									// loop through the file reading in the data and adding it to the base64 encoded output
									fr = fread(&inBuf, 1, 512, fp);
									int fen = ferror(fp);
									if(fen != 0) {
										tb = outBuf;
										outBuf = mprintf(L"%sCould not read from file: %d", tb, _errno());
										free(tb);
										break;
									}
									if(fr == 0) {
										// this shouldn't ever happen, but just in case
										break;
									}

									fSize += fr;
									encodedStr = base64encode(inBuf, fr, -1);
									encSize = strlen((const char *)encodedStr) + 1;
									wencodedStr = (wchar_t *)calloc(encSize + 1, sizeof(wchar_t));
									mbstowcs(wencodedStr, (const char *)encodedStr, encSize + 1);
									free(encodedStr);
									tb = outBuf;
									outBuf = mprintf(L"%s%s", tb, wencodedStr);
									free(tb);
									free(wencodedStr);
								}
								fclose(fp);
								tb = outBuf;
								outBuf = mprintf(L"%s;%d;%s", fnameBuf, fSize, tb);
								free(tb);
							}
						} else {
							outBuf = wcsdup(L"File name missing;-1;");
						}
						wsprintf(nameBuf, L"FlySwatter_AttachFiles_%d", iCount);
						// set the map value to our base64 encoded file data
						(*paramsStr)[nameBuf] = outBuf;
						// outBuf is no longer needed
						free(outBuf);
						iCount++;
					}
					free(tfnameBuf);
				}
			}
		}

		if(wcscmp(params[i].name, L"FlySwatter_AttachRegKeys") == 0) {
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
						wsprintf(nameBuf, L"FlySwatter_AttachRegKeys_%d", iCount);
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
						wsprintf(nameBuf, L"FlySwatter_AttachRegKeys_%d", iCount);
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
