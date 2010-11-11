#ifndef __FLYTRAP_H_INCLUDED__
#define __FLYTRAP_H_INCLUDED__		$Id: FlyTrap.h,v 1.4 2010-02-23 07:48:05 bits Exp $

#include <windows.h>

typedef struct __FlyTrapContextParameter {
	wchar_t *name;
	wchar_t *value;
} FLYTRAPPARAM, *LPFLYTRAPPARAM;

typedef int (__stdcall *flytrap_initclient_func_ptr)(wchar_t *dumpPath, wchar_t *reportUrl, wchar_t *OOPExePath);
typedef int (__stdcall *flytrap_initserver_func_ptr)(wchar_t *pipeName, wchar_t *reportUrl);
typedef int (__stdcall *flytrap_enable_func_ptr)();
typedef int (__stdcall *flytrap_disable_func_ptr)();
typedef int (__stdcall *flytrap_isenabled_func_ptr)();
typedef int (__stdcall *flytrap_crashalert_func_ptr)(const wchar_t *reportUrl, const wchar_t *miniDumpFilename, const LPFLYTRAPPARAM params, const int params_len);
typedef void (__stdcall *flytrap_setparam_func_ptr)(const wchar_t *name, const wchar_t *value);
typedef const wchar_t *(__stdcall *flytrap_getparam_func_ptr)(const wchar_t *name);
typedef void (__stdcall *flytrap_triggerreport_func_ptr)(void);

// These macros make initializing the crash handler easier
#define FLYTRAPVARPOINTERS(BASENAME) \
	HMODULE BASENAME ## LibraryHandle = NULL; \
	extern "C" { \
		flytrap_initclient_func_ptr BASENAME ## InitClient; \
		flytrap_initserver_func_ptr BASENAME ## InitServer; \
		flytrap_enable_func_ptr BASENAME ## Enable; \
		flytrap_disable_func_ptr BASENAME ## Disable; \
		flytrap_isenabled_func_ptr BASENAME ## IsEnabled; \
		flytrap_crashalert_func_ptr BASENAME ## CrashAlert; \
		flytrap_setparam_func_ptr BASENAME ## SetParam; \
		flytrap_getparam_func_ptr BASENAME ## GetParam; \
		flytrap_triggerreport_func_ptr BASENAME ## TriggerReport; \
	} \

#define FLYTRAPLOADLIBRARY(RVAL, BASENAME, DLLPATH) \
	BASENAME ## LibraryHandle = LoadLibrary(DLLPATH); \
	if(BASENAME ## LibraryHandle == NULL) { \
		RVAL = -101; \
	} else { \
		BASENAME ## InitClient = (flytrap_initclient_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlyTrapInitClient"); \
		if(BASENAME ## InitClient == NULL) { \
			RVAL = -102; \
		} \
		BASENAME ## InitServer = (flytrap_initserver_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlyTrapInitServer"); \
		if(BASENAME ## InitServer == NULL) { \
			RVAL = -103; \
		} \
		BASENAME ## Enable = (flytrap_enable_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlyTrapEnable"); \
		if(BASENAME ## Enable == NULL) { \
			RVAL = -104; \
		} \
		BASENAME ## Disable = (flytrap_disable_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlyTrapDisable"); \
		if(BASENAME ## Disable == NULL) { \
			RVAL = -105; \
		} \
		BASENAME ## IsEnabled = (flytrap_isenabled_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlyTrapIsEnabled"); \
		if(BASENAME ## IsEnabled == NULL) { \
			RVAL = -106; \
		} \
		BASENAME ## SetParam = (flytrap_setparam_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlyTrapSetParam"); \
		if(BASENAME ## SetParam == NULL) { \
			RVAL = -107; \
		} \
		BASENAME ## GetParam = (flytrap_getparam_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlyTrapGetParam"); \
		if(BASENAME ## GetParam == NULL) { \
			RVAL = -108; \
		} \
		BASENAME ## TriggerReport = (flytrap_triggerreport_func_ptr)GetProcAddress(BASENAME ## LibraryHandle, "FlyTrapTriggerReport"); \
		if(BASENAME ## TriggerReport == NULL) { \
			RVAL = -109; \
		} \
	}

#define FLYTRAPINITCLIENT(RVAL, BASENAME, DUMPPATH, REPORTURL, OOP_EXE_PATH, DLLPATH) \
	FLYTRAPLOADLIBRARY(RVAL, BASENAME, DLLPATH); \
	if(BASENAME ## LibraryHandle != NULL) { \
		RVAL = BASENAME ## InitClient(DUMPPATH, REPORTURL, OOP_EXE_PATH); \
	}

#endif	// !__FLYTRAP_H_INCLUDED__
