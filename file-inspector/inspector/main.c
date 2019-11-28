///////////////////////////////////////////////////////////////////
//****************************************************************
//main.c - реализация GUI
//****************************************************************
//////////////////////////////////////////////////////////////////

#include <stdio.h>
#include "hashfunc.h"
#include "filehash.h"
#include <string.h>
#include <tchar.h>
#include <Windows.h>
#include "hashtable.h"
#include "dirview.h"
#include <Windowsx.h>
#include <CommCtrl.h>

#define WND_HEIGHT 300
#define WND_WIDTH 350
#define WND_CH_HEIGHT 150
#define WND_RS_HEIGHT 470
#define WND_RS_WIDTH 600
#define ScreenX GetSystemMetrics(SM_CXSCREEN)
#define ScreenY GetSystemMetrics(SM_CYSCREEN)

#define BUFF_LEN 26*4

#define MENU_EXIT 31 
#define MENU_SCAN 27
#define MENU_IMPRINT 28
#define MENU_CHOOSEDRIVE 29
#define MENU_ABOUT 30
#define IMPRWND_CANCEL 26
#define IMPRWND_START 25
#define MENU_RESULTS 24
#define LIBO_CHNGD 23

#define UNUCODE
#define _UNICODE

#ifdef UNICODE
#define SetWindowText SetWindowTextW
#else
#define SetWindowText SetWindowTextA
#endif // !UNICODE

HWND liboNew, liboChanged, liboDeleted;
HMENU hMenu;
HWND cbDrivesImpr, cbDrivesScan;
HWND hPlain, hScanProgressBar, hImprintProgressBar;
HINSTANCE My_hInst;
HWND MainWnd, ImprintWnd, ScanWnd, ResultsWnd;

HANDLE hImprintThread, hScanThread;

