
#include "emu.h"
#include <windows.h>
#include "emuopts.h"
#include "window.h"
#include "resource.h"
#include "strconv.h"
#include <commdlg.h>
#include <io.h>
#undef delete
#include <string>

#ifndef W_OK
#define W_OK 4
#endif

static running_machine *machine;
bool bReplayReadOnly = false;
WCHAR szChoice[_MAX_PATH] = L"";
OPENFILENAME ofn;
WCHAR szFilter[_MAX_PATH];

static std::string empty_driver("empty");

#define _AtoT(a) ANSIToTCHAR(a, NULL, 0)
static WCHAR* ANSIToTCHAR(const char* pszInString, WCHAR* pszOutString, int nOutSize)
{
	static WCHAR szStringBuffer[1024];

	WCHAR* pszBuffer = pszOutString ? pszOutString : szStringBuffer;
	int nBufferSize  = pszOutString ? nOutSize * 2 : sizeof(szStringBuffer);

	if (MultiByteToWideChar(CP_ACP, 0, pszInString, -1, pszBuffer, nBufferSize)) {
		return pszBuffer;
	}

	return NULL;
}

#define _TtoA(a)	TCHARToANSI(a, NULL, 0)
static char* TCHARToANSI(const TCHAR* pszInString, char* pszOutString, int nOutSize)
{
	static char szStringBuffer[1024];
	memset(szStringBuffer, 0, sizeof(szStringBuffer));

	char* pszBuffer = pszOutString ? pszOutString : szStringBuffer;
	int nBufferSize = pszOutString ? nOutSize * 2 : sizeof(szStringBuffer);

	if (WideCharToMultiByte(CP_ACP, 0, pszInString, -1, pszBuffer, nBufferSize, NULL, NULL)) {
		return pszBuffer;
	}

	return NULL;
}

static void MakeOfn(TCHAR* pszFilter)
{
//	wsprintf(pszFilter, FBALoadStringEx(GetModuleHandle(NULL), IDS_DISK_FILE_REPLAY, true), _T(APP_TITLE));
//	memcpy(pszFilter + _tcslen(pszFilter), _T(" (*.mar)\0*.mar\0\0"), 14 * sizeof(TCHAR));
	memcpy(pszFilter, L" (*.mar)\0*.mar\0\0", 14 * sizeof(WCHAR));

	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = win_window_list->hwnd;
	ofn.lpstrFilter = pszFilter;
	ofn.lpstrFile = szChoice;
	ofn.nMaxFile = sizeof(szChoice) / sizeof(TCHAR);
	ofn.lpstrInitialDir = L".\\inp";
	ofn.Flags = OFN_NOCHANGEDIR | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = L"fr";

	return;
}

//------------------------------------------------------

static void GetRecordingPath(WCHAR* szPath)
{
	WCHAR szDrive[_MAX_PATH];
	WCHAR szDirectory[_MAX_PATH];
	WCHAR szFilename[_MAX_PATH];
	WCHAR szExt[_MAX_PATH];
	szDrive[0] = '\0';
	szDirectory[0] = '\0';
	szFilename[0] = '\0';
	szExt[0] = '\0';
	_wsplitpath(szPath, szDrive, szDirectory, szFilename, szExt);
	if (szDrive[0] == '\0' && szDirectory[0] == '\0') {
		WCHAR szTmpPath[_MAX_PATH];
		wcscpy(szTmpPath, L"inp\\");
		wcsncpy(szTmpPath + wcslen(szTmpPath), szPath, _MAX_PATH - wcslen(szTmpPath));
		szTmpPath[_MAX_PATH-1] = '\0';
		wcscpy(szPath, szTmpPath);
	}
}

