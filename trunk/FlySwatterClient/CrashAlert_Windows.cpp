#include "stdafx.h"
#define USE_TAB_CONTROL_FOR_REPORTLAYOUT 1
static HINSTANCE fsgh_Instance = NULL;

void FlySwatter_UnregisterCrashAlertDialogWindowClass(HINSTANCE hInstance)
{
	fsgh_Instance = NULL;
}

void FlySwatter_RegisterCrashAlertDialogWindowClass(HINSTANCE hInstance)
{
	fsgh_Instance = hInstance;
}

typedef struct __FlySwatterCrashAlertDialogInitDataStructure {
	wchar_t *dumpFileName;
	wchar_t *dumpDir;
	wchar_t *dumpId;
	wchar_t *reportUrl;
	LPFLYSWATTERPARAM params;
	int params_len;
	int useTabControl;
} FLYSWATTERCRASHALERTDIALOGINITDATA, *LPFLYSWATTERCRASHALERTDIALOGINITDATA;

const wchar_t *FlySwatterGetParamEx(LPFLYSWATTERPARAM params, int params_len, const wchar_t *name);

#include <stdarg.h>

wchar_t *mprintf(const wchar_t *format, ...)
{
	va_list varlist;
	va_start(varlist, format);
	size_t outLen = _vscwprintf(format, varlist);
	wchar_t *outBuf = (wchar_t*)calloc(outLen+1, sizeof(wchar_t));
	vswprintf(outBuf, outLen, format, varlist);
	va_end(varlist);
	return(outBuf);
}

#define BUFFER_BLOCK_SIZE	512

// This macro works inside the formatStr function
// It ensures the buffer is large enough to hold the addition number of characters specified
// by the argument, including space for the null terminator
#define ENSURE_BUFFER_SIZE(ADDITIONAL_CHARS_REQUIRED) \
	nts = ((o + ((ADDITIONAL_CHARS_REQUIRED) + 1)) - outSize); \
	if(nts > 0) { \
	outSize = ((((nts/BUFFER_BLOCK_SIZE) + 1) * BUFFER_BLOCK_SIZE) + outSize); \
	outBuf = (wchar_t*)realloc(outBuf, outSize * sizeof(wchar_t)); \
	}

// this function is very slow as it handles each character one a time, there are probably
// far far faster methods of doing this, but its used so rarely that speed shouldn't be an
// issue
wchar_t *formatStr(wchar_t *format, LPFLYSWATTERPARAM params, int params_len)
{
	if(format == NULL) {
		return(NULL);
	}
	size_t inLen = wcslen(format);
	size_t outSize = inLen + 1;
	wchar_t *outBuf = (wchar_t*)calloc(outSize, sizeof(wchar_t));
	wchar_t *varName = NULL;
	wchar_t *varValue = NULL;
	size_t nts = 0;
	size_t o = 0;
	size_t t = 0;
	size_t i = 0;

	for(i = 0; i < inLen; i++) {
		if(format[i] != L'{') {
			// normal character, process it and move on.
			ENSURE_BUFFER_SIZE(1);
			outBuf[o++] = format[i];
			if(format[i] == L'\0') {
				break;
			} else {
				continue;
			}
		}

		for(t = (i + 1); t < (inLen+1); t++) {
			switch(format[t]) {
				case L'}':
					if((i+1) == t) {
						// this means we need to just print one { and continue on, so increment i as well
						ENSURE_BUFFER_SIZE(1);
						outBuf[o++] = format[i];
					} else {
						// this is an end indicator, lets see what we need to do
						varName = (wchar_t*)calloc(((t - i) + 1), sizeof(wchar_t));
						wcsncpy(varName, (wchar_t*)&format[(i + 1)], (t - i));
						varName[((t-i)-1)] = L'\0';

						varValue = (wchar_t*)FlySwatterGetParamEx(params, params_len, varName);
						free(varName);
						if(varValue != NULL) {
							ENSURE_BUFFER_SIZE(wcslen(varValue));
							outBuf[o] = L'\0';
							wcscat(&outBuf[o], varValue);
							o += wcslen(varValue);
							i = t; // this will be our closing }, but it will get updated when the loop continues to point to the next one
						} else {
							// the variable name isn't know, just pass it through without replacing it so it might
							// be recognized in the output to help debugging the problem
							ENSURE_BUFFER_SIZE(1);
							outBuf[o++] = format[i];
						}
					}
					// this will break us out of this for loop
					t = inLen;
					break;
				case L'{':
					if((i+1) == t) {
						// this means we need to just print one { and continue on, so increment i as well
						ENSURE_BUFFER_SIZE(1);
						outBuf[o++] = format[i++];
					} else {
						// This means our previous { probably wasn't closed, lets ignore pass 
						// is straigh through and hopefully that will be useful for debugging 
						// the deeper problem when its noticed
						ENSURE_BUFFER_SIZE(1);
						outBuf[o++] = format[i];						
					}
					// this will break us out of this for loop
					t = inLen;
					break;
				case L'\0':
					// this 'variable isn't properly closed, just pass the remainder of the
					// string through and return
					ENSURE_BUFFER_SIZE(wcslen((const wchar_t*)&format[i]));
					outBuf[o] = L'\0';
					wcsncat(outBuf, (const wchar_t*)&format[i], outSize);
					outBuf[outSize-1] = L'\0';
					return(outBuf);
					break;
				default:
					if(iswspace(format[t]) != 0) {
						// this wasn't a valid variable, spaces aren't allowed
						// we'll just copy this one character manually then jump back to the normal loop;
						ENSURE_BUFFER_SIZE(1);
						outBuf[o++] = format[i];
						// this will break us out of this for loop
						t = inLen;
					}
					break;
			}
		}
		continue;
	}
	// copies the trailing NULL regardless of how the loop ended
	outBuf[o] = L'\0';
	return(outBuf);
}