LRESULT CALLBACK wndScanProcedure(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK wndImprintProcedure(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK wndResultsProcedure(HWND, UINT, WPARAM, LPARAM);
void GetImprint(HWND);
void GetScan(HWND);
void SetResultsControls(HWND);
void SetImprControls(HWND);
void SetScanControls(HWND);
void SetControls(HWND);
void SetImprintWnd(void);
void SetScanWnd(void);
void SetResultsWnd(void);
void InitMsg(HWND);

//Точка входа программы
int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, 
	LPSTR args, int ncmdshow)
{
	//Initialize threads handle
	hScanThread = NULL;
	hImprintThread = NULL;

	My_hInst = hInst;
	WNDCLASSW wc = { 0 };
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hInstance = My_hInst;
	wc.lpszClassName = L"myWindowClass";
	wc.lpfnWndProc = WindowProcedure;

	if (!RegisterClassW(&wc))
		return -1;


	InitMsg(MainWnd);

	MainWnd = CreateWindowW(wc.lpszClassName, L"Inspector",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE, (ScreenX - WND_WIDTH) / 2, 
		(ScreenY - WND_HEIGHT) / 2, WND_WIDTH, WND_HEIGHT, NULL, NULL, My_hInst, NULL);

	SetImprintWnd();
	SetScanWnd();
	SetResultsWnd();

	MSG msg = {0};

	while (GetMessageW(&msg, 0, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

//Сообщение при старте программы
void InitMsg(HWND hWnd)
{
	//СДЕЛАТЬ ДИНАМИЧЕСКИМ
	WCHAR Buff[BUFF_LEN];

	memset(Buff, 0, BUFF_LEN);

	GetLogicalDriveStringsW(BUFF_LEN, Buff);

	WCHAR *ptr;
	WCHAR drive_str[10] = L"";
	WCHAR str_to_outp[100] = L"Drives defined: ";
	ptr = Buff;

	SendMessageW(cbDrivesScan, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)L"All");
	while (*ptr) {
		wcscat_s(drive_str, 10, ptr);
		wcscat_s(str_to_outp, 100, ptr);
		ptr += wcslen(ptr) + 1;
		if (*ptr) {			
			wcscat_s(str_to_outp, 100, L", ");
		}
		wcscpy_s(drive_str, 10, L"");		
	}

	MessageBoxW(hWnd, str_to_outp, L"Drives", MB_OK);
	return;
}

//Создание элементов окна
void SetControls(HWND hWnd)
{
	//Buttons
	CreateWindowW(L"Button", L"Imprint", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 200, 30, 100,
		30, hWnd, (HMENU)MENU_IMPRINT, NULL, NULL);

	CreateWindowW(L"Button", L"Scan", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 200, 70, 100,
		30, hWnd, (HMENU)MENU_SCAN, NULL, NULL);

	CreateWindowW(L"Static", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 0, 0, 150, 
		WND_HEIGHT, hWnd, NULL, NULL, NULL);


	CreateWindowW(L"Button", L"Show Results", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 200, 170, 100,
		30, hWnd, (HMENU)MENU_RESULTS, NULL, NULL);
	
	//Drives:
	CreateWindowW(L"Static", L"Drives:", WS_CHILD | WS_VISIBLE, 25, 10, 60,
		20, hWnd, NULL, NULL, NULL);	

	//СДЕЛАТЬ ДИНАМИЧЕСКИМ
	WCHAR Buff[BUFF_LEN];

	memset(Buff, 0, BUFF_LEN);

	GetLogicalDriveStringsW(BUFF_LEN, Buff);

	WCHAR *ptr;
	WCHAR drive_str[10] = L"- ";
	int y = 40;

	ptr = Buff;

	while (*ptr && y + 30 < WND_HEIGHT) {
		wcscat_s(drive_str, 10, ptr);
		CreateWindowW(L"Static", drive_str, WS_CHILD | WS_VISIBLE, 50, y, 60,
			20, hWnd, NULL, NULL, NULL);

		wcscpy_s(drive_str, 10, L"- ");
		ptr += wcslen(ptr) + 1;
		y += 30;
	}

}

//Главное меню программы
void MainMenu(HWND hWnd)
{
	hMenu = CreateMenu();
	HMENU hFileMenu = CreateMenu();
	HMENU hAboutMenu = CreateMenu();
	//Menu File:	
	AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"File");
	AppendMenuW(hFileMenu, MF_POPUP, MENU_IMPRINT, L"Imprint...");
	AppendMenuW(hFileMenu, MF_STRING, MENU_SCAN, L"Scan...");
	AppendMenuW(hFileMenu, MF_SEPARATOR, (UINT_PTR)NULL, NULL);
	AppendMenuW(hFileMenu, MF_STRING, MENU_EXIT, L"Exit");

	// Menu About:
	AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hAboutMenu, L"About");
	AppendMenuW(hAboutMenu, MF_STRING, MENU_ABOUT, L"Inspector");

	SetMenu(hWnd, hMenu);
}

//Создание элементов на окне Results
void SetResultsControls(HWND hWnd)
{
	//Labels

	CreateWindowW(L"Static", L"New:", WS_CHILD | WS_VISIBLE | WS_BORDER, 5, 5, WND_RS_WIDTH - 26,
		137, hWnd, NULL, NULL, NULL);

	CreateWindowW(L"Static", L"Changed:", WS_CHILD | WS_VISIBLE | WS_BORDER, 5, 145, WND_RS_WIDTH - 26,
		137, hWnd, NULL, NULL, NULL);

	CreateWindowW(L"Static", L"Deleted:", WS_CHILD | WS_VISIBLE | WS_BORDER, 5, 285, WND_RS_WIDTH - 26,
		137, hWnd, NULL, NULL, NULL);
	
	// Adding a ListBoxes.
	liboNew = CreateWindowExW(WS_EX_CLIENTEDGE, 
		L"LISTBOX", 
		NULL , 
		WS_CHILD | WS_VISIBLE | ES_AUTOVSCROLL | WS_VSCROLL | WS_HSCROLL,
		7, 
		25, 
		WND_RS_WIDTH - 30, 
		120, 
		hWnd, 
		NULL, 
		My_hInst, 
		NULL);

	liboChanged = CreateWindowExW(WS_EX_CLIENTEDGE,
		L"LISTBOX",
		NULL,
		WS_CHILD | WS_VISIBLE | ES_AUTOVSCROLL | WS_VSCROLL | WS_HSCROLL,
		7,
		165,
		WND_RS_WIDTH - 30,
		120,
		hWnd,
		LIBO_CHNGD,
		My_hInst,
		NULL);

	liboDeleted = CreateWindowExW(WS_EX_CLIENTEDGE,
		L"LISTBOX",
		NULL,
		WS_CHILD | WS_VISIBLE | ES_AUTOVSCROLL | WS_VSCROLL | WS_HSCROLL,
		7,
		305,
		WND_RS_WIDTH - 30,
		120,
		hWnd,
		NULL,
		My_hInst,
		NULL);
	//libo - ListBox :D
	SendMessageW(liboDeleted, LB_SETHORIZONTALEXTENT, (WPARAM)PATHMAXSIZE, (LPARAM)0);
	SendMessageW(liboNew, LB_SETHORIZONTALEXTENT, (WPARAM)PATHMAXSIZE, (LPARAM)0);
	SendMessageW(liboChanged, LB_SETHORIZONTALEXTENT, (WPARAM)PATHMAXSIZE, (LPARAM)0);
}

//Создание элементов окна Imprint
void SetImprControls(HWND hWnd)
{
	CreateWindowW(L"Static", L"Drives:", WS_CHILD | WS_VISIBLE, 25, 10, 60,
		20, hWnd, NULL, NULL, NULL);

	CreateWindowW(L"Button", L"Start", WS_CHILD | WS_VISIBLE, 135, 45, 80,
		25, hWnd, (HMENU)IMPRWND_START, NULL, NULL);

	CreateWindowW(L"Button", L"Cancel", WS_CHILD | WS_VISIBLE, 235, 45, 80,
		25, hWnd, (HMENU)IMPRWND_CANCEL, NULL, NULL);

	cbDrivesImpr = CreateWindowW(L"Combobox", L"", 
		CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, 25, 46, 100,
		300, hWnd, NULL, NULL, NULL);

	int cyVScroll = GetSystemMetrics(SM_CYVSCROLL);
	hImprintProgressBar = CreateWindowEx(0, PROGRESS_CLASS, (LPTSTR) NULL, 
                            WS_CHILD, 25, 
                            80, 
                            290, cyVScroll, 
                            hWnd, (HMENU) 0, NULL, NULL);

	//СДЕЛАТЬ ДИНАМИЧЕСКИМ
	WCHAR Buff[BUFF_LEN];

	memset(Buff, 0, BUFF_LEN);

	GetLogicalDriveStringsW(BUFF_LEN, Buff);

	WCHAR *ptr;
	WCHAR drive_str[10] = L"";

	ptr = Buff;

	SendMessageW(cbDrivesImpr, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)L"All");
	while (*ptr) {
		wcscat_s(drive_str, 10, ptr);

		SendMessageW(cbDrivesImpr, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)drive_str);
		wcscpy_s(drive_str, 10, L"");
		ptr += wcslen(ptr) + 1;
	}

	//free(Buff);
	//All - default choice
	SendMessageW(cbDrivesImpr, (UINT)CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
}

