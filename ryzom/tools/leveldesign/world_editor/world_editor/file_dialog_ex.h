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

#if !defined(AFX_FILE_DIALOG_EX_H__8301EBF3_1A27_4BAA_9A08_5DD3FD7A6D7D__INCLUDED_)
#define AFX_FILE_DIALOG_EX_H__8301EBF3_1A27_4BAA_9A08_5DD3FD7A6D7D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// file_dialog_ex.h : header file
//

// ***************************************************************************
// CFileDialogEx dialog
// ***************************************************************************

class CFileDialogEx : public CFileDialog
{
	DECLARE_DYNAMIC(CFileDialogEx)

public:
	CFileDialogEx(LPCTSTR lpszRegistryPath,
		LPCTSTR lpszFileType,
		BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);

	virtual INT_PTR DoModal( );

	virtual BOOL OnCommand( WPARAM wParam, LPARAM lParam );

protected:
	
	std::string		_RegistryPath;
	std::string		_RegistryList;
	std::string		_FileType;

	//{{AFX_MSG(CFileDialogEx)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILE_DIALOG_EX_H__8301EBF3_1A27_4BAA_9A08_5DD3FD7A6D7D__INCLUDED_)