LPDLGTEMPLATE CreateFSWin32CrashAlertDlgTemplate()
{
	LPDLGTEMPLATE dlgTemplate;
	//	LPDLGITEMTEMPLATE dlgItem;
	char *dlgPtr;

	wchar_t caption[] = L"Crash Detected!";
	wchar_t font[] = L"MS Shell Dlg";

	size_t dlgStringsSize;
	size_t dlgItemSize;
	size_t dlgTemplateSize;

	size_t dlgSize = 0;

	dlgStringsSize = (wcslen(caption)+1+wcslen(font)+1)*(sizeof(wchar_t)*3);
	//	dlgItemSize = ((sizeof(DLGITEMTEMPLATE)+(sizeof(DWORD)*2))*10);
	dlgItemSize = 0;
	dlgTemplateSize = (sizeof(DLGTEMPLATE)+(sizeof(DWORD)*2)+dlgItemSize+dlgStringsSize)*2;

	if(dlgTemplateSize%sizeof(DWORD)!=0) {
		dlgTemplateSize += sizeof(DWORD)-(dlgTemplateSize%sizeof(DWORD));
	}

	dlgTemplate = (LPDLGTEMPLATE)malloc(dlgTemplateSize);
	if(dlgTemplate == NULL) {
		return(NULL);
	}
	memset(dlgTemplate, 0, dlgTemplateSize);

	dlgTemplate->style = DS_SETFONT | DS_MODALFRAME | DS_3DLOOK | DS_FIXEDSYS | WS_POPUP | WS_CAPTION | WS_SYSMENU;
	dlgTemplate->dwExtendedStyle = 0;
	dlgTemplate->x = 0;
	dlgTemplate->y = 0;
	dlgTemplate->cx = 316;
	dlgTemplate->cy = 183;
	dlgTemplate->cdit = 0;
	dlgPtr = (char*)dlgTemplate;
	dlgPtr += sizeof(DLGTEMPLATE)+(sizeof(WORD)*2);



	memcpy(dlgPtr, caption, (wcslen(caption)+1)*sizeof(wchar_t));	// copy the title into the template
	dlgPtr += (wcslen(caption)+1)*sizeof(wchar_t);					// skip over the caption name
	dlgPtr[0] = 0x07;												// Set the font size high byte
	dlgPtr[1] = 0x00;												// Set the font size low byte
	dlgPtr += sizeof(WORD);											// skip over font point size;
	memcpy(dlgPtr, font, (wcslen(font)+1)*sizeof(wchar_t));			// Set the font name
	dlgPtr += (wcslen(font)+1)*sizeof(wchar_t);						// skip over the font face name

	// we don't do anything else with this dialog template, we'll add the controls
	// via code rather than resource


	return(dlgTemplate);
}

#define BASE_CONTROL_ID	100

#define IDC_BTN_SEND IDOK
#define IDC_BTN_CANCEL IDCANCEL
#define IDC_BTN_GENERAL (BASE_CONTROL_ID + 1)
#define IDC_BTN_MOREINFO (BASE_CONTROL_ID + 2)
#define IDC_BTN_PRIVACY (BASE_CONTROL_ID + 3)
#define IDC_STATIC_MAIN (BASE_CONTROL_ID + 4)
#define IDC_CHK_DONTASK (BASE_CONTROL_ID + 5)
#define IDC_TAB_MAIN (BASE_CONTROL_ID + 6)

BOOL CenterWindow(HWND hwnd)
{
	HWND hwndParent;
	RECT rect, rectP;
	int width, height;      
	int screenwidth, screenheight;
	int x, y;

	//make the window relative to its parent
	hwndParent = GetParent(hwnd);
	GetWindowRect(hwnd, &rect);
	if(hwndParent == NULL) {
		hwndParent = GetDesktopWindow();
	}
	GetWindowRect(hwndParent, &rectP);

	width  = rect.right  - rect.left;
	height = rect.bottom - rect.top;

	x = ((rectP.right-rectP.left) -  width) / 2 + rectP.left;
	y = ((rectP.bottom-rectP.top) - height) / 2 + rectP.top;

	screenwidth  = GetSystemMetrics(SM_CXSCREEN);
	screenheight = GetSystemMetrics(SM_CYSCREEN);

	//make sure that the dialog box never moves outside of
	//the screen
	if(x < 0) {
		x = 0;
	}
	if(y < 0) {
		y = 0;
	}

	if(x + width  > screenwidth) {
		x = screenwidth  - width;
	}
	if(y + height > screenheight) {
		y = screenheight - height;
	}

	MoveWindow(hwnd, x, y, width, height, FALSE);

	return(TRUE);
}

#include <commctrl.h>

typedef struct __FSCADHandles {
	HWND hWnd;
	HWND btnOk;
	HWND btnCancel;
	HWND chkDontAsk;
	HWND staticMain;
	HWND btnGeneral;
	HWND btnMoreInfo;
	HWND btnPrivacy;
	HWND tabMain;
	LPFLYSWATTERPARAM params;
	int params_len;
	int useTabControl;
} FSCADHANDLE, *LPFSCADHANDLE;

#define FSWS_ISVISIBLE(VAL) ((dHandle->useTabControl==VAL) ? WS_VISIBLE : 0)


