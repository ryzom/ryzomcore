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

#include "stdafx.h"
#include "world_editor.h"
#include "file_dialog_ex.h"

#include <dlgs.h>

using namespace std;
using namespace NLMISC;

#define PATH_REMEBERED_SIZE 15

// ***************************************************************************
// CFileDialogEx
// ***************************************************************************

IMPLEMENT_DYNAMIC(CFileDialogEx, CFileDialog)

// ***************************************************************************

CFileDialogEx::CFileDialogEx(LPCTSTR lpszRegistryPath, LPCTSTR lpszFileType,BOOL bOpenFileDialog, 
							 LPCTSTR lpszDefExt, LPCTSTR lpszFileName, DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
		CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
	_RegistryPath = lpszRegistryPath;
	_RegistryPath += "\\CFileDialogEx";
	_FileType = lpszFileType;
}

// ***************************************************************************

BEGIN_MESSAGE_MAP(CFileDialogEx, CFileDialog)
	//{{AFX_MSG_MAP(CFileDialogEx)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ***************************************************************************

INT_PTR CFileDialogEx::DoModal ()
{
	// Get the path
	char path[512];
	path[0] = 0;
	HKEY hKey;
	DWORD type = REG_SZ;
	DWORD size = 512;
	if (RegCreateKey (HKEY_CURRENT_USER, _RegistryPath.c_str (), &hKey) == ERROR_SUCCESS)
	{
		if (RegQueryValueEx (hKey, _FileType.c_str (), 0, &type, (LPBYTE)path, &size) == ERROR_SUCCESS)
			m_ofn.lpstrInitialDir = path;
	}

	// Template
	m_ofn.Flags |= OFN_ENABLETEMPLATE|OFN_EXPLORER;
	m_ofn.lpTemplateName = MAKEINTRESOURCE (IDD_DIRECTORY_SELECTOR);

	int result;
	if ((result = CFileDialog::DoModal ()) == IDOK)
	{
		// Update the path
		std::string newPath = (const char *)GetPathName ();
		newPath = NLMISC::CFile::getPath (newPath);
		RegSetValueEx (hKey, _FileType.c_str (), 0, REG_SZ, (LPBYTE)newPath.c_str (), newPath.size ()+1);

		// Update the path list
		set<string> oldPath;
		uint i;
		for (i=0; i<PATH_REMEBERED_SIZE; i++)
		{
			size = 512;
			if (RegQueryValueEx (hKey, toString (i).c_str (), 0, &type, (LPBYTE)path, &size) == ERROR_SUCCESS)
				oldPath.insert (path);
		}
		oldPath.insert (newPath);
		set<string>::const_iterator ite = oldPath.begin ();
		uint index = 0;
		while (ite != oldPath.end ())
		{
			RegSetValueEx (hKey, toString (index).c_str (), 0, REG_SZ, (LPBYTE)ite->c_str (), ite->size ()+1);
			ite++;
			index++;
		}
	}
	return result;
}

// ***************************************************************************

BOOL CFileDialogEx::OnCommand( WPARAM wParam, LPARAM lParam )
{
	if (HIWORD (wParam) == CBN_SELCHANGE)
	{
		if (LOWORD (wParam) == IDC_DIRLIST)
		{
			// The combo box
			CString text;
			CComboBox combo;
			combo.Attach (::GetDlgItem (*this, IDC_DIRLIST));
			combo.GetWindowText (text);
			combo.Detach ();

			// Set the new directory			
			TCHAR s[MAX_PATH];

			// Backup contents of 'edt1'
			HWND parent = *GetParent ();
			::GetDlgItemText (parent, edt1, s, MAX_PATH);
			
			// Replace with the directory name
			::SendMessage (parent, CDM_SETCONTROLTEXT, edt1, (LPARAM)(const char*)text);
			
			// Click on the OK button
			::SendMessage (parent, WM_COMMAND, IDOK, 0);
			
			// Restore contents of 'edt1'
			::SendMessage (parent, CDM_SETCONTROLTEXT, edt1, (LPARAM)s);
		}
	}
	return CFileDialog::OnCommand (wParam, lParam);
}

// ***************************************************************************

BOOL CFileDialogEx::OnInitDialog() 
{
	CFileDialog::OnInitDialog();
	
	// The edit box
	HWND editBox = ::GetDlgItem (*(this->GetParent ()), edt1);

	// The combo box
	CComboBox combo;
	combo.Attach (::GetDlgItem (*this, IDC_DIRLIST));

	// Insert the strings
	char text[512];
	text[0] = 0;
	HKEY hKey;
	DWORD type = REG_SZ;
	DWORD size;
	if (RegCreateKey (HKEY_CURRENT_USER, _RegistryPath.c_str (), &hKey) == ERROR_SUCCESS)
	{
		uint i;
		for (i=0; i<PATH_REMEBERED_SIZE; i++)
		{
			size = 512;
			if (RegQueryValueEx (hKey, toString (i).c_str (), 0, &type, (LPBYTE)text, &size) == ERROR_SUCCESS)
				combo.InsertString (-1, text);
		}
		if (m_ofn.lpstrInitialDir)
			combo.SelectString (-1, m_ofn.lpstrInitialDir);
	}

	combo.Detach ();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ***************************************************************************

