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

// base_dialog.cpp : implementation file
//

#include "stdafx.h"
#include "georges_edit.h"
#include "georges_edit_doc.h"
#include "georges_edit_view.h"
#include "base_dialog.h"
#include "main_frm.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CBaseDialog dialog

CBaseDialog::CBaseDialog(int accelResource)
	: CDialog(CBaseDialog::IDD)
{
	//{{AFX_DATA_INIT(CBaseDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	Accelerator = LoadAccelerators ( theApp.m_hInstance, MAKEINTRESOURCE (accelResource) );
}


void CBaseDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBaseDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBaseDialog, CDialog)
	//{{AFX_MSG_MAP(CBaseDialog)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CDoomyControl, CStatic)
	//{{AFX_MSG_MAP(CDoomyControl)
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBaseDialog message handlers

BOOL CBaseDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBaseDialog::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
}

void CBaseDialog::getFirstItemPos (RECT &rect)
{
	rect.left = 0;
	rect.top = 0;
}

void CBaseDialog::setComboSize (RECT &rect, uint width, uint height)
{
	rect.right = rect.left + width;
	rect.bottom = rect.top + height;
}

void CBaseDialog::setComboSpinSize (RECT &rect)
{
	rect.right = rect.left + SmallWidget-SpinWidth;
	rect.bottom = rect.top + ComboHeight;
}

void CBaseDialog::setComboBrowseSize (RECT &rect)
{
	rect.right = rect.left + SmallWidget-BrowseWidth;
	rect.bottom = rect.top + ComboHeight;
}

void CBaseDialog::adjusteComboSize (RECT &rect)
{
	rect.bottom += ComboDropDownHeight;
}

void CBaseDialog::setStaticSize (RECT &rect)
{
	rect.right = rect.left + SmallWidget;
	rect.bottom = rect.top + StaticHeight;
}

void CBaseDialog::setEditSize (RECT &rect, uint width, uint height)
{
	rect.right = rect.left + width;
	rect.bottom = rect.top + height;
}

void CBaseDialog::setColorSize (RECT &rect, uint width, uint height)
{
	rect.right = rect.left + width - ResetColorWidth;
	rect.bottom = rect.top + height;
}

void CBaseDialog::setBigEditSize (RECT &rect, uint width, uint height)
{
	rect.right = rect.left + width;
	rect.bottom = rect.top + height;
}

void CBaseDialog::getNextPos (RECT &rect)
{
	rect.top = rect.bottom + WidgetSpaceHeight;
}

void CBaseDialog::getNextPosLabel (RECT &rect)
{
	rect.top = rect.bottom + LabelSpaceHeight;
}

void CBaseDialog::initWidget (CWnd &wnd)
{
	CFont* font = GetFont ();
	wnd.SetFont (font);
	wnd.ShowWindow (SW_SHOW);
}

void CBaseDialog::setListSize (RECT &rect, uint width, uint height)
{
	rect.right = rect.left + width;
	rect.bottom = rect.top + height;
}

void CBaseDialog::setEditSpinSize (RECT &rect, uint width, uint height)
{
	rect.right = rect.left + width;
	rect.bottom = rect.top + height;
}

void CBaseDialog::setSpinSize (RECT &rect, uint width, uint height)
{
	rect.right = rect.left + width;
	rect.bottom = rect.top + height;
}

void CBaseDialog::setBrowseSize (RECT &rect, uint width, uint height)
{
	rect.right = rect.left + width;
	rect.bottom = rect.top + height;
}

void CBaseDialog::setResetColorSize (RECT &rect, uint width, uint height)
{
	rect.right = rect.left + width;
	rect.bottom = rect.top + height;
}

void CBaseDialog::getNextColumn (RECT &rect)
{
	rect.left += SmallWidgetNotLimited + WidgetSpaceWidth;
	rect.top = 0;
}

void CBaseDialog::getFirstColumn (RECT &rect)
{
	rect.left = 0;
}

void CBaseDialog::getNextSpinPos (RECT &rect)
{
	rect.left = rect.right;
}

void CBaseDialog::getNextBrowsePos (RECT &rect)
{
	rect.left = rect.right;
}

void CBaseDialog::getNextColorPos (RECT &rect)
{
	rect.left = rect.right;
}

void CBaseDialog::setButtonSize (RECT &rect, uint width, uint height)
{
	rect.right = rect.left + width;
	rect.bottom = rect.top + height;
}

void CBaseDialog::OnOK ()
{
	int toot=0;
}

void CBaseDialog::OnCancel ()
{
	((CMainFrame*)theApp.m_pMainWnd)->showOutputConsole (false);
}

BOOL CBaseDialog::PreTranslateMessage(MSG* pMsg) 
{
	// Check if it is a hotkey
	/*if ( TranslateAccelerator (theApp.m_pMainWnd->m_hWnd, Accelerator, pMsg) )
		return TRUE;*/
	if (theApp.m_pMainWnd->PreTranslateMessage(pMsg))
		return TRUE;

	return CDialog::PreTranslateMessage(pMsg);
}

void CBaseDialog::setEditTextMultiLine (CEdit &edit, const char *text)
{
	string temp;
	uint size = strlen (text);
	temp.reserve (2*size);
	bool previousR=false;
	for (uint c=0; c<size; c++)
	{
		if ((text[c] == '\n') && (!previousR))
			temp += "\r\n";
		else
			temp += text[c];
		previousR = (text[c] == '\r');
	}
	edit.SetWindowText (temp.c_str ());
}	

void CBaseDialog::onOpenSelected() 
{
}

void CBaseDialog::registerLastControl ()
{
	RECT rect = {0, 0, 0, 0};
	DummyStatic0.Create ("Coucou", WS_CHILD|WS_VISIBLE|WS_TABSTOP, rect, this, 30);
	DummyStatic0.Dlg = this;
	DummyStatic0.Index = 0;
	DummyStatic1.Create ("Coucou", WS_CHILD|WS_VISIBLE|WS_TABSTOP, rect, this, 31);
	DummyStatic1.Dlg = this;
	DummyStatic1.Index = 1;
}

void CBaseDialog::unRegisterLastControl ()
{
	if (IsWindow (DummyStatic0))
	{
		DummyStatic0.DestroyWindow ();
	}
	if (IsWindow (DummyStatic1))
	{
		DummyStatic1.DestroyWindow ();
	}
}

void CDoomyControl::OnSetFocus( CWnd* pOldWnd )
{
	if (Index == 0)
		Dlg->onLastFocus ();
	else
		Dlg->onFirstFocus ();
}

void CBaseDialog::setFocusLastWidget ()
{
	CWnd *wnd = GetNextDlgTabItem ( &DummyStatic0, TRUE);
	if (wnd)
		wnd->SetFocus ();
}

void CBaseDialog::resizeWidgets (uint virtualWidth, uint evaluatedHeight)
{
	// Get the window size
	RECT rect;
	View->GetClientRect (&rect);
	int height = rect.bottom - rect.top - View->WidgetTopMargin - View->WidgetBottomMargin;
	AdjusteHeight = (uint)std::max (0, (int)(height - evaluatedHeight));

	Width = virtualWidth - View->WidgetRightMargin - View->WidgetLeftMargin;
	SmallWidget = std::min ( (Width-WidgetSpaceWidth) /2, (uint)MaxWidgetWidth);
	SmallWidgetNotLimited = (Width-WidgetSpaceWidth) /2;
}