void SetActiveTab(LPFSCADHANDLE dHandle, int tabPage, BOOL activateTab)
{
	wchar_t *tmpPtr;
	wchar_t *fTmpPtr;

	switch(tabPage) {
		case 0:
			tmpPtr = (wchar_t*)FlySwatterGetParamEx(dHandle->params, dHandle->params_len, L"FlySwatter_CrashAlertDialog_Info1Message");
			if(tmpPtr == NULL) {
				tmpPtr = L"Something has caused {AppName} to crash and a crash report has been generated.\r\n\r\nThe crash report may contain confidential information from the program at the time it crashed.\r\n\r\nClick the 'Send' button to send this crash information to {CompanyShortName}.";
			}
			break;
		case 1:
			tmpPtr = (wchar_t*)FlySwatterGetParamEx(dHandle->params, dHandle->params_len, L"FlySwatter_CrashAlertDialog_Info2Message");
			if(tmpPtr == NULL) {
				tmpPtr = L"No additional information at this time.";
			}
			break;
		case 2:
			tmpPtr = (wchar_t*)FlySwatterGetParamEx(dHandle->params, dHandle->params_len, L"FlySwatter_CrashAlertDialog_Info3Message");
			if(tmpPtr == NULL) {
				tmpPtr = L"Reporting this crash will send information about what the program was doing when it crashed to {CompanyLegalName}\r\n\r\nThe information may include sections or all of the applications memory, information about running programs on your computer, various registry keys related to the way your system and this software is configured, log files relating to this application.  Any of these sources of information may contain confidential information and should only be sent if you are certain it contains only information you are willing to share over the Internet.";
			}
			break;
		default:
			return;
			break;
	}
	if(activateTab == TRUE) {
		TabCtrl_SetCurSel(dHandle->tabMain, tabPage);
	}
	fTmpPtr = formatStr(tmpPtr, dHandle->params, dHandle->params_len);
	SetWindowTextW(dHandle->staticMain, fTmpPtr);
	free(fTmpPtr);
}

