#ifndef FLYSWATTER_H
#define FLYSWATTER_H		$Id: flyswatterclient.h,v 1.3 2010-02-18 08:36:35 bits Exp $

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

#define FLYSWATTERINIT(BASENAME, DUMPPATH, REPORTURL, OOP_EXE_PATH, DLLPATH) \
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
	BASENAME ## Init(DUMPPATH, REPORTURL, OOP_EXE_PATH); \
	BASENAME ## Enable(); \

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
