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

// primitive_configuration_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "world_editor.h"
#include "main_frm.h"
#include "action.h"
#include "primitive_configuration_dlg.h"

using namespace NLLIGO;

// ***************************************************************************
// CPrimitiveConfigurationDlg dialog
// ***************************************************************************

uint	CPrimitiveConfigurationDlg::LastId;

// The configuration window
CPrimitiveConfigurationDlg	PrimitiveConfigurationDlg;

CPrimitiveConfigurationDlg::CPrimitiveConfigurationDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPrimitiveConfigurationDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPrimitiveConfigurationDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	_WidthMargin = 0;
}

// ***************************************************************************

void CPrimitiveConfigurationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrimitiveConfigurationDlg)
	DDX_Control(pDX, IDC_LIST, ListCtrl);
	//}}AFX_DATA_MAP
}

// ***************************************************************************

BEGIN_MESSAGE_MAP(CPrimitiveConfigurationDlg, CDialog)
	//{{AFX_MSG_MAP(CPrimitiveConfigurationDlg)
	ON_WM_SIZE()
	ON_NOTIFY(NM_CLICK, IDC_LIST, OnClickList)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, OnRclickList)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnClickList)
	ON_COMMAND(ID_PRIMITIVECONFIGURATION_HIDE, OnPrimitiveconfigurationHide)
	ON_COMMAND(ID_PRIMITIVECONFIGURATION_SELECT, OnPrimitiveconfigurationSelect)
	ON_COMMAND(ID_PRIMITIVECONFIGURATION_SHOW, OnPrimitiveconfigurationShow)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ***************************************************************************
// CPrimitiveConfigurationDlg message handlers
// ***************************************************************************

void CPrimitiveConfigurationDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// Resize the list
	if (_WidthMargin != 0)
	{
		ListCtrl.SetWindowPos (NULL, 0, 0, cx - _WidthMargin, cy - _HeightMargin, SWP_NOMOVE|SWP_NOZORDER);
		ListCtrl.UpdateData(FALSE);
	}
}

// ***************************************************************************

BOOL CPrimitiveConfigurationDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Backup list position for resize
	RECT clientRect;
	GetClientRect (&clientRect);
	RECT listRect;
	ListCtrl.GetWindowRect(&listRect);
	ScreenToClient(&listRect);
	_WidthMargin = clientRect.right - listRect.right + listRect.left;
	_HeightMargin = clientRect.bottom - listRect.bottom + listRect.top;

	// Init list
	ListView_SetExtendedListViewStyle (ListCtrl, LVS_EX_CHECKBOXES);

	// Add the list column
	ListCtrl.InsertColumn (0, "Configuration name");
	ListCtrl.SetColumnWidth (0, 350);

	// Add configurations
	const std::vector<CPrimitiveConfigurations> &configurations = theApp.Config.getPrimitiveConfiguration();
	uint i;
	for (i=0; i<configurations.size(); i++)
	{
		ListCtrl.InsertItem (i, configurations[i].Name.c_str());
		// setItemTextUTF8 (List, nItem, subString++, entry.Strings[CEntryFile::OldSize].c_str ());
	}

	ListCtrl.UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ***************************************************************************

void CPrimitiveConfigurationDlg::OnClickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UpdateData();

	LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW) pNMHDR;

	// The item clicked
	uint item = lpnmlv->iItem;

	// Get the click flags
	UINT flags;
	ListCtrl.HitTest (lpnmlv->ptAction, &flags);

	// Update the group flags
	const std::vector<CPrimitiveConfigurations> &configurations = theApp.Config.getPrimitiveConfiguration();

	nlassert (configurations.size() == theApp.Configurations.size());

	// Flags must be over check box icon
	if (flags & LVHT_ONITEMSTATEICON)
	{
		// Item must be a valid item id
		if (item < theApp.Configurations.size())
		{
			BOOL value = ListView_GetCheckState (ListCtrl, item);
			theApp.Configurations[item].Activated = value == FALSE;
			invalidateLeftView ();
		}
	}

	*pResult = 0;
	UpdateData(FALSE);

	getMainFrame()->updateData();
}

// ***************************************************************************

void CPrimitiveConfigurationDlg::OnRclickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW) pNMHDR;

	// The item clicked
	uint item = lpnmlv->iItem;

	// Get the click flags
	UINT flags;
	ListCtrl.HitTest (lpnmlv->ptAction, &flags);

	// Flags must be over check box icon
	if ((flags & LVHT_ONITEMSTATEICON) == 0)
	{
		// Item must be a valid item id
		if (item < theApp.Configurations.size())
		{
			// Save the last item selected
			LastId = item;

			CMenu *pMenu = new CMenu;

			pMenu->LoadMenu (IDR_MENU1);

			CMenu *pSubMenu = pMenu->GetSubMenu (0);
			CPoint point = lpnmlv->ptAction;
			ClientToScreen(&point);
			pSubMenu->TrackPopupMenu (TPM_LEFTALIGN | TPM_LEFTBUTTON, point.x, point.y, this);
		}
	}

	*pResult = 0;
}

// ***************************************************************************

void CPrimitiveConfigurationDlg::OnPrimitiveconfigurationHide() 
{
	iteratePrimitives (Hide);
}

// ***************************************************************************

void CPrimitiveConfigurationDlg::OnPrimitiveconfigurationSelect() 
{
	iteratePrimitives (Select);
}

// ***************************************************************************

void CPrimitiveConfigurationDlg::OnPrimitiveconfigurationShow() 
{
	iteratePrimitives (Show);
}

// ***************************************************************************

void CPrimitiveConfigurationDlg::iteratePrimitives(TIterateAction iterateAction)
{
	// Get the configuration to test
	nlassert (LastId < theApp.Config.getPrimitiveConfiguration().size());
	const CPrimitiveConfigurations &configuration = theApp.Config.getPrimitiveConfiguration()[LastId];

	// Modification
	CWorldEditorDoc *doc = getDocument ();
	doc->beginModification();

	// For each primitive
	CDatabaseLocatorPointer locator;
	doc->getFirstLocator (locator);
	do
	{
		// Primitive ?
		if (locator.Primitive)
		{
			// Belong to this configuration ?
			if (configuration.belong (*locator.Primitive))
			{
				switch (iterateAction)
				{
				case Show:
				case Hide:
					doc->addModification(new CActionShowHide (locator, iterateAction==Show));
					break;
				case Select:
					doc->addModification(new CActionSelect (locator));
					break;
				}
			}
		}
	}
	while (locator.next ());

	doc->endModification();

	// Update
	getMainFrame()->updateData();
}

// ***************************************************************************

void CPrimitiveConfigurationDlg::destroy ()
{
	if (IsWindow (*this))
		DestroyWindow ();
	_WidthMargin = 0;
}

// ***************************************************************************