void FlySwatterLayoutCrashAlertDialog(LPFSCADHANDLE dHandle, BOOL rePaint)
{
	RECT btnRect;		// standard button size, btnRect.left and btnRect.top are consider the border width and standard spacers
	RECT chkRect;		// chkRect.bottom is the reference point for how tall a checkbox is by default
	RECT borderRect;	// absolute coordinates for the valid area inside the board to place controls, can save some time
	RECT cRect;			// absolute size of the client area of the rectangle
	RECT txtRect;		// holds the static text area based on use of tab control or not

	RECT btnCancel;
	RECT btnOk;
	RECT chkDontAsk;
	
	RECT btnGeneralRect;
	RECT btnMoreInfoRect;
	RECT btnPrivacyRect;

	GetClientRect(dHandle->hWnd, &cRect);

	btnRect.left = 7;
	btnRect.top = 7;
	btnRect.right = 50;
	btnRect.bottom = 14;
	MapDialogRect(dHandle->hWnd, &btnRect);

	chkRect.left = 7;
	chkRect.top = 4;		// this is our difference between a checkbox size and the 'standard alignment' size
	chkRect.right = 191;
	chkRect.bottom = 10;
	MapDialogRect(dHandle->hWnd, &chkRect);

	txtRect.left = 46;	// this gives us room for a 64 pixel image on normal monitors
	txtRect.top = 7;
	txtRect.right = 50;
	txtRect.bottom = 14;
	MapDialogRect(dHandle->hWnd, &txtRect);

	// define our usable bounds
	borderRect.left = btnRect.left;
	borderRect.top = btnRect.top;
	borderRect.right = cRect.right - btnRect.left;
	borderRect.bottom = cRect.bottom - btnRect.top;

	// Setup the Cancel button, its the bottom right most object, which everything else is based off of
	btnCancel.right = btnRect.right;											// standard width
	btnCancel.bottom = btnRect.bottom;											// standard height
	btnCancel.left = borderRect.right - btnCancel.right;						// just left of the right border
	btnCancel.top = borderRect.bottom - btnCancel.bottom;						// just above the bottom border

	// put the Ok/Send button just left of the Cancel
	btnOk.right = btnRect.right;												// standard width
	btnOk.bottom = btnRect.bottom;												// standard height
	btnOk.left = btnCancel.left - (btnOk.right + btnRect.left);					// this button is just to the left of the last
	btnOk.top = btnCancel.top;													// these buttons are on the same line

	// setup the DontAsk checkbox
	chkDontAsk.bottom = chkRect.bottom;											// standard height
	chkDontAsk.right = btnOk.left - (btnRect.left + borderRect.left);			// We stretch up to within a border's width of the left side of the Ok/Send button
	chkDontAsk.left = borderRect.left;											// left side at the border
	chkDontAsk.top = btnOk.top + ((btnRect.bottom - chkDontAsk.bottom) / 2);	// this centers the checkbox based on the buttons

	// useTabControl get set to one if the tab control was created and it is visible, otherwise we use the old/ugly layout rules
	if(dHandle->tabMain == NULL) {
		// fix the tab control setting to off
		dHandle->useTabControl = 0;
	}

	int hasPrivacyInfo = (FlySwatterGetParamEx(dHandle->params, dHandle->params_len, L"FlySwatter_CrashAlertDialog_Info3Message") == NULL ? 0 : 1);

	if(hasPrivacyInfo == 1) {
		// put the privacy info on the far right
		btnPrivacyRect.right = btnRect.right;
		btnPrivacyRect.bottom = btnRect.bottom;
		btnPrivacyRect.left = borderRect.right - btnPrivacyRect.right;
		btnPrivacyRect.top = borderRect.top; // this positions with room for an extra row of buttons between the send/cancel button and the 'tab buttons' for the info pages

		// put the more info button just left of the privacy button
		btnMoreInfoRect.right = btnRect.right;
		btnMoreInfoRect.bottom = btnRect.bottom;
		btnMoreInfoRect.left = btnPrivacyRect.left - (btnMoreInfoRect.right + btnRect.left);
		btnMoreInfoRect.top = btnPrivacyRect.top;
	} else {
		// hide the privacy button
		btnPrivacyRect.right = 0;
		btnPrivacyRect.bottom = 0;
		btnPrivacyRect.left = cRect.right;
		btnPrivacyRect.top = cRect.bottom;

		// no privacy info, so put the MoreInfo button on the far right
		btnMoreInfoRect.right = btnRect.right;
		btnMoreInfoRect.bottom = btnRect.bottom;
		btnMoreInfoRect.left = borderRect.right - btnMoreInfoRect.right;
		btnMoreInfoRect.top = borderRect.top;
	}

	btnGeneralRect.right = btnRect.right;
	btnGeneralRect.bottom = btnRect.bottom;
	btnGeneralRect.left = btnMoreInfoRect.left - (btnGeneralRect.right + btnRect.left);
	btnGeneralRect.top = btnMoreInfoRect.top;

	// TODO: make staticImage 64x64 pixels for now, we'll make it something dyname later
	RECT staticImageRect;
	RECT tabMainRect;
	RECT staticMainRect;
	staticImageRect.left = borderRect.left;
	staticImageRect.top = borderRect.top;
	staticImageRect.right = 64;
	staticImageRect.bottom = 64;
	if(dHandle->useTabControl == 1) {
		tabMainRect.left = staticImageRect.left + staticImageRect.right + btnRect.left;
		tabMainRect.top = borderRect.top;
		tabMainRect.right = borderRect.right - tabMainRect.left;
		tabMainRect.bottom = (btnOk.top - btnRect.top) - tabMainRect.top;

		staticMainRect.left = 0;
		staticMainRect.top = 0;
		staticMainRect.right = 0;
		staticMainRect.bottom = 0;
		RECT r2;
		GetClientRect(dHandle->tabMain, &staticMainRect);
		memcpy(&r2, &staticMainRect, sizeof(RECT));
		TabCtrl_AdjustRect(dHandle->tabMain, FALSE, &staticMainRect);
		r2.bottom = (r2.bottom - staticMainRect.bottom);
		r2.right = (r2.right - staticMainRect.right);
		staticMainRect.bottom -= staticMainRect.top;
		staticMainRect.left += tabMainRect.left;
		staticMainRect.right = (borderRect.right - (staticMainRect.left + r2.right));
		staticMainRect.top += tabMainRect.top;
	} else {
		// hide the tab control off screen
		tabMainRect.left = cRect.right;
		tabMainRect.top = cRect.bottom;
		tabMainRect.right = 0;
		tabMainRect.bottom = 0;

		staticMainRect.left = staticImageRect.left + staticImageRect.right + btnRect.left;
		staticMainRect.top = btnGeneralRect.top + btnGeneralRect.bottom + (btnRect.top/2);
		staticMainRect.right = borderRect.right - staticMainRect.left;
		staticMainRect.bottom = btnOk.top - ((btnRect.top/2) + staticMainRect.top);
	}

	// select which windows to actually make visible
	if(dHandle->useTabControl == 1) {
		ShowWindow(dHandle->btnGeneral, SW_HIDE);
		ShowWindow(dHandle->btnMoreInfo, SW_HIDE);
		ShowWindow(dHandle->btnPrivacy, SW_HIDE);
		ShowWindow(dHandle->tabMain, SW_SHOW);
	} else {
		ShowWindow(dHandle->tabMain, SW_HIDE);
		ShowWindow(dHandle->btnGeneral, SW_SHOW);
		ShowWindow(dHandle->btnMoreInfo, SW_SHOW);
		if(hasPrivacyInfo==0) {
			ShowWindow(dHandle->btnPrivacy, SW_HIDE);
		} else {
			ShowWindow(dHandle->btnPrivacy, SW_SHOW);
		}
	}

	MoveWindow(dHandle->btnCancel, btnCancel.left, btnCancel.top, btnCancel.right, btnCancel.bottom, rePaint);
	MoveWindow(dHandle->btnOk, btnOk.left, btnOk.top, btnOk.right, btnOk.bottom, rePaint);
	MoveWindow(dHandle->chkDontAsk, chkDontAsk.left, chkDontAsk.top, chkDontAsk.right, chkDontAsk.bottom, rePaint);
	MoveWindow(dHandle->btnMoreInfo, btnMoreInfoRect.left, btnMoreInfoRect.top, btnMoreInfoRect.right, btnMoreInfoRect.bottom, rePaint);
	MoveWindow(dHandle->btnPrivacy, btnPrivacyRect.left, btnPrivacyRect.top, btnPrivacyRect.right, btnPrivacyRect.bottom, rePaint);
	MoveWindow(dHandle->btnGeneral, btnGeneralRect.left, btnGeneralRect.top, btnGeneralRect.right, btnGeneralRect.bottom, rePaint);
	MoveWindow(dHandle->tabMain, tabMainRect.left, tabMainRect.top, tabMainRect.right, tabMainRect.bottom, rePaint);
	MoveWindow(dHandle->staticMain, staticMainRect.left, staticMainRect.top, staticMainRect.right, staticMainRect.bottom, rePaint);

	// Set the first tab page as active, its the welcome page
	SetActiveTab(dHandle, 0, TRUE);
}

