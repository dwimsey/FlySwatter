// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

static HMODULE g_hModule;

void FlySwatter_UnregisterCrashAlertDialogWindowClass(HINSTANCE hInstance);
void FlySwatter_RegisterCrashAlertDialogWindowClass(HINSTANCE hInstance);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
			g_hModule = hModule;
			FlySwatter_RegisterCrashAlertDialogWindowClass(g_hModule);
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			FlySwatter_UnregisterCrashAlertDialogWindowClass(g_hModule);
			g_hModule = NULL;
			break;
	}
	return TRUE;
}
