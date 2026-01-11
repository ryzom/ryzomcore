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

// WatchBar.cpp: implementation of the CWatchBar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ide2.h"
#include "WatchBar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CWatchBar, CCJControlBar)
	//{{AFX_MSG_MAP(CWatchBar)
	ON_WM_CREATE()
	//ON_NOTIFY(LVN_ENDLABELEDIT, IDC_WATCHES, OnEndlabeledit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CWatchBar::CWatchBar()
{

}

CWatchBar::~CWatchBar()
{

}

int CWatchBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CCJControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here	
	if (!m_watches.Create(CRect(0, 0, 100, 100), this, IDC_WATCHES))	
	{
		TRACE0("Failed to create Watches ctrl\n");
		return -1;
	}	
	

	SetChild(&m_watches);

	m_watches.ModifyStyleEx(0, WS_EX_STATICEDGE);
	
	CHeaderCtrl& header = m_watches.GetHeaderCtrl();
	CTreeCtrl& tree = m_watches.GetTreeCtrl();

	DWORD dwStyle = GetWindowLong(tree, GWL_STYLE);
	// TMP TMP : faire comme ci dessous : 
	dwStyle |= TVS_HASBUTTONS | TVS_HASLINES | TVS_EDITLABELS;
	SetWindowLong(tree, GWL_STYLE, dwStyle);

	// TODO nico : add icons depending on type
	//m_ImgList.Create(IDB_IMAGES, 16, 1, RGB(255,0,255));
	//tree.SetImageList(&m_ImgList, TVSIL_NORMAL);

	HDITEM hditem;
	hditem.mask = HDI_TEXT | HDI_WIDTH | HDI_FORMAT;
	hditem.fmt = HDF_LEFT | HDF_STRING;
	hditem.cxy = 200;
	hditem.pszText = "Name";
	header.InsertItem(0, &hditem);
	hditem.cxy = 100;
	hditem.pszText = "Value";
	header.InsertItem(1, &hditem);
	hditem.cxy = 100;
	hditem.pszText = "Type";
	header.InsertItem(2, &hditem);
	m_watches.UpdateColumns();

	
	/*
	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	lvc.iSubItem = 0;
	lvc.pszText = "Name";
	lvc.cx = 70;
	lvc.fmt = LVCFMT_LEFT;
	m_watches.InsertColumn(0,&lvc);

	lvc.iSubItem = 1;
	lvc.pszText = "Value";
	lvc.cx = 300;
	lvc.fmt = LVCFMT_LEFT;
	m_watches.InsertColumn(2,&lvc);
	*/
/*	CHeaderCtrl& header = m_watches.GetHeaderCtrl();
	HDITEM hditem;
	hditem.mask = HDI_TEXT | HDI_WIDTH | HDI_FORMAT;
	hditem.fmt = HDF_LEFT | HDF_STRING;
	hditem.cxy = 200;
	hditem.pszText = "Name";
	header.InsertItem(0, &hditem);
	hditem.cxy = 100;
	hditem.pszText = "Value!!";
	header.InsertItem(1, &hditem);
	m_watches.UpdateColumns();
	*/

	m_watches.AddEmptyRow();

	return 0;
}

/*
void CWatchBar::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	// TODO: Add your control notification handler code here
	if ( pDispInfo->item.pszText )
		m_watches.AddEditItem(pDispInfo->item);

	*pResult = 0;
}
*/
