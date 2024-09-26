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

// select_event_window.cpp : implementation file
//

#ifdef _WINDOWS

#include "stdafx.h"
#include "resource.h"
#include "select_event_window.h"
#include "nel/misc/variable.h"
#include "game_share/file_description_container.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectEventWindow dialog


CSelectEventWindow::CSelectEventWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectEventWindow::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectEventWindow)
	Name = _T("");
	//}}AFX_DATA_INIT
}


void CSelectEventWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectEventWindow)
	DDX_Control(pDX, IDC_COMBO1, NameCombo);
	DDX_CBString(pDX, IDC_COMBO1, Name);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectEventWindow, CDialog)
	//{{AFX_MSG_MAP(CSelectEventWindow)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectEventWindow message handlers

extern NLMISC::CVariable<std::string>	EventName;
extern NLMISC::CVariable<std::string>	EventDirectory;

BOOL CSelectEventWindow::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CFileDescriptionContainer fdc;
	fdc.addFileSpec(EventDirectory.get()+"/*.worldedit");

	uint32 idx=0;
	for (uint32 i=0;i<fdc.size();++i)
	{
		NLMISC::CSString eventName= NLMISC::CFile::getFilenameWithoutExtension(fdc[i].FileName);
		NameCombo.InsertString (-1, eventName.c_str());
		if (eventName==EventName.get())
			idx= i;
	}
	NameCombo.SetCurSel (idx);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

#endif
