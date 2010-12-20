/*! @file FlyTrap.h FlyTrap API Reference
 * @brief FlyTrap API Reference
 *
 * @author David Wimsey
 * $Revision$
 * $Date$
 *
 * Primary FlyTrap definitions.
 *
 * @section License
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

#ifndef __FLYTRAP_H_INCLUDED__
#define __FLYTRAP_H_INCLUDED__	1

#ifdef __cplusplus
extern "C" {
#endif

#define FLYTRAP_ERROR_SUCCESS		1
#define FLYTRAP_ERROR_NO_CONTEXT	-1
#define FLYTRAP_ERROR_OUTOFMEMORY	-2

#ifndef FLYTRAP_API
	#if defined(_WIN32_WCE)
		#ifdef FLYTRAP_EXPORTS
			// Exports are created by FlyTrap.def
			//#define FLYTRAP_API __declspec(dllexport)
			#define FLYTRAP_API
		#else
			#define FLYTRAP_API __declspec(dllimport)
		#endif
	#else
		#define FLYTRAP_API
	#endif
#endif // !FLYTRAP_API

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
	flytrap_initclient_func_ptr BASENAME ## InitClient; \
	flytrap_initserver_func_ptr BASENAME ## InitServer; \
	flytrap_enable_func_ptr BASENAME ## Enable; \
	flytrap_disable_func_ptr BASENAME ## Disable; \
	flytrap_isenabled_func_ptr BASENAME ## IsEnabled; \
	flytrap_crashalert_func_ptr BASENAME ## CrashAlert; \
	flytrap_setparam_func_ptr BASENAME ## SetParam; \
	flytrap_getparam_func_ptr BASENAME ## GetParam; \
	flytrap_triggerreport_func_ptr BASENAME ## TriggerReport; \

// These macros make initializing the crash handler easier
#define FLYTRAPEXTERNVARPOINTERS(BASENAME) \
	HMODULE BASENAME ## LibraryHandle = NULL; \
	extern flytrap_initclient_func_ptr BASENAME ## InitClient; \
	extern flytrap_initserver_func_ptr BASENAME ## InitServer; \
	extern flytrap_enable_func_ptr BASENAME ## Enable; \
	extern flytrap_disable_func_ptr BASENAME ## Disable; \
	extern flytrap_isenabled_func_ptr BASENAME ## IsEnabled; \
	extern flytrap_crashalert_func_ptr BASENAME ## CrashAlert; \
	extern flytrap_setparam_func_ptr BASENAME ## SetParam; \
	extern flytrap_getparam_func_ptr BASENAME ## GetParam; \
	extern flytrap_triggerreport_func_ptr BASENAME ## TriggerReport; \

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


#ifdef FLYTRAP_IMPLICIT_DECLARATIONS

/*! @brief Initialize the FlyTrap client library and prepare it for use.
 *
 * @param dumpPath Path where dump files should be written.  This path must be writable by the process,
 * however it does not need to be persistent as the data is deleted after the report is sent.
 * @param reportUrl Address to which crash reports will be submitted.  This server should be prepared to
 * handle breakpad style reports.  SSL should be used to ensure sensitive user information does not end
 * up in the wrong hands.
 * @param OOPExePath Path to executable which will be used for OOP reporting.  If NULL, In Process handling
 * will be used.  If 'FlySwatter.dll' is given, the built in out of process server will be used.
 * @returns 1 on success
 */
FLYTRAP_API int __stdcall FlyTrapInitClient(wchar_t *dumpPath, wchar_t *reportUrl, wchar_t *OOPExePath);
/*! @brief Shutdown the FlyTrap client library and release memory associated with it.
 *
 * @param[in] clientContext The context you wish to shut down.
 * @returns @enum FLYTRAP_ERRORCODE
 */