//Создание элементов окна Scan
void SetScanControls(HWND hWnd)
{
	CreateWindowW(L"Static", L"Drives:", WS_CHILD | WS_VISIBLE, 25, 10, 60,
		20, hWnd, NULL, NULL, NULL);


	CreateWindowW(L"Button", L"Start", WS_CHILD | WS_VISIBLE, 135, 45, 80,
		25, hWnd, (HMENU)IMPRWND_START, NULL, NULL);

	CreateWindowW(L"Button", L"Cancel", WS_CHILD | WS_VISIBLE, 235, 45, 80,
		25, hWnd, (HMENU)IMPRWND_CANCEL, NULL, NULL);

	cbDrivesScan = CreateWindowW(L"Combobox", L"",
		CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, 25, 46, 100,
		300, hWnd, NULL, NULL, NULL);

	//Progress bar (hided), showed after pressing scan btn
	int cyVScroll = GetSystemMetrics(SM_CYVSCROLL);
	hScanProgressBar = CreateWindowEx(0, PROGRESS_CLASS, (LPTSTR) NULL, 
                            WS_CHILD, 25, 
                            80, 
                            290, cyVScroll, 
                            hWnd, (HMENU) 0, NULL, NULL);

	//СДЕЛАТЬ ДИНАМИЧЕСКИМ
	WCHAR Buff[BUFF_LEN];

	memset(Buff, 0, BUFF_LEN);

	GetLogicalDriveStringsW(BUFF_LEN, Buff);

	WCHAR *ptr;
	WCHAR drive_str[10] = L"";

	ptr = Buff;

	SendMessageW(cbDrivesScan, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)L"All");
	while (*ptr) {
		wcscat_s(drive_str, 10, ptr);

		SendMessageW(cbDrivesScan, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)drive_str);
		wcscpy_s(drive_str, 10, L"");
		ptr += wcslen(ptr) + 1;
	}

	//free(Buff);
	//All - default choice
	SendMessageW(cbDrivesScan, (UINT)CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
}