void InitFlySwatterCrashDialog(HWND hWnd, LPFLYSWATTERCRASHALERTDIALOGINITDATA idPtr)
{
	LPFSCADHANDLE dHandle;
	wchar_t *tmpPtr = NULL;
	wchar_t *fTmpPtr = NULL;
	RECT cRect;
	RECT btnRect;

/*
	INITCOMMONCONTROLSEX ccInfo;
	ccInfo.dwSize = sizeof(INITCOMMONCONTROLSEX);
	ccInfo.dwICC = ICC_WIN95_CLASSES | ICC_TAB_CLASSES;

	InitCommonControlsEx(&ccInfo);
*/

	dHandle = (LPFSCADHANDLE)calloc(1, sizeof(FSCADHANDLE));
	memset(dHandle, 0, sizeof(FSCADHANDLE));
	dHandle->hWnd = hWnd;
	dHandle->params = idPtr->params;
	dHandle->params_len = idPtr->params_len;
	dHandle->useTabControl = idPtr->useTabControl;
	SetWindowLongPtr(dHandle->hWnd, DWLP_USER, (LONG_PTR)dHandle);
	CenterWindow(hWnd);

	btnRect.left = 7;
	btnRect.top = 7;
	btnRect.right = 50;
	btnRect.bottom = 14;
	MapDialogRect(hWnd, &btnRect);
	RECT chkRect;
	chkRect.left = 7;
	chkRect.top = 4;		// this is our difference between a checkbox size and the 'standard alignment' size
	chkRect.right = 191;
	chkRect.bottom = 10;
	MapDialogRect(hWnd, &chkRect);
	RECT txtRect;
	txtRect.left = 46;	// this gives us room for a 64 pixel image on normal monitors
	txtRect.top = 7;
	txtRect.right = 50;
	txtRect.bottom = 14;
	MapDialogRect(hWnd, &txtRect);
	GetClientRect(hWnd, &cRect);

	TCITEM tie;
	dHandle->tabMain = CreateWindowEx(0, WC_TABCONTROL, L"", WS_CHILD | WS_CLIPSIBLINGS | FSWS_ISVISIBLE(1), txtRect.left, btnRect.top, cRect.right - (txtRect.left + btnRect.left), (cRect.bottom - ((btnRect.top * 3) + btnRect.bottom)), hWnd, NULL, fsgh_Instance, NULL);
	if(dHandle->tabMain != NULL) {
		SetWindowLongPtr(dHandle->tabMain, GWLP_ID, IDC_TAB_MAIN);

		// Add tabs
		tie.mask = TCIF_TEXT | TCIF_IMAGE;
		tie.iImage = -1;
		tie.pszText = (wchar_t*)FlySwatterGetParamEx(idPtr->params, idPtr->params_len, L"FlySwatter_CrashAlertDialog_Info1Button");
		if(tie.pszText == NULL) {
			tie.pszText = L"General";
		}
		if(TabCtrl_InsertItem(dHandle->tabMain, 0, &tie) == -1) {
			// TODO: Add error handling if the tab can't be added
		}

		tie.pszText = (wchar_t*)FlySwatterGetParamEx(idPtr->params, idPtr->params_len, L"FlySwatter_CrashAlertDialog_Info2Button");
		if(tie.pszText == NULL) {
			tie.pszText = L"Crash Report";
		}
		if(TabCtrl_InsertItem(dHandle->tabMain, 1, &tie) == -1) {
			// TODO: Add error handling if the tab can't be added
		}

		// We only add the privacy tab if we have a privacy statement
		if(FlySwatterGetParamEx(idPtr->params, idPtr->params_len, L"FlySwatter_CrashAlertDialog_Info3Message") != NULL) {
			tie.pszText = (wchar_t*)FlySwatterGetParamEx(idPtr->params, idPtr->params_len, L"FlySwatter_CrashAlertDialog_Info3Button");
			if(tie.pszText == NULL) {
				tie.pszText = L"Privacy Information";
			}
			if(TabCtrl_InsertItem(dHandle->tabMain, 2, &tie) == -1) {
				// TODO: Add error handling if the tab can't be added
			}
		}
	} else {
		// tab control couldn't be created so mark the ugly interface as the visible one
		dHandle->useTabControl = 0;
	}

	dHandle->btnOk = CreateWindowEx(0, L"BUTTON", L"Send", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_PUSHBUTTON | BS_VCENTER | BS_CENTER | BS_TEXT, (cRect.right - ((btnRect.right + btnRect.left) * 2)), (cRect.bottom - (btnRect.bottom + btnRect.top)), btnRect.right, btnRect.bottom, hWnd, NULL, fsgh_Instance, NULL);
	if(dHandle->btnOk != NULL) {
		SetWindowLongPtrW(dHandle->btnOk, GWLP_ID, IDC_BTN_SEND);
		tmpPtr = (wchar_t*)FlySwatterGetParamEx(idPtr->params, idPtr->params_len, L"FlySwatter_CrashAlertDialog_SendButton");
		if(tmpPtr != NULL) {
			fTmpPtr = formatStr(tmpPtr, idPtr->params, idPtr->params_len);
			SetWindowTextW(dHandle->btnOk, fTmpPtr);
			free(fTmpPtr);
		}
	}

	dHandle->btnCancel = CreateWindowEx(0, L"BUTTON", L"Cancel", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_VCENTER | BS_CENTER | BS_TEXT, (cRect.right - (btnRect.right + btnRect.left)), (cRect.bottom - (btnRect.bottom + btnRect.top)), btnRect.right, btnRect.bottom, hWnd, NULL, fsgh_Instance, NULL);
	if(dHandle->btnCancel != NULL) {
		SetWindowLongPtrW(dHandle->btnCancel, GWLP_ID, IDC_BTN_CANCEL);
		tmpPtr = (wchar_t*)FlySwatterGetParamEx(idPtr->params, idPtr->params_len, L"FlySwatter_CrashAlertDialog_CancelButton");
		if(tmpPtr != NULL) {
			fTmpPtr = formatStr(tmpPtr, idPtr->params, idPtr->params_len);
			SetWindowTextW(dHandle->btnCancel, fTmpPtr);
			free(fTmpPtr);
		}
	}

	// Ugly style button interface controls start here.

	// Create the more info button, but not visible
	dHandle->btnGeneral = CreateWindowEx(0, L"BUTTON", L"Welcome", WS_CHILD | FSWS_ISVISIBLE(0) | BS_PUSHBUTTON | BS_VCENTER | BS_CENTER | BS_TEXT, txtRect.left, (cRect.bottom - ((btnRect.top * 2) + (btnRect.bottom * 3))), btnRect.right, btnRect.bottom, hWnd, NULL, fsgh_Instance, NULL);
	if(dHandle->btnGeneral != NULL) {
		SetWindowLongPtrW(dHandle->btnGeneral, GWLP_ID, IDC_BTN_GENERAL);
		tmpPtr = (wchar_t*)FlySwatterGetParamEx(idPtr->params, idPtr->params_len, L"FlySwatter_CrashAlertDialog_Info1Button");
		if(tmpPtr != NULL) {
			fTmpPtr = formatStr(tmpPtr, idPtr->params, idPtr->params_len);
			SetWindowTextW(dHandle->btnGeneral, fTmpPtr);
			free(fTmpPtr);
		}
	}

	// Create the more info button, but not visible
	dHandle->btnMoreInfo = CreateWindowEx(0, L"BUTTON", L"More Info", WS_CHILD | FSWS_ISVISIBLE(0) | BS_PUSHBUTTON | BS_VCENTER | BS_CENTER | BS_TEXT, txtRect.left, (cRect.bottom - ((btnRect.top * 2) + (btnRect.bottom * 3))), btnRect.right, btnRect.bottom, hWnd, NULL, fsgh_Instance, NULL);
	if(dHandle->btnMoreInfo != NULL) {
		SetWindowLongPtrW(dHandle->btnMoreInfo, GWLP_ID, IDC_BTN_MOREINFO);
		tmpPtr = (wchar_t*)FlySwatterGetParamEx(idPtr->params, idPtr->params_len, L"FlySwatter_CrashAlertDialog_Info2Button");
		if(tmpPtr != NULL) {
			fTmpPtr = formatStr(tmpPtr, idPtr->params, idPtr->params_len);
			SetWindowTextW(dHandle->btnMoreInfo, fTmpPtr);
			free(fTmpPtr);
		}
	}

	// Create the more info button, but not visible
	dHandle->btnPrivacy = CreateWindowEx(0, L"BUTTON", L"Privacy Info", WS_CHILD | FSWS_ISVISIBLE(0) | BS_PUSHBUTTON | BS_VCENTER | BS_CENTER | BS_TEXT, txtRect.left, (cRect.bottom - ((btnRect.top * 2) + (btnRect.bottom * 3))), btnRect.right, btnRect.bottom, hWnd, NULL, fsgh_Instance, NULL);
	if(dHandle->btnPrivacy != NULL) {
		SetWindowLongPtrW(dHandle->btnPrivacy, GWLP_ID, IDC_BTN_PRIVACY);
		tmpPtr = (wchar_t*)FlySwatterGetParamEx(idPtr->params, idPtr->params_len, L"FlySwatter_CrashAlertDialog_Info3Button");
		if(tmpPtr != NULL) {
			fTmpPtr = formatStr(tmpPtr, idPtr->params, idPtr->params_len);
			SetWindowTextW(dHandle->btnPrivacy, fTmpPtr);
			free(fTmpPtr);
		}
		if(FlySwatterGetParamEx(idPtr->params, idPtr->params_len, L"FlySwatter_CrashAlertDialog_Info3Message") == NULL) {
			ShowWindow(dHandle->btnPrivacy, SW_HIDE);
		}
	}

	// Ugly style button interface controls ends here.

	// Create the don't ask me again, but not visible
	dHandle->chkDontAsk = CreateWindowEx(0, L"BUTTON", L"Don't ask me again!", WS_CHILD | BS_AUTOCHECKBOX | BS_VCENTER | BS_LEFT, btnRect.left, (cRect.bottom - (btnRect.top + btnRect.bottom - (chkRect.top/2))), ((cRect.right - btnRect.left) - (((btnRect.left + btnRect.right) * 2) + btnRect.left)), chkRect.bottom, hWnd, NULL, fsgh_Instance, NULL);
	if(dHandle->chkDontAsk != NULL) {
		SetWindowLongPtrW(dHandle->chkDontAsk, GWLP_ID, IDC_CHK_DONTASK);
		tmpPtr = (wchar_t*)FlySwatterGetParamEx(idPtr->params, idPtr->params_len, L"FlySwatter_CrashAlertDialog_DontAskCheckbox");
		if(tmpPtr != NULL) {
			ShowWindow(dHandle->chkDontAsk, SW_SHOW);
			fTmpPtr = formatStr(tmpPtr, idPtr->params, idPtr->params_len);
			SetWindowTextW(dHandle->chkDontAsk, fTmpPtr);
			free(fTmpPtr);
		} else {
			ShowWindow(dHandle->chkDontAsk, SW_HIDE);
		}
	}

	// Set the welcome message text
	//L"Something has caused {AppName} to crash and a crash report has been generated.\r\n\r\nThe crash report may contain confidential information from the program at the time it crashed.\r\n\r\nClick the 'Send' button to send this crash information to {CompanyShortName}."
	dHandle->staticMain = CreateWindowEx(0, L"STATIC", L"Welcome Message", WS_VISIBLE | WS_CHILD | SS_LEFT /*| SS_BLACKRECT */| SS_NOPREFIX, txtRect.left, btnRect.top, (cRect.right - (txtRect.left + btnRect.left)), (cRect.bottom - ((btnRect.top * 4) + (btnRect.bottom * 3))), hWnd, NULL, fsgh_Instance, NULL);
	if(dHandle->staticMain != NULL) {
		SetWindowLongPtrW(dHandle->staticMain, GWLP_ID, IDC_STATIC_MAIN);
		tmpPtr = (wchar_t*)FlySwatterGetParamEx(idPtr->params, idPtr->params_len, L"FlySwatter_CrashAlertDialog_WelcomeMessage");
		if(tmpPtr == NULL) {
			tmpPtr = L"Something has caused {AppName} to crash and a crash report has been generated.\r\n\r\nThe crash report may contain confidential information from the program at the time it crashed.\r\n\r\nClick the 'Send' button to send this crash information to {CompanyShortName}.";
		}
		fTmpPtr = formatStr(tmpPtr, idPtr->params, idPtr->params_len);
		SetWindowTextW(dHandle->staticMain, fTmpPtr);
		free(fTmpPtr);
	}

	// Set the window caption text
	tmpPtr = (wchar_t*)FlySwatterGetParamEx(idPtr->params, idPtr->params_len, L"FlySwatter_CrashAlertDialog_Title");
	if(tmpPtr == NULL) {
		tmpPtr = L"Crash detected!";
	}
	fTmpPtr = formatStr(tmpPtr, idPtr->params, idPtr->params_len);
	SetWindowTextW(hWnd, fTmpPtr);
	free(fTmpPtr);

	FlySwatterLayoutCrashAlertDialog(dHandle, FALSE);
}