static void DisplayReplayProperties(HWND hDlg, bool bClear)
{
	UINT8 movie_header[INP_HEADER_SIZE];
	static bool bReadOnlyStatus = true;
	UINT32 total_frames;
	UINT32 rerecord_count;

	// save status of read only checkbox
	if (IsWindowEnabled(GetDlgItem(hDlg, IDC_READONLY))) {
		bReadOnlyStatus = (BST_CHECKED == SendDlgItemMessage(hDlg, IDC_READONLY, BM_GETCHECK, 0, 0));
	}

	// set default values
	SetDlgItemTextA(hDlg, IDC_LENGTH, "");
	SetDlgItemTextA(hDlg, IDC_FRAMES, "");
	SetDlgItemTextA(hDlg, IDC_UNDO, "");
//	SetDlgItemTextA(hDlg, IDC_METADATA, "");
//	SetDlgItemTextA(hDlg, IDC_REPLAYRESET, "");
	EnableWindow(GetDlgItem(hDlg, IDC_READONLY), FALSE);
	SendDlgItemMessage(hDlg, IDC_READONLY, BM_SETCHECK, BST_UNCHECKED, 0);
	EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
	if(bClear) {
		return;
	}

	long lCount = SendDlgItemMessage(hDlg, IDC_CHOOSE_LIST, CB_GETCOUNT, 0, 0);
	long lIndex = SendDlgItemMessage(hDlg, IDC_CHOOSE_LIST, CB_GETCURSEL, 0, 0);
	if (lIndex == CB_ERR) {
		return;
	}

	if (lIndex == lCount - 1) { // Last item is "Browse..."
		EnableWindow(GetDlgItem(hDlg, IDOK), TRUE); // Browse is selectable
		return;
	}

	long lStringLength = SendDlgItemMessage(hDlg, IDC_CHOOSE_LIST, CB_GETLBTEXTLEN, (WPARAM)lIndex, 0);
	if(lStringLength + 1 > _MAX_PATH) {
		return;
	}

	SendDlgItemMessage(hDlg, IDC_CHOOSE_LIST, CB_GETLBTEXT, (WPARAM)lIndex, (LPARAM)szChoice);

	// check relative path
	GetRecordingPath(szChoice);

	// open the playback file
	FILE* fd = _wfopen(szChoice, L"r+b");
	if (!fd) {
		return;
	}

	if (fread(movie_header, 1, sizeof(movie_header), fd) != sizeof(movie_header))
		return;
	if (memcmp(movie_header, "MAMETAS\0", 8) != 0)
		return;
	if (movie_header[0x08] != INP_HEADER_MAJVERSION)
		return;

	if (_waccess(szChoice, W_OK)) {
		SendDlgItemMessage(hDlg, IDC_READONLY, BM_SETCHECK, BST_CHECKED, 0);
	} else {
		EnableWindow(GetDlgItem(hDlg, IDC_READONLY), TRUE);
		SendDlgItemMessage(hDlg, IDC_READONLY, BM_SETCHECK, BST_CHECKED, 0); //read-only by default
	}

/*
	fread(&movieFlags, 1, 1, fd);						// Read identifier
	if (movieFlags&MOVIE_FLAG_FROM_POWERON)			// starts from reset
		bStartFromReset = 1;
	else
		bStartFromReset = 0;

	if (!bStartFromReset) {
	memset(ReadHeader, 0, 4);
	fread(ReadHeader, 1, 4, fd);						// Read identifier
	if (memcmp(ReadHeader, szSavestateHeader, 4)) {		// Not the chunk type
		fclose(fd);
		return;
	}
*/

	fread(&total_frames, 1, sizeof(total_frames), fd);
	fread(&rerecord_count, 1, sizeof(rerecord_count), fd);

/*
	// read metadata
	fseek(fd, nChunkDataPosition + nChunkSize, SEEK_SET);
	memset(ReadHeader, 0, 4);
	fread(ReadHeader, 1, 4, fd);						// Read identifier
	if (memcmp(ReadHeader, szMetadataHeader, 4) == 0) {
		nChunkSize = 0;
		fread(&nChunkSize, 1, 4, fd);
		int nMetaLen = nChunkSize >> 1;
		if(nMetaLen >= MAX_METADATA) {
			nMetaLen = MAX_METADATA-1;
		}
		local_metadata = (wchar_t*)malloc((nMetaLen+1)*sizeof(wchar_t));
		int i;
		for(i=0; i<nMetaLen; ++i) {
			wchar_t c = 0;
			c |= fgetc(fd) & 0xff;
			c |= (fgetc(fd) & 0xff) << 8;
			local_metadata[i] = c;
		}
		local_metadata[i] = L'\0';
	}
*/

	// done reading file
	fclose(fd);

	// file exists and is the corrent format,
	// so enable the "Ok" button
	EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);

	// turn nFrames into a length string
	int nFPS = 60; // TODO: where to get FPS for each driver?
	int nSeconds = (total_frames * 100 + (nFPS>>1)) / nFPS;
	int nMinutes = nSeconds / 60;
	int nHours = nSeconds / 3600;

	// write strings to dialog
	char szFramesString[32];
	char szLengthString[32];
	char szUndoCountString[32];
	sprintf(szFramesString, "%d", total_frames);
	sprintf(szLengthString, "%02d:%02d:%02d", nHours, nMinutes % 60, nSeconds % 60);
	sprintf(szUndoCountString, "%d", rerecord_count);

	SetDlgItemTextA(hDlg, IDC_LENGTH, szLengthString);
	SetDlgItemTextA(hDlg, IDC_FRAMES, szFramesString);
	SetDlgItemTextA(hDlg, IDC_UNDO, szUndoCountString);