//Обработка сообщений окна Scan
LRESULT CALLBACK wndScanProcedure(HWND hWnd, UINT msg,
	WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_COMMAND:
		switch (wp) {
		case IMPRWND_CANCEL:
			if (TerminateScanOrImprint()) {				
				MessageBoxW(NULL, L"Operation was canselled", L"Canelled", MB_OK);
			}
			ShowWindow(hWnd, SW_HIDE);
			ShowWindow(hScanProgressBar, SW_HIDE);
			ShowWindow(MainWnd, SW_RESTORE);
			break;
		case IMPRWND_START:
			if ((hScanThread = CreateThread(NULL, 0, GetScan, (void*)cbDrivesScan, 0, NULL)) == NULL) {
				fprintf(stderr, "GetScan CreateThread %d\n", GetLastError());
				return -1;
			}
			break;
		}
		break;
	case WM_CREATE:
		SetScanControls(hWnd);
		break;
	case WM_CLOSE:
		ShowWindow(hWnd, SW_HIDE);
		ShowWindow(MainWnd, SW_RESTORE);
		break;
	case WM_DESTROY:
		break;
	default:
		return DefWindowProcW(hWnd, msg, wp, lp);
	}
	return 0;
}

int TerminateScanOrImprint() {
	int result = 0;
	if (hImprintThread)
	{
		if (TerminateThread(hImprintThread, (DWORD)-1) == NULL) {
			fprintf(stderr, " TerminateThread CreateThread %d\n", GetLastError());
			return -1;
		}
		result = (int)hImprintThread;
		hImprintThread = NULL;
		return result;
	}

	if (hScanThread)
	{
		if (TerminateThread(hScanThread, (DWORD)-1) == NULL) {
			fprintf(stderr, "TerminateThread hScanThread CreateThread %d\n", GetLastError());
			return -1;
		}
		result = (int)hScanThread;
		hScanThread = NULL;
		return result;
	}
	return result;
}

//Обработка сообщений окна Imprint
LRESULT CALLBACK wndImprintProcedure(HWND hWnd, UINT msg,
	WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_COMMAND:
		switch (wp) {
		case IMPRWND_CANCEL:
			if (TerminateScanOrImprint()) {				
				MessageBoxW(NULL, L"Operation was canselled", L"Canelled", MB_OK);
			}
			ShowWindow(hWnd, SW_HIDE);
			ShowWindow(hImprintProgressBar, SW_HIDE);
			ShowWindow(MainWnd, SW_RESTORE);
			break;
		case IMPRWND_START:
			if ((hImprintThread = CreateThread(NULL, 0, GetImprint, (void*)cbDrivesImpr, 0, NULL)) == NULL) {
				fprintf(stderr, "GetImprint CreateThread %d\n", GetLastError());
				return -1;
			}
			//GetImprint(cbDrivesImpr);
			break;
		}
		break;
	case WM_CREATE:
		SetImprControls(hWnd);
		break;
	case WM_CLOSE:
		ShowWindow(hWnd, SW_HIDE);
		ShowWindow(MainWnd, SW_RESTORE);
		break;
	case WM_DESTROY:
		break;
	default:
		return DefWindowProcW(hWnd, msg, wp, lp);
	}
	return 0;
}

//Обработка сообщений окна Results
LRESULT CALLBACK wndResultsProcedure(HWND hWnd, UINT msg,
	WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_COMMAND:
		switch (wp) {
		case LBN_SETFOCUS:
			SendMessageW(liboChanged, (UINT)CB_SETCURSEL, (WPARAM)-1, (LPARAM)0);
			break;
		}
		break;
	case WM_CREATE:
		SetResultsControls(hWnd);
		break;
	case WM_CLOSE:
		ShowWindow(hWnd, SW_HIDE);
		ShowWindow(MainWnd, SW_RESTORE);
		break;
	case WM_DESTROY:
		break;
	default:
		return DefWindowProcW(hWnd, msg, wp, lp);
	}
	return 0;
}

//Обработка сообщений главного окна 
LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg,
	WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_COMMAND:
		switch (wp) {
		case MENU_IMPRINT:
			ShowWindow(hWnd, SW_HIDE);
			ShowWindow(ImprintWnd, SW_RESTORE);
			break;
		case MENU_SCAN:
			ShowWindow(hWnd, SW_HIDE);
			ShowWindow(ScanWnd, SW_RESTORE);
			break;
		case MENU_RESULTS:
			ShowWindow(hWnd, SW_HIDE);
			ShowWindow(ResultsWnd, SW_RESTORE);
			break;
		case MENU_EXIT:
			MessageBeep(MB_OK);
			if (MessageBoxW(hWnd, L"Are you sure?", L"Exit", MB_YESNO) == IDYES)
				DestroyWindow(hWnd);
			break;
		case MENU_ABOUT:
			MessageBoxW(NULL, L"Program inspector", L"Info", MB_OK);
			break;
		}
		break;
	case WM_CREATE:
		MainMenu(hWnd);
		SetControls(hWnd); 
		break;
	case WM_CLOSE:
		MessageBeep(MB_OK);
		if (MessageBoxW(hWnd, L"Are you sure?", L"Exit", MB_YESNO) == IDYES)
			DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProcW(hWnd, msg, wp, lp);
	}
	return 0;
}