INT_PTR CALLBACK FlySwatterCrashAlertDialogWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	LPFSCADHANDLE dHandle = (LPFSCADHANDLE)GetWindowLongPtr(hDlg, DWLP_USER);
	switch(message) {
		case WM_INITDIALOG:
			InitFlySwatterCrashDialog(hDlg, (LPFLYSWATTERCRASHALERTDIALOGINITDATA)lParam);
			return((INT_PTR)TRUE);

		case WM_COMMAND:
			if(HIWORD(wParam) == BN_CLICKED) {
				switch(LOWORD(wParam)) {
					case IDOK:
					case IDCANCEL:
						EndDialog(hDlg, LOWORD(wParam));
						return((INT_PTR)TRUE);
						break;
					case IDC_BTN_GENERAL:
						SetActiveTab(dHandle, 0, TRUE);
						break;
					case IDC_BTN_MOREINFO:
						SetActiveTab(dHandle, 1, TRUE);
						break;
					case IDC_BTN_PRIVACY:
						SetActiveTab(dHandle, 2, TRUE);
						break;
				}
			}
			break;
		case WM_NOTIFY:
			if(dHandle != NULL) {
				switch (((LPNMHDR)lParam)->code) {
					case TCN_SELCHANGE:
						if((dHandle->tabMain != NULL) && (((LPNMHDR)lParam)->idFrom == IDC_TAB_MAIN)) {
							// this message is for the main tab control and we have a valid window handle for it
							SetActiveTab(dHandle, TabCtrl_GetCurSel(dHandle->tabMain), FALSE);
						}
						break;
				}
			}
			break;
		case BN_CLICKED:
			switch(((LPNMHDR)lParam)->idFrom) {
				case IDC_BTN_GENERAL:
					SetActiveTab(dHandle, 0, TRUE);
					break;
				case IDC_BTN_MOREINFO:
					SetActiveTab(dHandle, 1, TRUE);
					break;
				case IDC_BTN_PRIVACY:
					SetActiveTab(dHandle, 2, TRUE);
					break;
			}
			break;
	}
	return((INT_PTR)FALSE);
}

