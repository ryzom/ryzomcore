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

// ChooseDir.cpp : implementation file
//

#include "nel/misc/types_nl.h"
#include "stdafx.h"
#include "master.h"
#include "ChooseDir.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChooseDir dialog


CChooseDir::CChooseDir(CWnd* pParent /*=NULL*/)
	: CDialog(CChooseDir::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChooseDir)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

// ---------------------------------------------------------------------------
void CChooseDir::setPath (const string &path)
{
	_Path = path;
	_Sel = 0;
}

// ---------------------------------------------------------------------------
BOOL CChooseDir::OnInitDialog ()
{
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	CListBox *pLB = (CListBox*)GetDlgItem(IDC_LIST);

	SetCurrentDirectory (_Path.c_str());
	hFind = FindFirstFile ("*.*", &findData);
	
	while (hFind != INVALID_HANDLE_VALUE)
	{

		if ((stricmp (findData.cFileName, ".") != 0) && (stricmp (findData.cFileName, "..") != 0))
		{
			if (findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				pLB->InsertString (-1, findData.cFileName);
				string tmp = findData.cFileName;
				_Names.push_back (tmp);
			}
		}
		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	FindClose (hFind);

	return true;	
}

// ---------------------------------------------------------------------------
const char *CChooseDir::getSelected ()
{
	if (_Names.size() == 0)
		return NULL;

	if (_Sel == LB_ERR) 
		return _Names[0].c_str();
	else
		return _Names[_Sel].c_str();
}

// ---------------------------------------------------------------------------
void CChooseDir::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseDir)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChooseDir, CDialog)
	//{{AFX_MSG_MAP(CChooseDir)
	ON_LBN_SELCHANGE(IDC_LIST, OnSelChangeList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseDir message handlers

void CChooseDir::OnSelChangeList() 
{
	// TODO: Add your control notification handler code here
	CListBox *pLB = (CListBox*)GetDlgItem(IDC_LIST);
	_Sel = pLB->GetCurSel();
}
