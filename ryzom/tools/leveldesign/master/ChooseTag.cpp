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

// ChooseTag.cpp : implementation file
//



#include "nel/misc/types_nl.h"
#include "stdafx.h"
#include "master.h"
#include "ChooseTag.h"

#include <set>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChooseTag dialog


// ---------------------------------------------------------------------------
CChooseTag::CChooseTag (CWnd* pParent /*=NULL*/)
	: CDialog(CChooseTag::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChooseTag)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

// ---------------------------------------------------------------------------
void CChooseTag::setPath (const string &path)
{
	_Path = path;
	_Sel = 0;
}

// ---------------------------------------------------------------------------
BOOL CChooseTag::OnInitDialog ()
{
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	CListBox *pLB = (CListBox*)GetDlgItem(IDC_LIST);

	set<string> allTags;

	SetCurrentDirectory (_Path.c_str());
	hFind = FindFirstFile ("*.*", &findData);
	
	while (hFind != INVALID_HANDLE_VALUE)
	{

		if ((stricmp (findData.cFileName, ".") != 0) && (stricmp (findData.cFileName, "..") != 0))
		{
			if (findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				// Get the tag if any
				if (findData.cFileName[0] == '_')
				{
					string tmp;
					int i = 1;
					while (findData.cFileName[i] != '_')
					{
						tmp += findData.cFileName[i];
						++i;
					}
					if (allTags.find(tmp) == allTags.end())
					{
						allTags.insert (tmp);
						pLB->InsertString (-1, tmp.c_str());
						tmp = "_" + tmp + "_";
						_Names.push_back (tmp);
					}
				}
			}
		}
		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	FindClose (hFind);

	return true;	
}

// ---------------------------------------------------------------------------
const char *CChooseTag::getSelected ()
{
	if (_Names.size() == 0)
		return NULL;

	if (_Sel == LB_ERR) 
		return _Names[0].c_str();
	else
		return _Names[_Sel].c_str();
}

// ---------------------------------------------------------------------------
void CChooseTag::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseTag)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChooseTag, CDialog)
	//{{AFX_MSG_MAP(CChooseTag)
	ON_LBN_SELCHANGE(IDC_LIST, OnSelchangeList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseTag message handlers

void CChooseTag::OnSelchangeList() 
{
	// TODO: Add your control notification handler code here
	CListBox *pLB = (CListBox*)GetDlgItem(IDC_LIST);
	_Sel = pLB->GetCurSel();
}