//Создание окна Results
void SetResultsWnd(void) {
	WNDCLASSW wc;
	memset(&wc, 0, sizeof(WNDCLASSW));
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hInstance = My_hInst;
	wc.lpszClassName = L"myResultsClass";
	wc.lpfnWndProc = wndResultsProcedure;

	if (!RegisterClassW(&wc))
		return;

	ResultsWnd = CreateWindowW(wc.lpszClassName, L"Results",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, (ScreenX - WND_RS_WIDTH) / 2,
		(ScreenY - WND_RS_HEIGHT) / 2, WND_RS_WIDTH, WND_RS_HEIGHT, NULL, NULL, My_hInst, NULL);
}

//Создание окна Imprint
void SetImprintWnd(void) {
	WNDCLASSW wc;
	memset(&wc, 0, sizeof(WNDCLASSW));
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hInstance = My_hInst;
	wc.lpszClassName = L"myChildClass";
	wc.lpfnWndProc = wndImprintProcedure;

	if (!RegisterClassW(&wc))
		return;

	ImprintWnd = CreateWindowW(wc.lpszClassName, L"Imprint",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, (ScreenX - WND_WIDTH) / 2,
		(ScreenY - WND_CH_HEIGHT) / 2, WND_WIDTH, WND_CH_HEIGHT, NULL, NULL, My_hInst, NULL);
}

//Создание окна Scan
void SetScanWnd(void) {
	WNDCLASSW wc;
	memset(&wc, 0, sizeof(WNDCLASSW));
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hInstance = My_hInst;
	wc.lpszClassName = L"myScanClass";
	wc.lpfnWndProc = wndScanProcedure;

	if (!RegisterClassW(&wc))
		return;

	ScanWnd = CreateWindowW(wc.lpszClassName, L"Scan",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, (ScreenX - WND_WIDTH) / 2,
		(ScreenY - WND_CH_HEIGHT) / 2, WND_WIDTH, WND_CH_HEIGHT, NULL, NULL, My_hInst, NULL);
}

/*
*Создание слепка диска
*/

