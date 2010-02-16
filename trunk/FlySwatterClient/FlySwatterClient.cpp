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
	// make sure the dump path exists if possible
	_wmkdir(lctx->dumpPath);
	lctx->handlerPtr = new ExceptionHandler((wchar_t*)&lctx->dumpPath, FlySwatterExceptionFilter, FlySwatterMiniDumpCallback, lctx, ExceptionHandler::HANDLER_ALL);
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
	MessageBox(NULL, L"Attach!", L"Attach!", MB_OK);
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


// misc functions
HKEY GetRootKeyHandleFromPathStr(const wchar_t *path)
{
	if(wcsnicmp(path, L"HKCR\\", 5)==0) {
		return(HKEY_CLASSES_ROOT);
	} else if(wcsnicmp(path, L"HKCU\\", 5)==0) {
		return(HKEY_CURRENT_USER);
	} else if(wcsnicmp(path, L"HKLM\\", 5)==0) {
		return(HKEY_LOCAL_MACHINE);
	} else if(wcsnicmp(path, L"HKCC\\", 5)==0) {
		return(HKEY_CURRENT_CONFIG);
//	} else if(wcsnicmp(fnameBuf, L"HKCC\\", 5)==0) {
//		return(HKEY_USERS);
	} else {
		return(0);
	}
}

wchar_t *DumpRegistryKey(const HKEY regPath)
{
	return(NULL);
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
	(*paramsStr)[L"FlySwatterVersion"] = _T(FLYSWATTER_VERSION_STRING);
	(*paramsStr)[L"FlySwatterCrashId"] = dumpId;
	(*paramsStr)[L"FlySwatterReportURL"] = reportUrl;

	for(int i = 0; i < params_len; i++) {
		if(params[i].name == NULL) {
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
								outBuf = mprintf(L"%s;%d;%s", fnameBuf, -2, L"File could not be opened for reading");
							} else {
								outBuf = wcsdup(L"");
								while(1) {
									// loop through the file reading in the data and adding it to the base64 encoded output
									fr = fread(&inBuf, 1, 512, fp);
									if(fr == 0) {
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
								outBuf = mprintf(L"%s;%d;%s", fnameBuf, -2, L"File could not be opened for reading");
							} else {
								outBuf = wcsdup(L"");
								while(1) {
									// loop through the file reading in the data and adding it to the base64 encoded output
									fr = fread(&inBuf, 1, 512, fp);
									if(fr == 0) {
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
							HKEY bKey;
							if(RegOpenKeyW(GetRootKeyHandleFromPathStr(fnameBuf), (wchar_t*)&fnameBuf[5], &bKey) == ERROR_SUCCESS) {
								tb = DumpRegistryKey(bKey);
								RegCloseKey(bKey);
								outBuf = mprintf(L"%s;%s", fnameBuf, tb);
								free(tb);
							} else {
								outBuf = mprintf(L"%s;Registry not open registry key", fnameBuf);
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
							HKEY bKey;
							if(RegOpenKeyW(GetRootKeyHandleFromPathStr(fnameBuf), (wchar_t*)&fnameBuf[5], &bKey) == ERROR_SUCCESS) {
								tb = DumpRegistryKey(bKey);
								RegCloseKey(bKey);
								outBuf = mprintf(L"%s;%s", fnameBuf, tb);
								free(tb);
							} else {
								outBuf = mprintf(L"%s;Registry not open registry key", fnameBuf);
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
