// CrashTestDummy.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "CrashTestDummy.h"
#include "CrashTestDummyDlg.h"
#include "../FlyTrap/FlyTrapVersion.h"
#include "fsctd_version.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "../FlyTrap/FlyTrap.h"


// CCrashTestDummyApp

BEGIN_MESSAGE_MAP(CCrashTestDummyApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CCrashTestDummyApp construction

FLYTRAPVARPOINTERS(FlyTrap)


void DoManualReport(void *windowObject)
{
	FlyTrapTriggerReport();
}

#define REPORT_URL	L"http://10.27.1.242/~dwimsey/flyswatter/sendreport.php"
//#define REPORT_URL	L"http://192.168.128.239/~dwimsey/flyswatter/sendreport.php"
CCrashTestDummyApp::CCrashTestDummyApp()
{
	int rVal;

	LPWSTR *szArglist;
	int nArgs;
	int inProcServer = 0;
	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if( NULL != szArglist ) {
		for(int i=0; i<nArgs; i++) {
			if(_wcsicmp(L"-InProcServer", szArglist[i]) == 0) {
				inProcServer = 1;
			} else if((_wcsicmp(L"-h", szArglist[i]) == 0) || (_wcsicmp(L"--h", szArglist[i]) == 0) || (wcscmp(L"-?", szArglist[i]) == 0) || (wcscmp(L"--?", szArglist[i]) == 0) || (wcscmp(L"/?", szArglist[i]) == 0) || (_wcsicmp(L"/h", szArglist[i]) == 0) || (_wcsicmp(L"/help", szArglist[i]) == 0) || (wcscmp(L"/?", szArglist[i]) == 0)) {
				MessageBoxW(NULL, 
					L"-InProcServer    - Start this application without using rundll32.exe and flytrap.dll to run an external crash handler application.\r\n"
					L"-h               - Show this help."
					, L"Help", MB_OK);
				exit(0);
			}
		}
	}
	// Free memory allocated for CommandLineToArgvW arguments.
	LocalFree(szArglist);
	if(inProcServer == 1) {
		// Use in process exception handling
		FLYTRAPINITCLIENT(rVal, FlyTrap, L"%APPDATA%\\CrashTestDummy", REPORT_URL, NULL, L"FlyTrap.dll");
	} else {
		// Use Out of Process exception handling, using the builtin FlyTrap crash server.
		FLYTRAPINITCLIENT(rVal, FlyTrap, L"%APPDATA%\\CrashTestDummy", REPORT_URL, L"FlyTrap.dll", L"FlyTrap.dll");
	}
	// @TODO Add this to real documentation system so it can be published somewhere useful
	// These are used in various other predefined template strings and recommended for all crash reports
	FlyTrapSetParam(L"CompanyShortName", L"RTS");
	FlyTrapSetParam(L"CompanyName", L"Research Triangle Software");
	FlyTrapSetParam(L"CompanyLegalName", L"Research Triangle Software, Inc.");
	FlyTrapSetParam(L"AppName", L"CrashTest Dummy");
	FlyTrapSetParam(L"AppGuid", L"EE6EADBF-79A6-4e24-B5D0-6D14945DDA17");
	FlyTrapSetParam(L"AppVersion", _T(FSCTD_STRFILEVERSION));
	wchar_t buf[30];
	wsprintf((LPWSTR)&buf, L"%d", FSCTD_VERSION_BUILD);
	FlyTrapSetParam(L"AppBuildId", (LPWSTR)&buf);
	FlyTrapSetParam(L"ProductName", L"FlySwatter");
	FlyTrapSetParam(L"ProductGuid", L"4225E489-0DF5-4673-B854-65D55997EDBC");
	
	// This defines what checkpoint file we should use and how many we're expected to be allowed per day, if unset, no checkpoint file will be used and the library will not attempt to limit the number of reports sent
	// FlyTrapSetParam(L"FlyTrap_CheckpointSettings", L"%APPDATA%\\CrashTestDummy\\fscreport.cpt;5;");

	// This is a semicolon/colon seperated of paths (appropriate for OS) of additional files
	// to attach to the report
	FlyTrapSetParam(FLYTRAP_PARAM_ATTACHFILES_PARAM L"s", L"..\\scripts\\buildver.py;C:\\autoexec.bat;c:\\boot.ini");

	// (MS Windows Only) This is a semicolon seperated list of registry keys to dump and include in the crash report
	FlyTrapSetParam(FLYTRAP_PARAM_ATTACHREGKEY_PARAM L"s", L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run;HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run;HKEY_CURRENT_USER\\Software\\CrashTestDummy;HKEY_CLASSES_ROOT\\CrashTestDummy");

	// Any variables prefixed with FlyTrap_CrashAlertDialog_ are not sent with the crash report,
	// they are just used to configure the alert dialog
	//

	// This sets the dialog caption text for the crash/debug report dialog
	FlyTrapSetParam(FLYTRAP_PARAM_CRASHREPORT_TITLE, L"Error Report - {AppName}");
	FlyTrapSetParam(FLYTRAP_PARAM_DEBUGREPORT_TITLE, L"Debugging Report - {AppName}");

	// Text to display in the initial display area when the dialog is first displayed
//	FlyTrapSetParam(L"FlyTrap_CrashAlertDialog_Info1Message", L"Something has caused {AppName} to crash and a crash report has been generated.\r\n\r\nThe crash report may contain confidential information from the program at the time it crashed.\r\n\r\nClick the 'Send' button to send this crash information to {CompanyShortName}.");
	// This is no string for the more info message, its generated internally
	// Text to display in the privacy notice area when required
	FlyTrapSetParam(FLYTRAP_PARAM_CRASHREPORT_INFOBUTTON3, L"Reporting error.");
	FlyTrapSetParam(FLYTRAP_PARAM_DEBUGREPORT_INFOBUTTON3, L"Reporting debug.");
	
	FlyTrapSetParam(FLYTRAP_PARAM_CRASHREPORT_WELCOMEMESSAGE, L"Reporting error.");
	FlyTrapSetParam(FLYTRAP_PARAM_DEBUGREPORT_WELCOMEMESSAGE, L"Reporting debug.");
	// This sets the text on the button which sends the report to the server
//	FlyTrapSetParam(L"FlyTrap_CrashAlertDialog_SendButton", L"Send");
	// This sets the text on the button that does not send anything to the server and destroys the report
//	FlyTrapSetParam(L"FlyTrap_CrashAlertDialog_CancelButton", L"Cancel");
	// This is the text on the 1st tab or button depending on which style is used
//	FlyTrapSetParam(L"FlyTrap_CrashAlertDialog_Info1Button", L"Welcome");
	// This is the text on the 2nd tab or button depending on which style is used
//	FlyTrapSetParam(L"FlyTrap_CrashAlertDialog_Info2Button", L"More Info");
	// This is the text on the 3rd tab or button depending on which style is used
//	FlyTrapSetParam(L"FlyTrap_CrashAlertDialog_Info3Button", L"Privacy Information");

	// This one we always set as it hides the button if its not set
	FlyTrapSetParam(FLYTRAP_PARAM_CRASHREPORT_NOPROMPTCHKBOXLABEL, L"Don't ask me again!");

	//	FlyTrapSetParam(L"FlyTrap_CrashAlertDialog_Info1Button", L"Don't ask me again!");

	FlyTrapEnable();
}


// The one and only CCrashTestDummyApp object

CCrashTestDummyApp theApp;


// CCrashTestDummyApp initialization

BOOL CCrashTestDummyApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("FlyTrapCrashTestDummy"));

	CCrashTestDummyDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
