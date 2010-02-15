#ifndef FLYSWATTER_H
#define FLYSWATTER_H		$Id: flyswatterclient.h,v 1.2 2010-02-15 21:02:33 bits Exp $

#include <windows.h>

typedef struct __FlySwatterContextParameter {
	wchar_t *name;
	wchar_t *value;
} FLYSWATTERPARAM, *LPFLYSWATTERPARAM;

typedef int (*flyswatter_init_func_ptr)(wchar_t *dumpPath, wchar_t *reportUrl, wchar_t *OOPExePath);
typedef int (*flyswatter_enable_func_ptr)();
typedef int (*flyswatter_disable_func_ptr)();
typedef int (*flyswatter_isenabled_func_ptr)();
typedef int (*flyswatter_crashalert_func_ptr)(const wchar_t *reportUrl, const wchar_t *miniDumpFilename, const LPFLYSWATTERPARAM params, const int params_len);
typedef void (*flyswatter_setparam_func_ptr)(const wchar_t *name, const wchar_t *value);
typedef const wchar_t *(*flyswatter_getparam_func_ptr)(const wchar_t *name);

// These macros make initializing the crash handler easier
#define FLYSWATTERVARPOINTERS(BASENAME) \
	HMODULE BASENAME ## LibraryHandle = NULL; \
	extern "C" { \
		flyswatter_init_func_ptr BASENAME ## Init; \
		flyswatter_enable_func_ptr BASENAME ## Enable; \
		flyswatter_disable_func_ptr BASENAME ## Disable; \
		flyswatter_isenabled_func_ptr BASENAME ## IsEnabled; \
		flyswatter_crashalert_func_ptr BASENAME ## CrashAlert; \
		flyswatter_setparam_func_ptr BASENAME ## SetParam; \
		flyswatter_getparam_func_ptr BASENAME ## GetParam; \
	} \

#define FLYSWATTERINIT(BASENAME, DUMPPATH, REPORTURL, OOP_EXE_PATH) \
	BASENAME ## LibraryHandle = LoadLibrary(_T("FlySwatter.dll")); \
	if(BASENAME ## LibraryHandle == NULL) { \
		MessageBox(NULL, _T("Could not load FlySwatter.dll"), _T("Initialization Error"), MB_OK); \
		exit(255); \
	} \
	BASENAME ## Init = (flyswatter_init_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlySwatterInit"); \
	BASENAME ## Enable = (flyswatter_enable_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlySwatterEnable"); \
	BASENAME ## Disable = (flyswatter_disable_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlySwatterDisable"); \
	BASENAME ## IsEnabled = (flyswatter_isenabled_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlySwatterIsEnabled"); \
	BASENAME ## SetParam = (flyswatter_setparam_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlySwatterSetParam"); \
	BASENAME ## GetParam = (flyswatter_getparam_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlySwatterGetParam"); \
	\
	BASENAME ## Init(DUMPPATH, REPORTURL, OOP_EXE_PATH); \
	BASENAME ## Enable(); \

#endif
