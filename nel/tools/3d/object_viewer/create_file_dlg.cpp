// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

// create_file_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "create_file_dlg.h"
#include <shlobj.h>


/////////////////////////////////////////////////////////////////////////////
// CCreateFileDlg dialog


CCreateFileDlg::CCreateFileDlg(const CString &title, const std::string &defaultBasePath, const std::string &extension, CWnd* pParent /*=NULL*/)
	: CDialog(CCreateFileDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCreateFileDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	_Title = title;
	_Extension = extension;
	_DefaultBasePath = defaultBasePath;
}


void CCreateFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreateFileDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreateFileDlg, CDialog)
	//{{AFX_MSG_MAP(CCreateFileDlg)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreateFileDlg message handlers

void CCreateFileDlg::OnBrowse() 
{
	CString chosenPath;
	if (browseFolder(getStrRsc(IDS_CHOOSE_BASE_PATH), chosenPath, this->m_hWnd))
	{	
		GetDlgItem(IDC_LOCATION)->SetWindowText((LPCTSTR) chosenPath);
	}
}


//*************************************************************************************************
BOOL CCreateFileDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();	
	SetWindowText((LPCTSTR) _Title);
	GetDlgItem(IDC_LOCATION)->SetWindowText(nlUtf8ToTStr(_DefaultBasePath));
	if (!_DefaultBasePath.empty())
	{
		GetDlgItem(IDC_FILENAME)->SetFocus();
	}
	else
	{
		GetDlgItem(IDC_LOCATION)->SetFocus();
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//*************************************************************************************************
void CCreateFileDlg::OnOK()
{
	CString filename;
	GetDlgItem(IDC_FILENAME)->GetWindowText(filename);
	_Filename = NLMISC::tStrToUtf8(filename);
	CString location;
	GetDlgItem(IDC_LOCATION)->GetWindowText(location);
	_Path = NLMISC::tStrToUtf8(location);
	if (_Path.empty())
	{
		localizedMessageBox(*this, IDS_EMPTY_PATH, IDS_ERROR, MB_ICONEXCLAMATION);
		return;
	}	
	if (_Filename.empty())
	{
		localizedMessageBox(*this, IDS_FILENAME_PATH, IDS_ERROR, MB_ICONEXCLAMATION);
		return;
	}
	// check that filename is valid (should be a single file name)
	if (_Filename != NLMISC::CFile::getFilename(_Filename))
	{
		localizedMessageBox(*this, IDS_INVALID_FILENAME, IDS_ERROR, MB_ICONEXCLAMATION);
		return;
	}
	// attempt to create containing folder
	if (!NLMISC::CFile::isExists(_Path))
	{
		bool result = NLMISC::CFile::createDirectory(_Path);
		if (!result)
		{
			MessageBox((LPCTSTR) (getStrRsc(IDS_COULDNT_CREATE_DIRECTORY) + _Path.c_str()), getStrRsc(IDS_ERROR), MB_ICONEXCLAMATION);
			return;
		}
	}	
	std::string oldPath = NLMISC::CPath::getCurrentPath();
	if (!NLMISC::CPath::setCurrentPath(_Path.c_str()))
	{
		MessageBox((LPCTSTR) (getStrRsc(IDS_COULDNT_CREATE_DIRECTORY) + _Path.c_str()), getStrRsc(IDS_ERROR), MB_ICONEXCLAMATION);
		return;
	}
	_FullPath = NLMISC::CPath::getFullPath(_Filename, false);
	NLMISC::CPath::setCurrentPath(oldPath.c_str());
	// append extension if not present
	if (NLMISC::nlstricmp(NLMISC::CFile::getExtension(_Filename), _Extension) != 0)
	{
		_Filename += "." + _Extension;
		_FullPath += "." + _Extension;
	}
	CDialog::OnOK();
}

void CCreateFileDlg::OnCancel()
{
	_Filename.clear();
	_Path.clear();
	_FullPath.clear();
	CDialog::OnCancel();
}

//*************************************************************************************************
bool CCreateFileDlg::touchFile()
{
	std::string path = getPath();
	std::string filename = getFileName();
	std::string fullPath = getFullPath();		
	// check if file already exists
	if (NLMISC::CFile::isExists(fullPath))
	{
		int result = MessageBox((LPCTSTR) (CString(filename.c_str()) + getStrRsc(IDS_OVERWRITE_FILE)), getStrRsc(IDS_WARNING), MB_ICONEXCLAMATION);
		if (result !=	IDOK) return false;
	}
	// create a dummy file		
	NLMISC::COFile testFile;
	if (!testFile.open(fullPath))
	{
		localizedMessageBox(*this, IDS_CANNOT_CREATE_FILE, IDS_ERROR, MB_ICONEXCLAMATION);
		return false;
	}
	testFile.close();
	return true;
}
		