FLYTRAP_API int __stdcall FlyTrapShutdownClient(void *clientContext);
/*! @brief Shutdown a FlyTrap server 
 *
 * Shuts down the server specified in serverContext.  If @ref OOP is enabled
 * the client will disconnect from the server which may result in it
 * shutting down if the client is the only connected client.
 *
 * The exception handler for the associated with this context will be removed if possible otherwise
 * it will be disabled.
 *
 * All memory associated with the context will be released, including any parameters set.
 *
 * @param[in] serverContext The context you wish to shut down.
 * @returns @enum FLYTRAP_ERRORCODE
 */
FLYTRAP_API int __stdcall FlyTrapShutdownServer(void *serverContext);
/*! @brief Initialize a FlyTrap server thread.
 *
 * @param[in] dumpPath Path where temporary dump files can be written when an error report is being generated.  The server process must have permission to create and write to files in this directory.
 * @param[in] reportUrl URL to upload reports to.  @Note This connection should be made using SSL to ensure the privacy of possibly sensitive data contained in error reports.
 * @param[in] pipeName Path or name of the IPC pipe used for communication between the client and server.
 * @param[in] serverReadyEventName Name of IPC event to signal when the server is ready to accept connections.
 */
FLYTRAP_API void * __stdcall FlyTrapInitServer(wchar_t *dumpPath, wchar_t *reportUrl, wchar_t *pipeName, wchar_t *serverReadyEventName);
/*! @brief Enable FlyTrap error reporting
 *
 * Enables FlyTrap processing of exceptions.  If exceptions are currently enabled, this function does nothing.
 * If FlyTrap has been previously enabled, when this function returns FlyTrap will trap unhandled exception
 * using the configuration supplied with the first call to @ref FlyTrapEnable.
 * If this is the first call to @ref FlyTrapEnable for this process, the current FlyTrap configuration is copied
 * to an area of memory which is then write protected to ensure it does not get modified by any invalid 
 * instructions in the application.  If Out of Process exception handling is enabled, FlyTrap will attempt
 * to start the Out of Process server.  If the server can not be started, FlyTrap will fall back to using 
 * in process error reporting.
 *
 * FlyTrap remains enabled until @ref FlyTrapDisable has been called or the application terminates.
 *
 * @returns
 *			1 - if previously enabled
 *			0 - if not previously enabled
 *			-1 - if there is no context for this thread.
 *			-2 - if the exception handler could not be enabled
 */
FLYTRAP_API int __stdcall FlyTrapEnable();
/*! @brief Disable FlyTrap error reporting.
 *
 * Disables FlyTrap processing of exceptions.  If exceptions are not currently enabled, this function does nothing.
 * If FlyTrap is handling exceptions, after this function returns FlyTrap will no longer handle exceptions until
 * @ref FlyTrapEnable has been called.
 *
 * @returns 1 if previously enabled, 0 if not previously enabled. -1 if there is no context for this thread. -2 if the exception handler could not be enabled
 */
FLYTRAP_API int __stdcall FlyTrapDisable();
/*! @brief Determine if FlyTrap error reporting is enabled.
 *
 * Used to determine if exceptions are currently being handled by FlyTrap.
 *
 * @returns 1 if previously enabled, 0 if not previously enabled. -1 if there is no context for this thread. -2 if the exception handler could not be enabled
 */
FLYTRAP_API int __stdcall FlyTrapIsEnabled();
/*! @brief Set a parameter or configuration option.
 *
 * Sets the value of the parameter specified by @ref name.
 * @param[in] name Name of value to set.
 * @param[in] value Value to set.
 */
FLYTRAP_API void __stdcall FlyTrapSetParam(const wchar_t *name, const wchar_t *value);
/*! @brief Get the value of a configured parameter or option.
 *
 * @return A unicode string representing the value stored for the named option.
 */
FLYTRAP_API const wchar_t * __stdcall FlyTrapGetParam(const wchar_t *name);
/*! @brief Manually trigger the error reporting process.
 *
 */