//	SetDlgItemTextW(hDlg, IDC_METADATA, local_metadata);
//	if (bStartFromReset)
//		SetDlgItemTextA(hDlg, IDC_REPLAYRESET, "Power-On");
//	else
//		SetDlgItemTextA(hDlg, IDC_REPLAYRESET, "Savestate");
//	free(local_metadata);
}

static BOOL CALLBACK ReplayDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM)
{
	if (Msg == WM_INITDIALOG) {
		WCHAR szFindPath[_MAX_PATH];
		WIN32_FIND_DATA wfd;
		HANDLE hFind;
		int i = 0;

		SendDlgItemMessage(hDlg, IDC_READONLY, BM_SETCHECK, BST_UNCHECKED, 0);

		memset(&wfd, 0, sizeof(WIN32_FIND_DATA));
		wsprintf(szFindPath, L"inp\\%s*.mar", _AtoT(machine->basename()));

		hFind = FindFirstFile(szFindPath, &wfd);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if(!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					SendDlgItemMessage(hDlg, IDC_CHOOSE_LIST, CB_INSERTSTRING, i++, (LPARAM)wfd.cFileName);
			} while(FindNextFile(hFind, &wfd));
			FindClose(hFind);
		}
		SendDlgItemMessage(hDlg, IDC_CHOOSE_LIST, CB_INSERTSTRING, i, (LPARAM)L"Browse...");
		SendDlgItemMessage(hDlg, IDC_CHOOSE_LIST, CB_SETCURSEL, i, 0);

		if (i>0) {
			SendDlgItemMessage(hDlg, IDC_CHOOSE_LIST, CB_SETCURSEL, i-1, 0);
			DisplayReplayProperties(hDlg, false);
		}

		SetFocus(GetDlgItem(hDlg, IDC_CHOOSE_LIST));
		return FALSE;
	}

	if (Msg == WM_COMMAND) {
		if (HIWORD(wParam) == CBN_SELCHANGE) {
			LONG lCount = SendDlgItemMessage(hDlg, IDC_CHOOSE_LIST, CB_GETCOUNT, 0, 0);
			LONG lIndex = SendDlgItemMessage(hDlg, IDC_CHOOSE_LIST, CB_GETCURSEL, 0, 0);
			if (lIndex != CB_ERR) {
				DisplayReplayProperties(hDlg, (lIndex == lCount - 1));		// Selecting "Browse..." will clear the replay properties display
			}
		} else if (HIWORD(wParam) == CBN_CLOSEUP) {
			LONG lCount = SendDlgItemMessage(hDlg, IDC_CHOOSE_LIST, CB_GETCOUNT, 0, 0);
			LONG lIndex = SendDlgItemMessage(hDlg, IDC_CHOOSE_LIST, CB_GETCURSEL, 0, 0);
			if (lIndex != CB_ERR) {
				if (lIndex == lCount - 1) {
					// send an OK notification to open the file browser
					SendMessage(hDlg, WM_COMMAND, (WPARAM)IDOK, 0);
				}
			}
		} else {
			int wID = LOWORD(wParam);
			switch (wID) {
				case IDOK:
					{
						LONG lCount = SendDlgItemMessage(hDlg, IDC_CHOOSE_LIST, CB_GETCOUNT, 0, 0);
						LONG lIndex = SendDlgItemMessage(hDlg, IDC_CHOOSE_LIST, CB_GETCURSEL, 0, 0);
						if (lIndex != CB_ERR) {
							if (lIndex == lCount - 1) {
								MakeOfn(szFilter);
								ofn.lpstrTitle = L"Replay Input from File";
								ofn.Flags &= ~OFN_HIDEREADONLY;

								int nRet = GetOpenFileName(&ofn);
								if (nRet != 0) {
									LONG lOtherIndex = SendDlgItemMessage(hDlg, IDC_CHOOSE_LIST, CB_FINDSTRING, (WPARAM)-1, (LPARAM)szChoice);
									if (lOtherIndex != CB_ERR) {
										// select already existing string
										SendDlgItemMessage(hDlg, IDC_CHOOSE_LIST, CB_SETCURSEL, lOtherIndex, 0);
									} else {
										SendDlgItemMessage(hDlg, IDC_CHOOSE_LIST, CB_INSERTSTRING, lIndex, (LPARAM)szChoice);
										SendDlgItemMessage(hDlg, IDC_CHOOSE_LIST, CB_SETCURSEL, lIndex, 0);
									}
									// restore focus to the dialog
									SetFocus(GetDlgItem(hDlg, IDC_CHOOSE_LIST));
									DisplayReplayProperties(hDlg, false);
									if (ofn.Flags & OFN_READONLY) {
										SendDlgItemMessage(hDlg, IDC_READONLY, BM_SETCHECK, BST_CHECKED, 0);
									} else {
										SendDlgItemMessage(hDlg, IDC_READONLY, BM_SETCHECK, BST_UNCHECKED, 0);
									}
								}
							} else {
								// get readonly status
								bReplayReadOnly = false;
								if (BST_CHECKED == SendDlgItemMessage(hDlg, IDC_READONLY, BM_GETCHECK, 0, 0)) {
									bReplayReadOnly = true;
								}
								EndDialog(hDlg, 1); // only allow OK if a valid selection was made
							}
						}
					}
					return TRUE;
				case IDCANCEL:
					szChoice[0] = '\0';
					EndDialog(hDlg, 0);
					return FALSE;
			}
		}
	}

	return FALSE;
}

