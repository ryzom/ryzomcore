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

// header_dialog.cpp : implementation file
//

#include "stdafx.h"
#include "georges_edit.h"
#include "georges_edit_view.h"
#include "georges_edit_doc.h"
#include "header_dialog.h"
#include "action.h"
#include "left_view.h"

#include "nel/georges/type.h"

using namespace NLGEORGES;
using namespace NLMISC;


/////////////////////////////////////////////////////////////////////////////
// CHeaderDialog dialog


CHeaderDialog::CHeaderDialog () : CBaseDialog (IDR_MAINFRAME)
{
	//{{AFX_DATA_INIT(CHeaderDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	View = NULL;
}


void CHeaderDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHeaderDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHeaderDialog, CDialog)
	//{{AFX_MSG_MAP(CHeaderDialog)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHeaderDialog message handlers

void CHeaderDialog::OnSize(UINT nType, int cx, int cy) 
{
	CBaseDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
}

BOOL CHeaderDialog::OnInitDialog() 
{
	CBaseDialog::OnInitDialog();

	// Refresh sizes
	RECT viewRect;
	View->GetClientRect (&viewRect);
	uint virtualWidth = std::max ((uint)MinViewWidth, (uint)(viewRect.right-viewRect.left));
	CBaseDialog::resizeWidgets (virtualWidth, 0);
	
	// Get first item coordinate
	RECT currentPos;
	getFirstItemPos (currentPos);

	// Create the version
	setStaticSize (currentPos);
	char versionText[512];
	smprintf (versionText, 512, "Version %d.%d", 0, 0);
	LabelVersion.Create (versionText, WS_VISIBLE, currentPos, this);
	initWidget (LabelVersion);
	getNextPosLabel (currentPos);

	setButtonSize (currentPos, SmallWidget);
	IncrementVersion.Create ("Increment Version", WS_VISIBLE|WS_TABSTOP, currentPos, this, BtIncrement);
	initWidget (IncrementVersion);
	getNextPos (currentPos);

	// Create the state combo
	setStaticSize (currentPos);
	LabelState.Create ("State:", WS_VISIBLE, currentPos, this);
	initWidget (LabelState);
	getNextPosLabel (currentPos);

	setComboSize (currentPos, SmallWidget);
	RECT pos = currentPos;
	adjusteComboSize (pos);
	ComboState.Create (WS_VISIBLE|CBS_DROPDOWNLIST|WS_TABSTOP, pos, this, CbState);
	uint item;
	for (item=0; item<CFileHeader::StateCount; item++)
		ComboState.InsertString (-1, CFileHeader::getStateString ((CFileHeader::TState)item));
	ComboState.SetCurSel (0);
	initWidget (ComboState);
	getNextPos (currentPos);

	// Default value
	setStaticSize (currentPos);
	LabelComments.Create ("Comments:", WS_VISIBLE, currentPos, this);
	initWidget (LabelComments);
	getNextPosLabel (currentPos);

	setBigEditSize (currentPos, SmallWidget);
	Comments.CreateEx (WS_EX_CLIENTEDGE, _T("EDIT"), "", WS_VSCROLL|ES_OEMCONVERT|ES_MULTILINE|ES_WANTRETURN|WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL|ES_AUTOVSCROLL, currentPos, this, EdComments);
	initWidget (Comments);
	getNextPos (currentPos);

	// Min value
	setStaticSize (currentPos);
	LabelLog.Create ("Log:", WS_VISIBLE, currentPos, this);
	initWidget (LabelLog);
	getNextPosLabel (currentPos);

	setBigEditSize (currentPos, SmallWidget);
	Log.CreateEx (WS_EX_CLIENTEDGE, _T("EDIT"), "", WS_VSCROLL|ES_READONLY|ES_MULTILINE|WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL|ES_AUTOVSCROLL, currentPos, this, EdLog);
	initWidget (Log);
	getNextPos (currentPos);

	registerLastControl ();

	UpdateData (FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CHeaderDialog::resizeWidgets ()
{
	RECT viewRect;
	View->GetClientRect (&viewRect);
	uint virtualWidth = std::max ((uint)MinViewWidth, (uint)(viewRect.right-viewRect.left));

	// Refresh sizes
	CBaseDialog::resizeWidgets (virtualWidth, 0);

	// Get first item coordinate
	RECT currentPos;
	getFirstItemPos (currentPos);
	setStaticSize (currentPos);
	getNextPosLabel (currentPos);
	setButtonSize (currentPos, SmallWidget);
	getNextPos (currentPos);
	setStaticSize (currentPos);
	getNextPosLabel (currentPos);
	setComboSize (currentPos, SmallWidget);
	getNextPos (currentPos);
	setStaticSize (currentPos);
	getNextPosLabel (currentPos);
	setBigEditSize (currentPos, Width, BigEditHeight);
	getNextPos (currentPos);
	setStaticSize (currentPos);
	getNextPosLabel (currentPos);
	setBigEditSize (currentPos, Width, BigEditHeight);
	getNextPos (currentPos);

	// Refresh sizes
	CBaseDialog::resizeWidgets (virtualWidth, currentPos.bottom);

	// Get first item coordinate
	currentPos;
	getFirstItemPos (currentPos);
	
	uint adjust = AdjusteHeight / 2;

	// Resize
	setStaticSize (currentPos);
	LabelVersion.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
		currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	getNextPosLabel (currentPos);
	
	// Resize
	setButtonSize (currentPos, SmallWidget);
	IncrementVersion.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
		currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	getNextPos (currentPos);
	
	// Resize
	setStaticSize (currentPos);
	LabelState.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
		currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	getNextPosLabel (currentPos);
	
	// Resize
	setComboSize (currentPos, SmallWidget);
	RECT pos = currentPos;
	adjusteComboSize (pos);
	ComboState.SetWindowPos (NULL, pos.left, pos.top, pos.right - pos.left, 
		pos.bottom - pos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	getNextPos (currentPos);

	// Resize
	setStaticSize (currentPos);
	LabelComments.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
		currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	getNextPosLabel (currentPos);

	// Resize
	setBigEditSize (currentPos, Width, BigEditHeight + adjust);
	Comments.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
		currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	getNextPos (currentPos);

	// Resize
	setStaticSize (currentPos);
	LabelLog.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
		currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	getNextPosLabel (currentPos);

	// Resize
	setBigEditSize (currentPos, Width, BigEditHeight + AdjusteHeight - adjust);
	Log.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
		currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	getNextPos (currentPos);
	
	// Resize the current view
	View->setViewSize (
		virtualWidth,
		currentPos.bottom+CGeorgesEditView::WidgetTopMargin+CGeorgesEditView::WidgetBottomMargin);
}

void CHeaderDialog::getFromDocument (const NLGEORGES::CFileHeader &header)
{
	if (View)
	{
		// Nel standard version number
		ComboState.SetCurSel (header.State);
		char name[512];
		smprintf (name, 512, "Version %d.%d", header.MajorVersion, header.MinorVersion);
		LabelVersion.SetWindowText (name);

		// Set comments
		setEditTextMultiLine (Comments, header.Comments.c_str());

		// Set logs
		setEditTextMultiLine (Log, header.Log.c_str());
	}
}

void CHeaderDialog::setStateToDocument ()
{
	CGeorgesEditDoc *doc = View->GetDocument ();
	if (doc)
	{
		CGeorgesEditDocSub *current = doc->getSelectedObject ();
		CFileHeader *header = doc->getHeaderPtr ();
		doc->modify (new CActionString (IAction::HeaderState, toString ((int)(CFileHeader::TState)ComboState.GetCurSel ()).c_str (), 
			*doc, "", "", doc->getLeftView ()->getCurrentSelectionId (), 0), true, false);
	}
}

void CHeaderDialog::setVersionToDocument ()
{
	CGeorgesEditDoc *doc = View->GetDocument ();
	if (doc)
	{
		uint v0, v1;
		CString str;
		LabelVersion.GetWindowText (str);
		if (sscanf ((const char*)str, "Version %d.%d", &v0, &v1)==2)
		{
			v0++;
			v1=0;
			char name[512];
			smprintf (name, 512, "Version %d.%d", v0, v1);
			LabelVersion.SetWindowText (name);

			// Modify docuemnt
			doc->modify (new CActionString (IAction::HeaderVersion, name, *doc, "",  "",
				doc->getLeftView ()->getCurrentSelectionId (), 0));
		}
	}
}

void CHeaderDialog::setCommentsToDocument ()
{
	CGeorgesEditDoc *doc = View->GetDocument ();
	if (doc)
	{
		CGeorgesEditDocSub *current = doc->getSelectedObject ();
		CString str;
		Comments.GetWindowText (str);
		doc->modify (new CActionString (IAction::HeaderComments, str, *doc, "",  "",
			doc->getLeftView ()->getCurrentSelectionId (), 0));
	}
}

BOOL CHeaderDialog::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch (HIWORD(wParam))
	{
	case EN_CHANGE:
		{
			// identifier 
			switch (LOWORD(wParam))
			{
			case EdComments:
				setCommentsToDocument ();
				break;
			}
		}
		break;
	case CBN_SELCHANGE:
		{
			// identifier 
			switch (LOWORD(wParam))
			{
			case CbState:
				setStateToDocument ();
				break;
			}
		}
		break;
	case BN_CLICKED:
		{
			// identifier 
			switch (LOWORD(wParam))
			{
			case BtIncrement:
				setVersionToDocument ();
				break;
			}
		}
 	}
	
	return CWnd::OnCommand(wParam, lParam);
}

void CHeaderDialog::onFirstFocus ()
{
	View->SetFocus ();
}

void CHeaderDialog::onLastFocus ()
{
	View->setFocusLeftView ();
}
