/*! @internal
 * @file stdafx.h
 * @brief Include file for standard system include files.
 *
 * @author David Wimsey
 * $Revision$
 * $Date$
 *
 * stdafx.h : include file for standard system include files,
 * or project specific include files that are used frequently, but
 * are changed infrequently
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

#pragma once

#ifndef _CRT_SECURE_NO_WARNINGS
// we don't want this warnings, they really provide little usefulness and we're aware of how to code properly for them
#define _CRT_SECURE_NO_WARNINGS 1
#endif

#ifndef _CRT_NONSTDC_NO_DEPRECATE
// we don't care about these warnings either, we prefer compatibility with other compilers and OSes
#define _CRT_NONSTDC_NO_DEPRECATE 1
#endif

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>


#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <atlbase.h>
#include <atlstr.h>

#include "client/windows/handler/exception_handler.h"
#include "client/windows/sender/crash_report_sender.h"
#include "client/windows/crash_generation/client_info.h"

using namespace google_breakpad;

#define FLYTRAP_IMPLICIT_DECLARATIONS 1
#include "FlyTrap.h"

extern "C" {


	// This are used internally and need to be available to all bits
	int FlyTrapCrashAlert(void *parentWindowHandle, const wchar_t *reportUrl, const int report_mode, const wchar_t *dumpId, const wchar_t *miniDumpFilename, const LPFLYTRAPPARAM params, const int params_len);
	bool FlyTrapExceptionFilter(void *ctx, EXCEPTION_POINTERS *exceptionInfo, MDRawAssertionInfo *assertionInfo);
	void FlyTrapOutOfProcessDumpCallback(void *dump_context, ClientInfo *client_info, const std::wstring *dump_path);
	bool FlyTrapInProcessDumpCallback(const wchar_t* dumpPath, const wchar_t* dumpId, void* ctx, EXCEPTION_POINTERS* exceptionInfo, MDRawAssertionInfo* assertionInfo, bool dumpSucceeded);

	size_t base64blockencode(unsigned char *inBuf, unsigned char *outBuf, size_t byteCount);
	unsigned char *base64encode(unsigned char *inBuf, size_t inSize, int lineSize);
	char *b64append(const char *inbuf, const char* format);

	void DeleteParamMap(map<wstring, wstring> *oldMap);
	map<wstring, wstring> *CreateParamMap(const LPFLYTRAPPARAM params, const int params_len, const wchar_t *dumpId, const wchar_t *reportUrl);
	wchar_t *ExpandEnvVarsInStr(const wchar_t *inStrPtr);
	wchar_t *mprintf(const wchar_t *format, ...);
}
