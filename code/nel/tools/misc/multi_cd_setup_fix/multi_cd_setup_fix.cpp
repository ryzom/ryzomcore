// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
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

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
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
	char tempPath[1024];
	if (GetTempPath(1024, tempPath))
	{
		char pathToDelete[1024];
		strcpy (pathToDelete, tempPath);
		strcat (pathToDelete, "Ryzom");

		CreateDirectory(tempPath, NULL);
		strcat (tempPath, "Ryzom\\");
		CreateDirectory(tempPath, NULL);

		// Copy the files

		pump ();
		// Setup.dat
		char setupFile[1024];
		strcpy (setupFile, tempPath);
		strcat (setupFile, "setup.exe");
		SetFileAttributes(setupFile, GetFileAttributes(setupFile)&~FILE_ATTRIBUTE_READONLY);
		BOOL deleted = DeleteFile (setupFile);
		if (!CopyFile ("setup.dat", setupFile, FALSE) && deleted)
			MessageBox (NULL, "Not enough disk space", "Setup", MB_OK|MB_ICONERROR);
		SetFileAttributes(setupFile, GetFileAttributes(setupFile)&~FILE_ATTRIBUTE_READONLY);

		pump ();
		// Ryzom.msi
		char msiFile[1024];
		strcpy (msiFile, tempPath);
		strcat (msiFile, "Ryzom.msi");
		SetFileAttributes(msiFile, GetFileAttributes(msiFile)&~FILE_ATTRIBUTE_READONLY);
		deleted = DeleteFile (msiFile); 
		if (!CopyFile ("Ryzom.msi", msiFile, FALSE) && deleted)
			MessageBox (NULL, "Not enough disk space", "Setup", MB_OK|MB_ICONERROR);
		SetFileAttributes(msiFile, GetFileAttributes(msiFile)&~FILE_ATTRIBUTE_READONLY);

		pump ();
		// Generate the remove bat file
		char batFile[1024];
		strcpy (batFile, tempPath);
		strcat (batFile, "remove.bat");
		FILE *file = fopen (batFile, "w");
		fprintf (file, "@echo off\nrmdir /S /Q \"%s\" > NUL\ndeltree /Y \"%s\" > NUL\n", pathToDelete, pathToDelete);
		fclose (file);

		// Register the remove bat file
		HKEY hKey;
		RegCreateKey (HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Runonce", &hKey);
		char batFileReg[1024];
		sprintf (batFileReg, "\"%s\"", batFile);
		RegSetValueEx(hKey, "RyzomSetupClean", 0, REG_SZ, (const unsigned char*)batFileReg, (DWORD)strlen (batFileReg)+1);

		pump ();
		// Get the current path
		char currentPath[1024];
		GetCurrentDirectory (1024, currentPath);
		if (currentPath[strlen(currentPath)-1] == '\\')
			currentPath[strlen(currentPath)-1] = 0;

		pump ();
		// Build the command line : /z"f:\"
		char option[1024];
		sprintf (option, "\"%s\" /z\"%s\"", setupFile, currentPath);

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