void start_playback_dialog(running_machine *machine_ptr)
{
	machine = machine_ptr;
	if (empty_driver.compare(machine->basename()) == 0) {
		MessageBox(win_window_list->hwnd,L"You can't replay a movie before loading a game.",L"Replay input",MB_OK | MB_ICONSTOP);
		return;
	}
	if (get_record_file(machine) || get_playback_file(machine)) {
		MessageBox(win_window_list->hwnd,L"There's already a movie running.",L"Replay input",MB_OK | MB_ICONSTOP);
		return;
	}
	if (DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_REPLAYINP), win_window_list->hwnd, ReplayDialogProc)) {
		schedule_playback(_TtoA(szChoice));
		machine->schedule_hard_reset();
	}
}

static int VerifyRecordingAccessMode(WCHAR* szFilename, int mode)
{
	GetRecordingPath(szFilename);
	if(_waccess(szFilename, mode)) {
		return 0; // not writeable, return failure
	}

	return 1;
}

static void VerifyRecordingFilename(HWND hDlg)
{
	WCHAR szFilename[_MAX_PATH];
	GetDlgItemText(hDlg, IDC_FILENAME, szFilename, _MAX_PATH);

	// if filename null, or, file exists and is not writeable
	// then disable the dialog controls
	if(szFilename[0] == '\0' ||
		(VerifyRecordingAccessMode(szFilename, 0) != 0 && VerifyRecordingAccessMode(szFilename, W_OK) == 0)) {
		EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
//		EnableWindow(GetDlgItem(hDlg, IDC_METADATA), FALSE);
	} else {
		EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
//		EnableWindow(GetDlgItem(hDlg, IDC_METADATA), TRUE);
	}
}

