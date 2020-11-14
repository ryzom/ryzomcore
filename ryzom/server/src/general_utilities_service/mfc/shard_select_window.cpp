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

// shard_select_window.cpp : implementation file
//

#ifdef _WINDOWS

#include "stdafx.h"
#include "resource.h"
#include "shard_select_window.h"
#include "nel/misc/variable.h"
#include "nel/misc/path.h"
#include "../em_event_manager.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CShardSelectWindow dialog


CShardSelectWindow::CShardSelectWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CShardSelectWindow::IDD, pParent)
{
	//{{AFX_DATA_INIT(CShardSelectWindow)
	ShardName = _T("");
	//}}AFX_DATA_INIT
}


void CShardSelectWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CShardSelectWindow)
	DDX_Control(pDX, IDC_COMBO3, ShardNameCombo);
	DDX_CBString(pDX, IDC_COMBO3, ShardName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CShardSelectWindow, CDialog)
	//{{AFX_MSG_MAP(CShardSelectWindow)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CShardSelectWindow message handlers

extern NLMISC::CVariable<std::string> Shard;

BOOL CShardSelectWindow::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Build the list of shards that we're logged into

	NLMISC::CVectorSString shardNames;
	CEventManager::getInstance()->getShards(shardNames);
	uint32 shardIdx=0;
	for (uint32 i=0;i<shardNames.size();++i)
	{
		ShardNameCombo.InsertString (-1, shardNames[i].c_str());
		if (shardNames[i]==::Shard.get())
			shardIdx= i;
	}
	ShardNameCombo.SetCurSel (shardIdx);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

#endif
