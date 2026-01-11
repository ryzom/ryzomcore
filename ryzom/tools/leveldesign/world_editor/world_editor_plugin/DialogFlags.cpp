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

// DailogFlags.cpp : implementation file
//

#include "stdafx.h"
#include "DialogFlags.h"
#include <map>
#include "plugin.h"

using namespace std;
using namespace NLMISC;
using namespace NLSOUND;

/////////////////////////////////////////////////////////////////////////////
// CDialogFlags dialog


CDialogFlags::CDialogFlags(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogFlags::IDD, pParent)
{
}

void CDialogFlags::init(CPlugin *plugin)
{
	_Plugin = plugin;
}

void CDialogFlags::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogFlags)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogFlags, CDialog)
	//{{AFX_MSG_MAP(CDialogFlags)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


int CDialogFlags::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	// TODO: Add your specialized creation code here
	return 0;
}

void CDialogFlags::OnDestroy() 
{
	CDialog::OnDestroy();
	// TODO: Add your message handler code here
}

BOOL CDialogFlags::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	return CDialog::OnCommand(wParam, lParam);
}