int files_count = 0;
void GetImprint(HWND cb)
{
	WCHAR drive_str[20] = L"";
	uint32_t res = 0;
	WCHAR str_res[40];

	htable_t *h_table = _ht_init();

	memset(drive_str, '\0', 10*sizeof(WCHAR));

	LRESULT i = SendMessageW(cb, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

	SendMessageW(cb, (UINT)CB_GETLBTEXT, (WPARAM)i, (LPARAM)drive_str);

	//size_t len = wcstombs(ch_drive_str, drive_str, wcslen(drive_str));
	//if (len > 0u)
		//ch_drive_str[len] = '\0';

	if (wcscmp(drive_str, L"All") == 0) {
		MessageBeep(MB_OK);
		MessageBoxW(cb, L"I can't find any files...", L"Imprint", MB_OK);
	}
	else {
		//thread
		recursiveFilesCount(drive_str, &files_count);
		//подготовка прогрес бара:
		//ShowWindow(hImprintProgressBar, SW_SHOW);
		//SendMessageW(hImprintProgressBar, PBM_SETRANGE, 0, (LPARAM) MAKELONG(0, files_count));    
		//SendMessageW(hImprintProgressBar, PBM_SETSTEP, (WPARAM) 1, 0); 
		PB_init(hImprintProgressBar, files_count);
		ListDirectoryContents(drive_str, h_table->ht_items, hImprintProgressBar);
		//strcat_s(ch_drive_str, 10, ".csm");
		fname_to_str(drive_str);		
		wcscat_s(drive_str, 20, L".csm");
		res = (uint32_t)fhash_save(h_table->ht_items, drive_str);
	}
	ht_destroy(h_table->ht_items);
	free(h_table);
	swprintf_s(str_res, 40, L"%u files was imprinted!", res);
	MessageBoxW(NULL, str_res, L"Imprint", MB_OK);
	ShowWindow(hImprintProgressBar, SW_HIDE);
	MessageBeep(MB_OK);
	SendMessageW(ImprintWnd, WM_CLOSE, (WPARAM)0, (LPARAM)0);
	hImprintThread = NULL;
}

int PB_init(HANDLE pb, int pb_size) {
	ShowWindow(pb, SW_SHOW);
	SendMessageW(pb, PBM_SETRANGE, 0, (LPARAM) MAKELONG(0, pb_size));    
	SendMessageW(pb, PBM_SETSTEP, (WPARAM) 1, 0); 
}

// Сканирование диска
void GetScan(HWND cb)
{
	// Указатели на измененные файлы
	ht_item_t *tmp_changed = NULL, *tmp_deleted = NULL, *tmp_new = NULL, *ptr_to_del = NULL;

	WCHAR drive_str[20] = L"";
	WCHAR _drive_str[20] = L"";
	uint32_t res = 0;
	WCHAR str_res[40];

	memset(drive_str, L'\0', 20 * sizeof(WCHAR));

	LRESULT i = SendMessageW(cb, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

	SendMessageW(cb, (UINT)CB_GETLBTEXT, (WPARAM)i, (LPARAM)drive_str);

	//size_t len = wcstombs(ch_drive_str, drive_str, wcslen(drive_str));
	//if (len > 0u)
	//	ch_drive_str[len] = '\0';

	if (wcscmp(drive_str, L"All") == 0) {
		MessageBeep(MB_OK);
		MessageBoxW(NULL, L"I can't find any files...", L"Imprint", MB_OK);
	}
	else {
		//strcpy_s(_ch_drive_str, 10, ch_drive_str);
		wcscpy_s(_drive_str, 20, drive_str);
		fname_to_str(drive_str);
		wcscat_s(drive_str, 20, L".csm");

		recursiveFilesCount(_drive_str, &files_count);
		//подготовка прогрес бара:
		//ShowWindow(hScanProgressBar, SW_SHOW);
		//SendMessageW(hScanProgressBar, PBM_SETRANGE, 0, (LPARAM) MAKELONG(0, files_count));    
		//SendMessageW(hScanProgressBar, PBM_SETSTEP, (WPARAM) 1, 0); 
		PB_init(hScanProgressBar, files_count);
		//memory leak*
		if (_ht_chngs(&tmp_new, &tmp_deleted, &tmp_changed, drive_str, _drive_str, hScanProgressBar) == -1) {
			MessageBeep(MB_OK);
			if (MessageBoxW(NULL, L"There is not imprint of this file.\nMake it?", L"No Imprint", MB_YESNO) == IDYES) {
				GetImprint(cbDrivesScan);
				SendMessageW(ScanWnd, WM_CLOSE, (WPARAM)0, (LPARAM)0);
			}
		}
		else {
			//clear table
			SendMessageW(liboChanged, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
			SendMessageW(liboNew, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
			SendMessageW(liboDeleted, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
			while (tmp_deleted) {
				ptr_to_del = tmp_deleted;
				SendMessageW(liboDeleted, LB_ADDSTRING, (WPARAM)0, (LPARAM)tmp_deleted->fname);
				tmp_deleted = tmp_deleted->pnext;
				free(ptr_to_del);
			}
			free(tmp_deleted);
			while (tmp_changed) {
				ptr_to_del = tmp_changed;
				SendMessageW(liboChanged, LB_ADDSTRING, (WPARAM)0, (LPARAM)tmp_changed->fname);
				tmp_changed = tmp_changed->pnext;
				free(ptr_to_del);
			}
			free(tmp_changed);
			while (tmp_new) {
				ptr_to_del = tmp_new;
				SendMessageW(liboNew, LB_ADDSTRING, (WPARAM)0, (LPARAM)tmp_new->fname);
				tmp_new = tmp_new->pnext;
				free(ptr_to_del);
			}
			free(tmp_new);
			MessageBeep(MB_OK);
			if (MessageBoxW(NULL, L"Scan ends.\nDo you want to see results now?", L"Scan", MB_YESNO) == IDYES) {
				SendMessageW(ScanWnd, WM_CLOSE, (WPARAM)0, (LPARAM)0);
				SendMessageW(MainWnd, WM_COMMAND, (WPARAM)MENU_RESULTS, (LPARAM)0);
			}
		}
	}
	// FINILIZE:
	ShowWindow(hScanProgressBar, SW_HIDE);
	hScanThread = NULL;
}