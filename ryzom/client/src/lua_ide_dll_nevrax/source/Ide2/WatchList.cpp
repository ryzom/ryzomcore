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

// WatchList.cpp : implementation file
//

#include "stdafx.h"
#include "ide2.h"
#include "WatchList.h"

#include "MainFrame.h"
#include "Debugger.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWatchList

CWatchList::CWatchList()
{
}

CWatchList::~CWatchList()
{
}


BEGIN_MESSAGE_MAP(CWatchList, CColumnTreeWnd)
	//{{AFX_MSG_MAP(CWatchList)	
	ON_NOTIFY(TVN_ENDLABELEDIT, TreeID, OnEndLabelEdit)	
	ON_NOTIFY(WM_CHAR, TreeID, OnChar)	
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWatchList message handlers


BOOL CWatchList::PreCreateWindow(CREATESTRUCT& cs) 
{	
	//cs.style |= TVS_HASLINES|TVS_EDITLABELS;	
	return CColumnTreeWnd::PreCreateWindow(cs);
}



void CWatchList::AddEmptyRow()
{
	CEntry	entry;
	entry.Item = GetTreeCtrl().InsertItem("", 0, 0, TVI_ROOT);
	_Entries.push_back(entry);
}


void CWatchList::OnEndLabelEdit(NMHDR * pNotifyStruct, LRESULT * result)
{
	ASSERT(!_Entries.empty());
	LPNMTVDISPINFO ptvdi = (LPNMTVDISPINFO) pNotifyStruct;
	// if item is not the last item, just replace if string not empty, or erase otherwise
	bool isLast = ptvdi->item.hItem == _Entries.back().Item;
	bool found = true;
	for (int k = 0; k < (int) _Entries.size(); ++k)
	{
		if (_Entries[k].Item == ptvdi->item.hItem)
		{
			found = true;
			if (!ptvdi->item.pszText || strlen(ptvdi->item.pszText) == 0)
			{
				_Entries.erase(_Entries.begin() + k);
				GetTreeCtrl().DeleteItem(ptvdi->item.hItem);
			}
			else
			{
				_Entries[k].Expr = ptvdi->item.pszText;
			}
			break;
		}
	}
	ASSERT(found);
	if (isLast)
	{
		AddEmptyRow();
		GetTreeCtrl().SelectItem(_Entries.back().Item);
	}
	else
	{
		GetTreeCtrl().SelectItem(ptvdi->item.hItem);
	}
	*result = FALSE;
	Redraw();
}



void CWatchList::Redraw()
{	
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	for (int k = 0; k < (int) _Entries.size(); ++k)
	{
		if (_Entries[k].Expr.empty())
		{
			GetTreeCtrl().SetItemText(_Entries[k].Item, "");
		}
		else
		{
			GetTreeCtrl().SetItemText(_Entries[k].Item, (_Entries[k].Expr + std::string("\t") + (LPCTSTR) pFrame->GetDebugger()->Eval(_Entries[k].Expr.c_str())).c_str());
		}
	}				
}

void CWatchList::OnChar(NMHDR * pNotifyStruct, LRESULT * result)
{
	/*
	NMCHAR *nm = (NMCHAR *) pNotifyStruct;
	if (_Entries.size() > 1 && nm->ch == VK_DELETE)
	{
		if (GetTreeCtrl().GetSelectedItem() != 0)
		{
			for (int k = 0; k < (int) _Entries.size(); ++k)
			{
				if (_Entries[k].Item == GetTreeCtrl().GetSelectedItem())
				{					
					_Entries.erase(_Entries.begin() + k);
					GetTreeCtrl().DeleteItem(_Entries[k].Item);
					GetTreeCtrl().SelectItem(_Entries[k % _Entries.size()].Item);
					*result = TRUE;
				}
			}			
		}
	}
	*/
	*result = FALSE;
}



void CWatchList::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
		
	CColumnTreeWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}
