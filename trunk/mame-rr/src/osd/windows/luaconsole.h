#ifndef _LUACONSOLE_H
#define _LUACONSOLE_H

extern HWND LuaConsoleHWnd;
INT_PTR CALLBACK DlgLuaScriptDialog(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void PrintToWindowConsole(int hDlgAsInt, const char* str);
void WinLuaOnStart(int hDlgAsInt);
void WinLuaOnStop(int hDlgAsInt);

#endif
