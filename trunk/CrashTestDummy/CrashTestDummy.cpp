// CrashTestDummy.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "CrashTestDummy.h"
#include "CrashTestDummyDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "../FlySwatterClient/flyswatterclient.h"


// CCrashTestDummyApp

BEGIN_MESSAGE_MAP(CCrashTestDummyApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CCrashTestDummyApp construction

FLYSWATTERVARPOINTERS(FlySwatter)


void DoManualReport(void *windowObject)
{
	FlySwatterTriggerReport();
}

CCrashTestDummyApp::CCrashTestDummyApp()
{
	int rVal;
	// Use in process exception handling
//	FLYSWATTERINITCLIENT(rVal, FlySwatter, L"%APPDATA%\\CrashTestDummy", L"http://flyswatter.notresponsible.org/report.php", NULL, L"FlySwatter.dll");

	// Use Out of Process exception handling, using the builtin FlySwatter crash server.
	FLYSWATTERINITCLIENT(rVal, FlySwatter, L"%APPDATA%\\CrashTestDummy", L"http://flyswatter.notresponsible.org/report.php", L"FlySwatter.dll", L"FlySwatter.dll");

	// @TODO Add this to real documentation system so it can be published somewhere useful
	// These are used in various other predefined template strings and recommended for all crash reports
	FlySwatterSetParam(L"AppName", L"CrashTest Dummy");
	FlySwatterSetParam(L"CompanyShortName", L"RTS");
	FlySwatterSetParam(L"CompanyName", L"Research Triangle Software");
	FlySwatterSetParam(L"CompanyLegalName", L"Research Triangle Software, Inc.");
	FlySwatterSetParam(L"AppVersion", L"0.1.2.3");

	// This defines what checkpoint file we should use and how many we're expected to be allowed per day, if unset, no checkpoint file will be used and the library will not attempt to limit the number of reports sent
	// FlySwatterSetParam(L"FlySwatter_CheckpointSettings", L"%APPDATA%\\CrashTestDummy\\fscreport.cpt;5;");

	// This is a semicolon/colon seperated of paths (appropriate for OS) of additional files
	// to attach to the report
	FlySwatterSetParam(L"FlySwatter_AttachFiles", L"%APPDATA%\\CrashTestDummy\\Debug.log;C:\\autoexec.bat;c:\\boot.ini");

	// (MS Windows Only) This is a semicolon seperated list of registry keys to dump and include in the crash report
	FlySwatterSetParam(L"FlySwatter_AttachRegKeys", L"HKEY_LOCAL_MACHINE\\Software\\CrashTestDummy;HKEY_CURRENT_USER\\Software\\CrashTestDummy;HKEY_CLASSES_ROOT\\CrashTestDummy");

	// Any variables prefixed with FlySwatter_CrashAlertDialog_ are not sent with the crash report,
	// they are just used to configure the alert dialog
	//

	// This sets the dialog caption text for the crash alert dialog
//	FlySwatterSetParam(L"FlySwatter_CrashAlertDialog_Title", L"{AppName} has crashed!");
	// Text to display in the initial display area when the dialog is first displayed
//	FlySwatterSetParam(L"FlySwatter_CrashAlertDialog_Info1Message", L"Something has caused {AppName} to crash and a crash report has been generated.\r\n\r\nThe crash report may contain confidential information from the program at the time it crashed.\r\n\r\nClick the 'Send' button to send this crash information to {CompanyShortName}.");
	// This is no string for the more info message, its generated internally
	// Text to display in the privacy notice area when required
	FlySwatterSetParam(L"FlySwatter_CrashAlertDialog_Info3Message", L"Reporting this crash will send information about what the program was doing when it crashed to {CompanyLegalName}  The information included may include sections or all of the applications memory while running, information about running processes on your computer, various registry keys related to the way your system and this software is configured, log files relating to this application.  Any of these sources of information may contain confidential information and should only be sent if you are certain it contains only information you trust sending over the Internet.");
	// This sets the text on the button which sends the report to the server
//	FlySwatterSetParam(L"FlySwatter_CrashAlertDialog_SendButton", L"Send");
	// This sets the text on the button that does not send anything to the server and destroys the report
//	FlySwatterSetParam(L"FlySwatter_CrashAlertDialog_CancelButton", L"Cancel");
	// This is the text on the 1st tab or button depending on which style is used
//	FlySwatterSetParam(L"FlySwatter_CrashAlertDialog_Info1Button", L"Welcome");
	// This is the text on the 2nd tab or button depending on which style is used
//	FlySwatterSetParam(L"FlySwatter_CrashAlertDialog_Info2Button", L"More Info");
	// This is the text on the 3rd tab or button depending on which style is used
//	FlySwatterSetParam(L"FlySwatter_CrashAlertDialog_Info3Button", L"Privacy Information");

	// This one we always set as it hides the button if its not set
	FlySwatterSetParam(L"FlySwatter_CrashAlertDialog_DontAskCheckbox", L"Don't ask me again!");

	//	FlySwatterSetParam(L"FlySwatter_CrashAlertDialog_Info1Button", L"Don't ask me again!");

	FlySwatterEnable();
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
	SetRegistryKey(_T("FlySwatterCrashTestDummy"));

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
