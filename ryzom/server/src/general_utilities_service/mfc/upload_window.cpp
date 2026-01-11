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

// upload_window.cpp : implementation file
//

#ifdef _WINDOWS

#include "stdafx.h"
#include "resource.h"
#include "upload_window.h"
#include "nel/misc/variable.h"
#include "nel/misc/path.h"
#include "game_share/file_description_container.h"
#include "../em_event_manager.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUploadWindow dialog


CUploadWindow::CUploadWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CUploadWindow::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUploadWindow)
	Shard = _T("");
	Event = _T("");
	//}}AFX_DATA_INIT
}


void CUploadWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUploadWindow)
	DDX_Control(pDX, IDC_COMBO2, EventCombo);
	DDX_Control(pDX, IDC_COMBO1, ShardCombo);
	DDX_CBString(pDX, IDC_COMBO1, Shard);
	DDX_CBString(pDX, IDC_COMBO2, Event);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUploadWindow, CDialog)
	//{{AFX_MSG_MAP(CUploadWindow)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUploadWindow message handlers

extern NLMISC::CVariable<std::string> EventDirectory;
extern NLMISC::CVariable<std::string> EventName;
extern NLMISC::CVariable<std::string> Shard;

BOOL CUploadWindow::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Build the list of events and setup default value

	CFileDescriptionContainer fdc;
	fdc.addFileSpec(EventDirectory.get()+"/*.worldedit");
	uint32 eventIdx=0;
	for (uint32 i=0;i<fdc.size();++i)
	{
		NLMISC::CSString eventName= NLMISC::CFile::getFilenameWithoutExtension(fdc[i].FileName);
		EventCombo.InsertString (-1, eventName.c_str());
		if (eventName==EventName.get())
			eventIdx= i;
	}
	EventCombo.SetCurSel (eventIdx);
		
	// Build the list of shards that we're logged into

	NLMISC::CVectorSString shardNames;
	CEventManager::getInstance()->getShards(shardNames);
	uint32 shardIdx=0;
	for (uint32 i=0;i<shardNames.size();++i)
	{
		ShardCombo.InsertString (-1, shardNames[i].c_str());
		if (shardNames[i]==::Shard.get())
			shardIdx= i;
	}
	ShardCombo.SetCurSel (shardIdx);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

#endif
