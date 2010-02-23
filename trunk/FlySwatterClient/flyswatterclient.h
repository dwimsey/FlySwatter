#ifndef FLYSWATTER_H
#define FLYSWATTER_H		$Id: flyswatterclient.h,v 1.4 2010-02-23 07:48:05 bits Exp $

#include <windows.h>

typedef struct __FlySwatterContextParameter {
	wchar_t *name;
	wchar_t *value;
} FLYSWATTERPARAM, *LPFLYSWATTERPARAM;

typedef int (__stdcall *flyswatter_init_func_ptr)(wchar_t *dumpPath, wchar_t *reportUrl, wchar_t *OOPExePath);
typedef int (__stdcall *flyswatter_initserver_func_ptr)(wchar_t *pipeName, wchar_t *reportUrl);
typedef int (__stdcall *flyswatter_enable_func_ptr)();
typedef int (__stdcall *flyswatter_disable_func_ptr)();
typedef int (__stdcall *flyswatter_isenabled_func_ptr)();
typedef int (__stdcall *flyswatter_crashalert_func_ptr)(const wchar_t *reportUrl, const wchar_t *miniDumpFilename, const LPFLYSWATTERPARAM params, const int params_len);
typedef void (__stdcall *flyswatter_setparam_func_ptr)(const wchar_t *name, const wchar_t *value);
typedef const wchar_t *(__stdcall *flyswatter_getparam_func_ptr)(const wchar_t *name);

// These macros make initializing the crash handler easier
#define FLYSWATTERVARPOINTERS(BASENAME) \
	HMODULE BASENAME ## LibraryHandle = NULL; \
	extern "C" { \
		flyswatter_init_func_ptr BASENAME ## Init; \
		flyswatter_initserver_func_ptr BASENAME ## InitServer; \
		flyswatter_enable_func_ptr BASENAME ## Enable; \
		flyswatter_disable_func_ptr BASENAME ## Disable; \
		flyswatter_isenabled_func_ptr BASENAME ## IsEnabled; \
		flyswatter_crashalert_func_ptr BASENAME ## CrashAlert; \
		flyswatter_setparam_func_ptr BASENAME ## SetParam; \
		flyswatter_getparam_func_ptr BASENAME ## GetParam; \
	} \

#define FLYSWATTERINIT(RVAL, BASENAME, DUMPPATH, REPORTURL, OOP_EXE_PATH, DLLPATH) \
	BASENAME ## LibraryHandle = LoadLibrary(DLLPATH); \
	if(BASENAME ## LibraryHandle == NULL) { \
		RVAL = -101; \
	} else { \
		BASENAME ## Init = (flyswatter_init_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlySwatterInit"); \
		if(BASENAME ## Init == NULL) { \
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
		\
		RVAL = BASENAME ## Init(DUMPPATH, REPORTURL, OOP_EXE_PATH); \
	}

#define FLYSWATTERINITSERVER(BASENAME, PIPENAME, REPORTURL, DLLPATH) \
	BASENAME ## LibraryHandle = LoadLibrary(DLLPATH); \
	if(BASENAME ## LibraryHandle == NULL) { \
		MessageBox(NULL, _T("Could not load FlySwatter.dll"), _T("Initialization Error"), MB_OK); \
		exit(255); \
	} \
	BASENAME ## Init = (flyswatter_init_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlySwatterInit"); \
	BASENAME ## InitServer = (flyswatter_initserver_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlySwatterInitServer"); \
	BASENAME ## Enable = (flyswatter_enable_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlySwatterEnable"); \
	BASENAME ## Disable = (flyswatter_disable_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlySwatterDisable"); \
	BASENAME ## IsEnabled = (flyswatter_isenabled_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlySwatterIsEnabled"); \
	BASENAME ## SetParam = (flyswatter_setparam_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlySwatterSetParam"); \
	BASENAME ## GetParam = (flyswatter_getparam_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlySwatterGetParam"); \
	\
	BASENAME ## InitServer(PIPENAME, REPORTURL); \

#endif
