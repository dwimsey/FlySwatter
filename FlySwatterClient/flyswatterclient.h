#ifndef FLYSWATTER_H
#define FLYSWATTER_H		$Id: flyswatterclient.h,v 1.4 2010-02-23 07:48:05 bits Exp $

#include <windows.h>

typedef struct __FlySwatterContextParameter {
	wchar_t *name;
	wchar_t *value;
} FLYSWATTERPARAM, *LPFLYSWATTERPARAM;

typedef int (__stdcall *flyswatter_initclient_func_ptr)(wchar_t *dumpPath, wchar_t *reportUrl, wchar_t *OOPExePath);
typedef int (__stdcall *flyswatter_initserver_func_ptr)(wchar_t *pipeName, wchar_t *reportUrl);
typedef int (__stdcall *flyswatter_enable_func_ptr)();
typedef int (__stdcall *flyswatter_disable_func_ptr)();
typedef int (__stdcall *flyswatter_isenabled_func_ptr)();
typedef int (__stdcall *flyswatter_crashalert_func_ptr)(const wchar_t *reportUrl, const wchar_t *miniDumpFilename, const LPFLYSWATTERPARAM params, const int params_len);
typedef void (__stdcall *flyswatter_setparam_func_ptr)(const wchar_t *name, const wchar_t *value);
typedef const wchar_t *(__stdcall *flyswatter_getparam_func_ptr)(const wchar_t *name);
typedef void (__stdcall *flyswatter_triggerreport_func_ptr)(void);

// These macros make initializing the crash handler easier
#define FLYSWATTERVARPOINTERS(BASENAME) \
	HMODULE BASENAME ## LibraryHandle = NULL; \
	extern "C" { \
		flyswatter_initclient_func_ptr BASENAME ## InitClient; \
		flyswatter_initserver_func_ptr BASENAME ## InitServer; \
		flyswatter_enable_func_ptr BASENAME ## Enable; \
		flyswatter_disable_func_ptr BASENAME ## Disable; \
		flyswatter_isenabled_func_ptr BASENAME ## IsEnabled; \
		flyswatter_crashalert_func_ptr BASENAME ## CrashAlert; \
		flyswatter_setparam_func_ptr BASENAME ## SetParam; \
		flyswatter_getparam_func_ptr BASENAME ## GetParam; \
		flyswatter_triggerreport_func_ptr BASENAME ## TriggerReport; \
	} \

#define FLYSWATTERLOADLIBRARY(RVAL, BASENAME, DLLPATH) \
	BASENAME ## LibraryHandle = LoadLibrary(DLLPATH); \
	if(BASENAME ## LibraryHandle == NULL) { \
		RVAL = -101; \
	} else { \
		BASENAME ## InitClient = (flyswatter_initclient_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlySwatterInitClient"); \
		if(BASENAME ## InitClient == NULL) { \
			RVAL = -102; \
		} \
		BASENAME ## InitServer = (flyswatter_initserver_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlySwatterInitServer"); \
		if(BASENAME ## InitServer == NULL) { \
			RVAL = -103; \
		} \
		BASENAME ## Enable = (flyswatter_enable_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlySwatterEnable"); \
		if(BASENAME ## Enable == NULL) { \
			RVAL = -104; \
		} \
		BASENAME ## Disable = (flyswatter_disable_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlySwatterDisable"); \
		if(BASENAME ## Disable == NULL) { \
			RVAL = -105; \
		} \
		BASENAME ## IsEnabled = (flyswatter_isenabled_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlySwatterIsEnabled"); \
		if(BASENAME ## IsEnabled == NULL) { \
			RVAL = -106; \
		} \
		BASENAME ## SetParam = (flyswatter_setparam_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlySwatterSetParam"); \
		if(BASENAME ## SetParam == NULL) { \
			RVAL = -107; \
		} \
		BASENAME ## GetParam = (flyswatter_getparam_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlySwatterGetParam"); \
		if(BASENAME ## GetParam == NULL) { \
			RVAL = -108; \
		} \
		BASENAME ## TriggerReport = (flyswatter_triggerreport_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlySwatterTriggerReport"); \
		if(BASENAME ## TriggerReport == NULL) { \
			RVAL = -109; \
		} \
	}

#define FLYSWATTERINITCLIENT(RVAL, BASENAME, DUMPPATH, REPORTURL, OOP_EXE_PATH, DLLPATH) \
	FLYSWATTERLOADLIBRARY(RVAL, BASENAME, DLLPATH); \
	if(BASENAME ## LibraryHandle != NULL) { \
		RVAL = BASENAME ## InitClient(DUMPPATH, REPORTURL, OOP_EXE_PATH); \
	}

#endif