wchar_t *ExpandEnvVarsInStr(const wchar_t *inStrPtr)
{
	DWORD sn = ExpandEnvironmentStringsW(inStrPtr, NULL, 0);
	wchar_t *outStrPtr = (wchar_t*)calloc(sn + 2, sizeof(wchar_t));
	ExpandEnvironmentStringsW(inStrPtr, outStrPtr, sn + 1);
	// ensure the string is always safely NULL terminated
	outStrPtr[sn+1] = L'\0';
	return(outStrPtr);
}
int FlySwatterCrashAlert(const wchar_t *reportUrl, const wchar_t *miniDumpFilename, const LPFLYSWATTERPARAM params, const int params_len)
{
	MessageBox(NULL, L"Attach!", L"Attach!", MB_OK);
	wchar_t *dumpPath = NULL;
	wchar_t *dumpId = NULL;
	wchar_t *pathStr = wcsdup(miniDumpFilename);
	wchar_t *pathStrSPtr = wcsrchr(pathStr, L'\\');
	wchar_t *pathStrPtr = wcsrchr(pathStr, L'/');
	if(pathStrSPtr > pathStrPtr) {
		pathStrPtr = pathStrSPtr;
	}

	if(pathStrPtr != NULL) {
		pathStrPtr[0] = L'\0';
		if(pathStrPtr[1] != L'\0') {
			pathStrPtr++;
		} else {
			pathStrPtr = NULL;
		}
	}

	if(pathStrPtr != NULL) {
		dumpPath = wcsdup(pathStr);
		wchar_t *extPtr = wcsrchr(pathStrPtr, L'.');
		if(extPtr != NULL) {
			extPtr[0] = L'\0';
		}
		dumpId = wcsdup(pathStrPtr);
	}
	free(pathStr);

	// display the dump info to the user and allow them to determine if they want to send a crash report
	FLYSWATTERCRASHALERTDIALOGINITDATA idData;
	idData.dumpFileName = (wchar_t*)miniDumpFilename;
	idData.reportUrl = (wchar_t*)reportUrl;
	idData.params = params;
	idData.params_len = params_len;
	idData.dumpId = dumpId;
	idData.dumpDir = dumpPath;
	idData.useTabControl = USE_TAB_CONTROL_FOR_REPORTLAYOUT;
	LPDLGTEMPLATE dlgTemplate = CreateFSWin32CrashAlertDlgTemplate();
	int dialogResult = DialogBoxIndirectParamW(fsgh_Instance, dlgTemplate, NULL, FlySwatterCrashAlertDialogWndProc, (LPARAM)&idData);
	free(dlgTemplate);
	wchar_t buff[256];
	switch(dialogResult) {
		case 1: // Send the report
			break;
		case 2: // don't send a report
			return(0);
		default:
			// anything else is an error, we're going to send the report anyway
			wsprintf(buff, L"This application has crashed, however the error reporting dialog could not be displayed or failed(%u).  Would you like to send a crash report anyway?", dialogResult);
			if(MessageBoxW(NULL, buff, L"Application crash detected!", MB_OKCANCEL) != IDOK) {
				if(dumpId != NULL)
					free(dumpId);
				if(dumpPath != NULL)
					free(dumpPath);
				return(-5);
			}
			break;
	}

	// We've got a crash dump and we're enabled
	// Compile the data to be sent

	// Sends the specified minidump file, along with the map of
	// name value pairs, as a multipart POST request to the given URL.
	// Parameter names must contain only printable ASCII characters,
	// and may not contain a quote (") character.
	// Only HTTP(S) URLs are currently supported.  The return value indicates
	// the result of the operation (see above for possible results).
	// If report_code is non-NULL and the report is sent successfully (that is,
	// the return value is RESULT_SUCCEEDED), a code uniquely identifying the
	// report will be returned in report_code.
	// (Otherwise, report_code will be unchanged.)

	// Prepare the report extra parameters list.
	map<wstring, wstring> paramsStr;
	paramsStr[L"FlySwatterVersion"] = _T(FLYSWATTER_VERSION_STRING);
	paramsStr[L"FlySwatterCrashId"] = dumpId;

	for(int i = 0; i < params_len; i++) {
		if(params[i].name == NULL) {
			// blank entry, just skip it.  We could probably break out of the loop but we won't do that since
			// we may have a way to delete entries in the future
			continue;
		}
		if(wcsncmp(params[i].name, L"FlySwatter_CrashAlertDialog_", wcslen(L"FlySwatter_CrashAlertDialog_")) == 0) {
			// Parameters that start with FlySwatter_CrashAlertDialog_ are for use by the dialog display only
			paramsStr[params[i].name] = params[i].value;
		}
	}
	/// send the report
	ReportResult rs;	
	wstring reportCode;

	// determine if we need to use a checkpoint file and do so
	wchar_t *tmpPtr = (wchar_t*)FlySwatterGetParamEx(params, params_len, L"FlySwatter_CheckpointSettings");
	if(tmpPtr == NULL) {
		tmpPtr = wcsdup(L"");
	} else {
		tmpPtr = ExpandEnvVarsInStr(tmpPtr);
	}
	wchar_t *offset = wcschr(tmpPtr, L';');
	wchar_t *next_offset = NULL;
	if(offset != NULL) {
		offset[0] = L'\0';
		offset++;
		next_offset = wcschr(offset, L';');
		if(next_offset != NULL) {
			next_offset[0] = L'\0';
			next_offset++;
		}
	}

	CrashReportSender cs(tmpPtr);

	if(offset != NULL) {
		cs.set_max_reports_per_day(_wtoi(offset));
		offset = next_offset;
		next_offset = NULL;
	}
	rs = cs.SendCrashReport(reportUrl, paramsStr, miniDumpFilename, &reportCode);
	free(tmpPtr);
	int returnVal = 0;
	if(dialogResult<1 || dialogResult>2) {
		returnVal = -10;
	}
	wchar_t buf[1025];
	switch(rs) {
		case RESULT_FAILED:		// Failed to communicate with the server; try later.
			_sntprintf(buf, 1024, L"Upload failed: %s", reportCode.c_str());
			returnVal += -1;
			break;
		case RESULT_REJECTED:	// Successfully sent the crash report, but the server rejected it; don't resend this report.
			_sntprintf(buf, 1024, L"Upload completed, but rejected by server: (No Reason): %s", reportCode.c_str());
			returnVal += -2;
			break;
		case RESULT_SUCCEEDED:	// The server accepted the crash report.
			_sntprintf(buf, 1024, L"Upload successful: %s", reportCode.c_str());
			returnVal += 1;
			break;
		case RESULT_THROTTLED:	// No attempt was made to send the crash report, because we exceeded the maximum reports per day.
			_sntprintf(buf, 1024, L"Upload throttled: %s", reportCode.c_str());
			returnVal += -3;
			break;
		default:
			_sntprintf(buf, 1024, L"Report Result: %u: %s", rs, reportCode.c_str());
			returnVal += -4;
	}

	if(dumpId != NULL)
		free(dumpId);
	if(dumpPath != NULL)
		free(dumpPath);

	return(returnVal);
}