FLYTRAP_API void __stdcall FlyTrapTriggerReport();

#endif // FLYTRAP_IMPLICIT_DECLARATIONS

// Internal shared defines
#define FLYTRAP_REPORTMODE_CRASH					0
#define FLYTRAP_REPORTMODE_ASSERTION				1
#define FLYTRAP_REPORTMODE_USERTRIGGER				2

#define FLYTRAP_PARAM_CRASHREPORT_TITLE					L"CrashReport_Title"
#define FLYTRAP_PARAM_CRASHREPORT_WELCOMEMESSAGE		L"CrashReport_WelcomeMessage"
#define FLYTRAP_PARAM_CRASHREPORT_INFOMESSAGE1			L"CrashReport_Info1Message"
#define FLYTRAP_PARAM_CRASHREPORT_INFOMESSAGE2			L"CrashReport_Info2Message"
#define FLYTRAP_PARAM_CRASHREPORT_INFOMESSAGE3			L"CrashReport_Info3Message"
#define FLYTRAP_PARAM_CRASHREPORT_INFOBUTTON1			L"CrashReport_Info1Button"
#define FLYTRAP_PARAM_CRASHREPORT_INFOBUTTON2			L"CrashReport_Info2Button"
#define FLYTRAP_PARAM_CRASHREPORT_INFOBUTTON3			L"CrashReport_Info3Button"
#define FLYTRAP_PARAM_CRASHREPORT_NOPROMPTCHKBOXLABEL	L"CrashReport_DontAskCheckbox"
#define FLYTRAP_PARAM_CRASHREPORT_SENDBUTTON			L"CrashReport_SendButton"
#define FLYTRAP_PARAM_CRASHREPORT_CANCELBUTTON			L"CrashReport_CancelButton"

#define FLYTRAP_PARAM_DEBUGREPORT_TITLE					L"DebugReport_Title"
#define FLYTRAP_PARAM_DEBUGREPORT_WELCOMEMESSAGE		L"DebugReport_WelcomeMessage"
#define FLYTRAP_PARAM_DEBUGREPORT_INFOMESSAGE1			L"DebugReport_Info1Message"
#define FLYTRAP_PARAM_DEBUGREPORT_INFOMESSAGE2			L"DebugReport_Info2Message"
#define FLYTRAP_PARAM_DEBUGREPORT_INFOMESSAGE3			L"DebugReport_Info3Message"
#define FLYTRAP_PARAM_DEBUGREPORT_INFOBUTTON1			L"DebugReport_Info1Button"
#define FLYTRAP_PARAM_DEBUGREPORT_INFOBUTTON2			L"DebugReport_Info2Button"
#define FLYTRAP_PARAM_DEBUGREPORT_INFOBUTTON3			L"DebugReport_Info3Button"
#define FLYTRAP_PARAM_DEBUGREPORT_SENDBUTTON			L"DebugReport_SendButton"
#define FLYTRAP_PARAM_DEBUGREPORT_CANCELBUTTON			L"DebugReport_CancelButton"

#define FLYTRAP_PARAM_CHECKPOINT_SETTINGS				L"CheckpointSettings"
#define FLYTRAP_PARAM_ATTACHFILES_PARAM					L"AttachedFile"
#define FLYTRAP_PARAM_ATTACHREGKEY_PARAM				L"AttachedRegKey"

#define FLYTRAP_PARAM_FLYTRAPVERSION					L"FlyTrapVersion"
#define FLYTRAP_PARAM_REPORTURL							L"FlyTrapReportURL"
#define FLYTRAP_PARAM_CRASHID							L"FlyTrapCrashId"
#define FLYTRAP_PARAM_FLYTRAPBUILDFLAGS					L"FlyTrapBuildFlags"

#ifdef __cplusplus
}
#endif

#endif	// !__FLYTRAP_H_INCLUDED__