static BOOL CALLBACK RecordDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM)
{
	if (Msg == WM_INITDIALOG) {
		// come up with a unique name
		WCHAR szPath[_MAX_PATH];
		WCHAR szFilename[_MAX_PATH];
		int i = 0;
		wsprintf(szFilename, L"%s.mar", _AtoT(machine->basename()));
		wcscpy(szPath, szFilename);
		while(VerifyRecordingAccessMode(szPath, 0) == 1) {
			wsprintf(szFilename, L"%s-%d.mar", _AtoT(machine->basename()), ++i);
			wcscpy(szPath, szFilename);
		}

		SetDlgItemText(hDlg, IDC_FILENAME, szFilename);
//		SetDlgItemTextW(hDlg, IDC_METADATA, L"");
//		CheckDlgButton(hDlg, IDC_REPLAYRESET, BST_CHECKED);

		VerifyRecordingFilename(hDlg);

//		SetFocus(GetDlgItem(hDlg, IDC_METADATA));
		return FALSE;
	}

	if (Msg == WM_COMMAND) {
		if (HIWORD(wParam) == EN_CHANGE) {
			VerifyRecordingFilename(hDlg);
		} else {
			int wID = LOWORD(wParam);
			switch (wID) {
				case IDC_BROWSE:
					{
						wsprintf(szChoice, L"%s", _AtoT(machine->basename()));
						MakeOfn(szFilter);
						ofn.lpstrTitle = L"Record Input to File";
						ofn.Flags |= OFN_OVERWRITEPROMPT;
						int nRet = GetSaveFileName(&ofn);
						if (nRet != 0) {
							// this should trigger an EN_CHANGE message
							SetDlgItemText(hDlg, IDC_FILENAME, szChoice);
						}
					}
					return TRUE;
				case IDOK:
					GetDlgItemText(hDlg, IDC_FILENAME, szChoice, _MAX_PATH);
//					GetDlgItemTextW(hDlg, IDC_METADATA, wszMetadata, MAX_METADATA);
//					bStartFromReset = false;
//					if (BST_CHECKED == SendDlgItemMessage(hDlg, IDC_REPLAYRESET, BM_GETCHECK, 0, 0)) {
//						bStartFromReset = true;
//					}
//					wszMetadata[MAX_METADATA-1] = L'\0';
					// ensure a relative path has the "inp\" path in prepended to it
					GetRecordingPath(szChoice);
					EndDialog(hDlg, 1);
					return TRUE;
				case IDCANCEL:
					szChoice[0] = '\0';
					EndDialog(hDlg, 0);
					return FALSE;
			}
		}
	}

	return FALSE;
}

void start_record_dialog(running_machine *machine_ptr)
{
	machine = machine_ptr;
	if (empty_driver.compare(machine->basename()) == 0) {
		MessageBox(win_window_list->hwnd,L"You can't record a movie before loading a game.",L"Record input",MB_OK | MB_ICONSTOP);
		return;
	}
	if (get_record_file(machine) || get_playback_file(machine)) {
		MessageBox(win_window_list->hwnd,L"There's already a movie running.",L"Record input",MB_OK | MB_ICONSTOP);
		return;
	}
	if (DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_RECORDINP), win_window_list->hwnd, RecordDialogProc)) {
		schedule_record(_TtoA(szChoice));
		machine->schedule_hard_reset();
	}
}
