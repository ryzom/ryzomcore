// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "resource.h"

INT_PTR CALLBACK MyDialogProc(
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	return FALSE;
}

void pump ()
{

	// Display the window
	MSG	msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE /* hPrevInstance */, LPTSTR /* lpCmdLine */, int /* nCmdShow */)
{
	// Windows
	HWND hwnd = CreateDialog (hInstance, MAKEINTRESOURCE(IDD_WAIT), NULL, MyDialogProc);
	RECT rect;
	RECT rectDesktop;
	GetWindowRect (hwnd, &rect);
	GetWindowRect (GetDesktopWindow (), &rectDesktop);
	SetWindowPos (hwnd, HWND_TOPMOST, (rectDesktop.right-rectDesktop.left-rect.right+rect.left)/2, (rectDesktop.bottom-rectDesktop.top-rect.bottom+rect.top)/2 , 0, 0, SWP_NOSIZE);
	ShowWindow (hwnd, SW_SHOW);

	pump ();

 	// Get the temp directory
	TCHAR tempPath[1024];
	if (GetTempPath(1024, tempPath))
	{
		TCHAR pathToDelete[1024];
		_tcscpy (pathToDelete, tempPath);
		_tcscat (pathToDelete, _T("Ryzom"));

		CreateDirectory(tempPath, NULL);
		_tcscat (tempPath, _T("Ryzom\\"));
		CreateDirectory(tempPath, NULL);

		// Copy the files

		pump ();
		// Setup.dat
		TCHAR setupFile[1024];
		_tcscpy (setupFile, tempPath);
		_tcscat(setupFile, _T("setup.exe"));
		SetFileAttributes(setupFile, GetFileAttributes(setupFile)&~FILE_ATTRIBUTE_READONLY);
		BOOL deleted = DeleteFile (setupFile);
		if (!CopyFile (_T("setup.dat"), setupFile, FALSE) && deleted)
			MessageBox (NULL, _T("Not enough disk space"), _T("Setup"), MB_OK|MB_ICONERROR);
		SetFileAttributes(setupFile, GetFileAttributes(setupFile)&~FILE_ATTRIBUTE_READONLY);

		pump ();
		// Ryzom.msi
		TCHAR msiFile[1024];
		_tcscpy(msiFile, tempPath);
		_tcscat(msiFile, _T("Ryzom.msi"));
		SetFileAttributes(msiFile, GetFileAttributes(msiFile)&~FILE_ATTRIBUTE_READONLY);
		deleted = DeleteFile (msiFile); 
		if (!CopyFile (_T("Ryzom.msi"), msiFile, FALSE) && deleted)
			MessageBox (NULL, _T("Not enough disk space"), _T("Setup"), MB_OK|MB_ICONERROR);
		SetFileAttributes(msiFile, GetFileAttributes(msiFile)&~FILE_ATTRIBUTE_READONLY);

		pump ();
		// Generate the remove bat file
		TCHAR batFile[1024];
		_tcscpy (batFile, tempPath);
		_tcscat(batFile, _T("remove.bat"));
		FILE *file = _tfopen (batFile, _T("w"));
		_ftprintf (file, _T("@echo off\nrmdir /S /Q \"%s\" > NUL\ndeltree /Y \"%s\" > NUL\n"), pathToDelete, pathToDelete);
		fclose (file);

		// Register the remove bat file
		HKEY hKey;
		RegCreateKey (HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Runonce"), &hKey);
		TCHAR batFileReg[1024];
		_stprintf (batFileReg, _T("\"%s\""), batFile);
		RegSetValueEx(hKey, _T("RyzomSetupClean"), 0, REG_SZ, (const unsigned char *)batFileReg, (DWORD)(_tcslen(batFileReg) + 1) * sizeof(TCHAR));

		pump ();
		// Get the current path
		TCHAR currentPath[1024];
		GetCurrentDirectory (1024, currentPath);
		if (currentPath[_tcslen(currentPath) - 1] == '\\')
			currentPath[_tcslen(currentPath) - 1] = 0;

		pump ();
		// Build the command line : /z"f:\"
		TCHAR option[1024];
		_stprintf(option, _T("\"%s\" /z\"%s\""), setupFile, currentPath);

		pump ();
		// Execute the setup
		STARTUPINFO         si;
		PROCESS_INFORMATION pi;
		memset(&si, 0, sizeof(si));
		memset(&pi, 0, sizeof(pi));
		si.cb = sizeof(si);
		// MessageBox (NULL, option, option, MB_OK);
		if (CreateProcess (setupFile, option, NULL, NULL, FALSE, 0, NULL, tempPath, &si, &pi))
			return 0;
	}

	return -1;
}
