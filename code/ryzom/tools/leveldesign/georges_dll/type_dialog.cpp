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

// type_dialog.cpp : implementation file
//

#include "stdafx.h"
#include "georges_edit.h"
#include "georges_edit_view.h"
#include "georges_edit_doc.h"
#include "type_dialog.h"
#include "action.h"
#include "left_view.h"

#include "nel/georges/type.h"

using namespace std;
using namespace NLMISC;
using namespace NLGEORGES;

/////////////////////////////////////////////////////////////////////////////
// CTypeDialog dialog


CTypeDialog::CTypeDialog () : CBaseDialog (IDR_MAINFRAME)
{
	//{{AFX_DATA_INIT(CTypeDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	View = NULL;
}


void CTypeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTypeDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTypeDialog, CDialog)
	//{{AFX_MSG_MAP(CTypeDialog)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTypeDialog message handlers

void CTypeDialog::OnSize(UINT nType, int cx, int cy) 
{
	CBaseDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
}

BOOL CTypeDialog::OnInitDialog() 
{
	CBaseDialog::OnInitDialog();

	RECT viewRect;
	View->GetClientRect (&viewRect);
	uint virtualWidth = std::max ((uint)MinViewWidth, (uint)(viewRect.right-viewRect.left));

	// Refresh sizes
	CBaseDialog::resizeWidgets (virtualWidth, 0);
	
	// Get first item coordinate
	RECT currentPos;
	RECT pos;
	getFirstItemPos (currentPos);

	// Create the type combo
	setStaticSize (currentPos);
	LabelType.Create ("Type:", WS_VISIBLE, currentPos, this);
	initWidget (LabelType);
	getNextPosLabel (currentPos);

	setComboSize (currentPos, SmallWidget);
	pos = currentPos;
	adjusteComboSize (pos);
	ComboType.Create (WS_VISIBLE|CBS_DROPDOWNLIST|WS_TABSTOP, pos, this, CbType);
	
	// Insert type string
	uint item;
	ComboType.ResetContent ();
	for (item=0; item<CType::TypeCount; item++)
		ComboType.InsertString (-1, CType::getTypeName ((CType::TType)item));

	ComboType.SetCurSel (0);
	initWidget (ComboType);
	getNextPos (currentPos);

	// Create the type combo
	setStaticSize (currentPos);
	LabelUIType.Create ("User Interface:", WS_VISIBLE, currentPos, this);
	initWidget (LabelUIType);
	getNextPosLabel (currentPos);

	setComboSize (currentPos, SmallWidget);
	pos = currentPos;
	adjusteComboSize (pos);
	ComboUIType.Create (WS_VISIBLE|CBS_DROPDOWNLIST|WS_TABSTOP, pos, this, CbUI);
	ComboUIType.SetCurSel (0);
	initWidget (ComboUIType);
	getNextPos (currentPos);

	// Default value
	setStaticSize (currentPos);
	LabelDefault.Create ("Default value:", WS_VISIBLE, currentPos, this);
	initWidget (LabelDefault);
	getNextPosLabel (currentPos);

	setComboSize (currentPos, SmallWidget);
	Default.create (WS_CHILD|WS_TABSTOP, currentPos, this, EdDefault, GEORGES_EDIT_BASE_REG_KEY"\\Type Default MemCombo", theApp.RememberListSize);
	initWidget (Default);
	getNextPos (currentPos);

	// Min value
	setStaticSize (currentPos);
	LabelMin.Create ("Min value:", WS_VISIBLE, currentPos, this);
	initWidget (LabelMin);
	getNextPosLabel (currentPos);

	setComboSize (currentPos, SmallWidget);
	Min.create (WS_CHILD|WS_TABSTOP, currentPos, this, EdMin, GEORGES_EDIT_BASE_REG_KEY"\\Type Min MemCombo", theApp.RememberListSize);
	initWidget (Min);
	getNextPos (currentPos);

	// Max value
	setStaticSize (currentPos);
	LabelMax.Create ("Max value:", WS_VISIBLE, currentPos, this);
	initWidget (LabelMax);
	getNextPosLabel (currentPos);

	setComboSize (currentPos, SmallWidget);
	Max.create (WS_CHILD|WS_TABSTOP, currentPos, this, EdMax, GEORGES_EDIT_BASE_REG_KEY"\\Type Max MemCombo", theApp.RememberListSize);
	initWidget (Max);
	getNextPos (currentPos);

	// Increment value
	setStaticSize (currentPos);
	LabelIncrement.Create ("Increment value:", WS_VISIBLE, currentPos, this);
	initWidget (LabelIncrement);
	getNextPosLabel (currentPos);

	setComboSize (currentPos, SmallWidget);
	Increment.create (WS_CHILD|WS_TABSTOP, currentPos, this, EdIncrement, GEORGES_EDIT_BASE_REG_KEY"\\Type Increment MemCombo", theApp.RememberListSize);
	initWidget (Increment);
	getNextColumn (currentPos);

	// Predef list value
	setStaticSize (currentPos);
	LabelPreDef.Create ("Predefintion list:", WS_VISIBLE, currentPos, this);
	initWidget (LabelPreDef);
	getNextPosLabel (currentPos);

	setListSize (currentPos, SmallWidgetNotLimited, 250);
	Predef.create (WS_TABSTOP, currentPos, this, LtPredef);
	Predef.Dialog = this;
	Predef.insertColumn (0, "Label");
	Predef.insertColumn (1, "Value");
	Predef.recalcColumn ();
	initWidget (Predef);
	getNextPos (currentPos);

	registerLastControl ();

	resizeWidgets ();

	UpdateData (FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTypeDialog::resizeWidgets ()
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
	setComboSize (currentPos, SmallWidget);
	getNextPos (currentPos);
	setStaticSize (currentPos);
	getNextPosLabel (currentPos);
	setComboSize (currentPos, SmallWidget);
	getNextPos (currentPos);
	setStaticSize (currentPos);
	getNextPosLabel (currentPos);
	setComboSize (currentPos, SmallWidget);
	getNextPos (currentPos);
	setStaticSize (currentPos);
	getNextPosLabel (currentPos);
	setComboSize (currentPos, SmallWidget);
	getNextPos (currentPos);
	setStaticSize (currentPos);
	getNextPosLabel (currentPos);
	setComboSize (currentPos, SmallWidget);
	getNextPos (currentPos);
	setStaticSize (currentPos);
	getNextPosLabel (currentPos);
	setComboSize (currentPos, SmallWidget);
	getNextPos (currentPos);
	setStaticSize (currentPos);
	getNextPosLabel (currentPos);
	setComboSize (currentPos, SmallWidget);

	getNextColumn (currentPos);
	setStaticSize (currentPos);
	getNextPosLabel (currentPos);
	setListSize (currentPos, SmallWidgetNotLimited, 250);
	getNextPos (currentPos);

	// Refresh sizes
	CBaseDialog::resizeWidgets (virtualWidth, currentPos.bottom);

	getFirstItemPos (currentPos);

	// Create the type combo
	setStaticSize (currentPos);
	LabelType.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
		currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	getNextPosLabel (currentPos);

	setComboSize (currentPos, SmallWidget);
	RECT pos = currentPos;
	adjusteComboSize (pos);
	ComboType.SetWindowPos (NULL, pos.left, pos.top, pos.right - pos.left, 
		pos.bottom - pos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	getNextPos (currentPos);

	// Create the type combo
	setStaticSize (currentPos);
	LabelUIType.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
		currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	getNextPosLabel (currentPos);

	setComboSize (currentPos, SmallWidget);
	pos = currentPos;
	adjusteComboSize (pos);
	ComboUIType.SetWindowPos (NULL, pos.left, pos.top, pos.right - pos.left, 
		pos.bottom - pos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	getNextPos (currentPos);

	// Default value
	setStaticSize (currentPos);
	LabelDefault.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
		currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	getNextPosLabel (currentPos);

	setComboSize (currentPos, SmallWidget);
	Default.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
		currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	getNextPos (currentPos);

	// Min value
	setStaticSize (currentPos);
	LabelMin.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
		currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	getNextPosLabel (currentPos);

	setComboSize (currentPos, SmallWidget);
	Min.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
		currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	getNextPos (currentPos);

	// Max value
	setStaticSize (currentPos);
	LabelMax.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
		currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	getNextPosLabel (currentPos);

	setComboSize (currentPos, SmallWidget);
	Max.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
		currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	getNextPos (currentPos);

	// Increment value
	setStaticSize (currentPos);
	LabelIncrement.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
		currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	getNextPosLabel (currentPos);

	setComboSize (currentPos, SmallWidget);
	Increment.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
		currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	getNextColumn (currentPos);

	// Predef list value
	setStaticSize (currentPos);
	LabelPreDef.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
		currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	getNextPosLabel (currentPos);

	setListSize (currentPos, SmallWidgetNotLimited, 250 + AdjusteHeight);
	Predef.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
		currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	Predef.recalcColumn ();
	getNextPos (currentPos);
	
	// Resize the current view
	View->setViewSize (
		virtualWidth,
		currentPos.bottom+CGeorgesEditView::WidgetTopMargin+CGeorgesEditView::WidgetBottomMargin);
}

void CTypeDialog::OnOK ()
{
	Predef.onOK ();
	if (Default.haveFocus ())
		Default.onOK ();
	if (Min.haveFocus ())
		Min.onOK ();
	if (Max.haveFocus ())
		Max.onOK ();
	if (Increment.haveFocus ())
		Increment.onOK ();
}

void CTypeDialog::OnCancel ()
{
	Predef.onCancel ();
}

void CTypeDialog::getFromDocument (const NLGEORGES::CType &type)
{
	if (View)
	{
		uint item;

		// Insert type string
		ComboType.ResetContent ();
		for (item=0; item<CType::TypeCount; item++)
			ComboType.InsertString (-1, CType::getTypeName ((CType::TType)item));

		// Insert UI types
		ComboUIType.ResetContent ();
		for (item=0; item<CType::UITypeCount; item++)
		{
			if (CType::uiCompatible (type.Type, (CType::TUI)item))
			{
				int index = ComboUIType.InsertString (-1, CType::getUIName ((CType::TUI)item));
				ComboUIType.SetItemData (index, item);
				
				if (item == (uint)type.UIType)
					ComboUIType.SetCurSel (index);
			}
		}

		ComboType.SetCurSel (type.Type);
		Default.SetWindowText (type.Default.c_str());
		Min.SetWindowText (type.Min.c_str());
		Max.SetWindowText (type.Max.c_str());
		Increment.SetWindowText (type.Increment.c_str());

		// Disable some windows
		bool number = (type.Type == UType::UnsignedInt) || (type.Type == UType::SignedInt) || (type.Type == UType::Double);
		Min.EnableWindow (number);
		Max.EnableWindow (number);
		Increment.EnableWindow (number);

		// Add the predef
		Predef.ListCtrl.DeleteAllItems ();
		uint predef;
		for (predef=0; predef<type.Definitions.size(); predef++)
		{
			// Add the label and value
			Predef.ListCtrl.InsertItem (predef, type.Definitions[predef].Label.c_str());
			Predef.ListCtrl.SetItemText (predef, 1, type.Definitions[predef].Value.c_str());
		}
	}
}

void CTypeDialog::setTypeToDocument ()
{
	CGeorgesEditDoc *doc = View->GetDocument ();
	if (doc)
	{
		CGeorgesEditDocSub *current = doc->getSelectedObject ();
		
		doc->modify (new CActionString (IAction::TypeType, toString (ComboType.GetCurSel ()).c_str (), *doc, 
			"", "", doc->getLeftView ()->getCurrentSelectionId (), 0));
	}
}

void CTypeDialog::setUIToDocument ()
{
	CGeorgesEditDoc *doc = View->GetDocument ();
	if (doc)
	{
		CGeorgesEditDocSub *current = doc->getSelectedObject ();
		int curSel = ComboUIType.GetCurSel ();
		if (curSel != CB_ERR)
		{
			curSel = (CType::TUI)ComboUIType.GetItemData (curSel);
		}
		else
		{
			curSel = CType::Edit;
		}

		doc->modify (new CActionString (IAction::TypeUI, toString (curSel).c_str (), *doc, "", "", 
			doc->getLeftView ()->getCurrentSelectionId (), 0));
	}
}

void CTypeDialog::setDefaultToDocument ()
{
	CGeorgesEditDoc *doc = View->GetDocument ();
	if (doc)
	{
		CGeorgesEditDocSub *current = doc->getSelectedObject ();
		if (current)
		{
			CString str;

			Default.UpdateData ();
			Default.GetWindowText (str);

			doc->modify (new CActionString (IAction::TypeDefault, str, *doc, "",  "",
				doc->getLeftView ()->getCurrentSelectionId (), 0));
		}
	}
}

void CTypeDialog::setMinToDocument ()
{
	CGeorgesEditDoc *doc = View->GetDocument ();
	if (doc)
	{
		CGeorgesEditDocSub *current = doc->getSelectedObject ();
		if (current)
		{
			CString str;

			Min.UpdateData ();
			Min.GetWindowText (str);

			doc->modify (new CActionString (IAction::TypeMin, str, *doc, "", "", 
				doc->getLeftView ()->getCurrentSelectionId (), 0));
		}
	}
}

void CTypeDialog::setMaxToDocument ()
{
	CGeorgesEditDoc *doc = View->GetDocument ();
	if (doc)
	{
		CGeorgesEditDocSub *current = doc->getSelectedObject ();
		CString str;

		Max.UpdateData ();
		Max.GetWindowText (str);

		doc->modify (new CActionString (IAction::TypeMax, str, *doc, "", "", 
			doc->getLeftView ()->getCurrentSelectionId (), 0));
	}
}

void CTypeDialog::setIncrementToDocument ()
{
	CGeorgesEditDoc *doc = View->GetDocument ();
	if (doc)
	{
		CGeorgesEditDocSub *current = doc->getSelectedObject ();
		CString str;

		Increment.UpdateData ();
		Increment.GetWindowText (str);

		doc->modify (new CActionString (IAction::TypeIncrement, str, *doc, "", "",
			doc->getLeftView ()->getCurrentSelectionId (), 0));
	}
}

void CTypeDialog::setPredefToDocument ()
{
	CGeorgesEditDoc *doc = View->GetDocument ();
	if (doc)
	{
		CGeorgesEditDocSub *current = doc->getSelectedObject ();
		CString str;

		// Add the predef
		vector<vector<string> > stringVector;
		stringVector.resize (Predef.ListCtrl.GetItemCount());
		uint predef;
		for (predef=0; predef<(uint)Predef.ListCtrl.GetItemCount(); predef++)
		{
			stringVector[predef].resize (2);

			// Add the label and value
			str = Predef.ListCtrl.GetItemText (predef, 0);
			stringVector[predef][0] = (const char*)str;
			str = Predef.ListCtrl.GetItemText (predef, 1);
			stringVector[predef][1] = (const char*)str;
		}

		doc->modify (new CActionStringVectorVector (IAction::TypePredef, stringVector, *doc, 
			doc->getLeftView ()->getCurrentSelectionId (), 0));
	}
}

BOOL CTypeDialog::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch (HIWORD(wParam))
	{
	case CBN_SELCHANGE:
		{
			// identifier 
			switch (LOWORD(wParam))
			{
			case CbType:
				setTypeToDocument ();
				break;
			case CbUI:
				setUIToDocument ();
				break;
			}
		}
		break;
 	}
	
	return CWnd::OnCommand(wParam, lParam);
}

LRESULT CTypeDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message)
	{
	case LC_CHANGE:
		setPredefToDocument ();
		break;
	case MC_STRINGCHANGE:
		{
			// identifier 
			switch (LOWORD(wParam))
			{
			case EdDefault:
				setDefaultToDocument ();
				break;
			case EdMin:
				setMinToDocument ();
				break;
			case EdMax:
				setMaxToDocument ();
				break;
			case EdIncrement:
				setIncrementToDocument ();
				break;
			}
		}
		break;
	}
	
	return CDialog::WindowProc(message, wParam, lParam);
}

void CTypeDialog::onLastFocus ()
{
	View->setFocusLeftView ();
}

void CTypeDialog::onFirstFocus ()
{
	View->SetFocus ();
}

CEditListCtrl::TItemEdit CTypeParentEditListCtrl::getItemEditMode (uint item, uint subItem)
{
	return CEditListCtrl::EditMemCombo;
}

void CTypeParentEditListCtrl::getMemComboBoxProp (uint item, uint subItem, std::string &regAdr, bool &browse)
{
	browse = false;
	if (subItem == 0)
		regAdr = GEORGES_EDIT_BASE_REG_KEY"\\Type Label MemCombo";
	else
		regAdr = GEORGES_EDIT_BASE_REG_KEY"\\Type Name MemCombo";
}
