// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

// dfn_dialog.cpp : implementation file
//

#include "stdafx.h"
#include "georges_edit.h"
#include "georges_edit_view.h"
#include "georges_edit_doc.h"
#include "dfn_dialog.h"
#include "left_view.h"
#include "action.h"

#include "nel/misc/path.h"
#include "nel/georges/type.h"

using namespace std;
using namespace NLMISC;
using namespace NLGEORGES;


// ***************************************************************************

CDfnDialog::CDfnDialog () : CBaseDialog (IDR_MAINFRAME)
{
	//{{AFX_DATA_INIT(CDfnDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	View = NULL;
}

// ***************************************************************************

void CDfnDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDfnDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

// ***************************************************************************

BEGIN_MESSAGE_MAP(CDfnDialog, CDialog)
	//{{AFX_MSG_MAP(CDfnDialog)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ***************************************************************************

void CDfnDialog::OnSize(UINT nType, int cx, int cy) 
{
	CBaseDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
}

// ***************************************************************************

BOOL CDfnDialog::OnInitDialog() 
{
	CBaseDialog::OnInitDialog();
	
	RECT viewRect;
	View->GetClientRect (&viewRect);
	uint virtualWidth = std::max ((uint)MinViewWidth, (uint)(viewRect.right-viewRect.left));

	// Refresh sizes
	CBaseDialog::resizeWidgets (virtualWidth, 0);

	// Get first item coordinate
	RECT currentPos;
	getFirstItemPos (currentPos);

	// Create the type combo
	setStaticSize (currentPos);
	LabelParents.Create (_T("Parents:"), WS_VISIBLE, currentPos, this);
	initWidget (LabelParents);
	getNextPosLabel (currentPos);

	setListSize (currentPos, SmallWidget, ParentHeight);
	Parents.create (WS_TABSTOP, currentPos, this, LtParents);
	Parents.insertColumn (0, _T("Parent Dfn"));
	Parents.Dialog = this;
	Parents.recalcColumn ();
	initWidget (Parents);
	getNextPos (currentPos);


	// Create the type combo
	setStaticSize (currentPos);
	LabelStruct.Create (_T("Structure:"), WS_VISIBLE, currentPos, this);
	initWidget (LabelStruct);
	getNextPosLabel (currentPos);

	setListSize (currentPos, Width, DfnHeight);
	Struct.create (WS_TABSTOP, currentPos, this, LtStruct);
	Struct.insertColumn (0, _T("Name"));
	Struct.insertColumn (1, _T("Type"));
	Struct.insertColumn (2, _T("Value"));
	Struct.insertColumn (3, _T("Default"));
	Struct.insertColumn (4, _T("FilenameExt"));
	Struct.Dialog = this;
	Struct.recalcColumn ();
	initWidget (Struct);
	getNextPos (currentPos);

	registerLastControl ();

	resizeWidgets ();

	UpdateData (FALSE);	

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ***************************************************************************

void CDfnDialog::OnOK ()
{
	Parents.UpdateData ();
	Struct.UpdateData ();

	CWnd *focusWnd = CWnd::GetFocus ();
	if (focusWnd)
	{
		focusWnd = focusWnd->GetParent ();
		if (focusWnd)
		{
			if (focusWnd->m_hWnd == Parents.m_hWnd)
			{
				Parents.onOK ();
			}
			else if (focusWnd->m_hWnd == Struct.m_hWnd)
			{
				Struct.onOK ();
			}
		}
	}
}

// ***************************************************************************

void CDfnDialog::OnCancel ()
{
	CWnd *focusWnd = CWnd::GetFocus ();
	if (focusWnd)
	{
		focusWnd = focusWnd->GetParent ();
		if (focusWnd)
		{
			if (focusWnd->m_hWnd == Parents.m_hWnd)
			{
				Parents.onCancel ();
				return;
			}
			else if (focusWnd->m_hWnd == Struct.m_hWnd)
			{
				Struct.onCancel ();
				return;
			}
		}
	}
	CBaseDialog::OnCancel ();
}

// ***************************************************************************

void CDfnDialog::getFromDocument (const NLGEORGES::CFormDfn &dfn)
{
	if (View)
	{
		// Add the parents
		Parents.ListCtrl.DeleteAllItems ();
		uint parent;
		for (parent=0; parent<dfn.getNumParent (); parent++)
		{
			// Add the label and value
			Parents.ListCtrl.InsertItem(parent, nlUtf8ToTStr(dfn.getParentFilename(parent)));
		}

		// Add the struct element
		Struct.ListCtrl.DeleteAllItems ();
		uint elm;
		for (elm=0; elm<dfn.getNumEntry (); elm++)
		{
			// Add the label and value
			Struct.ListCtrl.InsertItem(elm, nlUtf8ToTStr(dfn.getEntry(elm).getName()));
			switch (elm, dfn.getEntry (elm).getType ())
			{
			case UFormDfn::EntryType:
				Struct.ListCtrl.SetItemText (elm, 1, dfn.getEntry (elm).getArrayFlag () ? _T("Type array") : _T("Type"));
				Struct.ListCtrl.SetItemText(elm, 4, nlUtf8ToTStr(dfn.getEntry(elm).getFilenameExt()));
				break;
			case UFormDfn::EntryDfn:
				Struct.ListCtrl.SetItemText (elm, 1, dfn.getEntry (elm).getArrayFlag () ? _T("Dfn array") : _T("Dfn"));
				break;
			case UFormDfn::EntryVirtualDfn:
				Struct.ListCtrl.SetItemText (elm, 1, _T("Virtual Dfn"));
				break;
			}
			Struct.ListCtrl.SetItemText(elm, 2, nlUtf8ToTStr(dfn.getEntry(elm).getFilename()));
			Struct.ListCtrl.SetItemText(elm, 3, nlUtf8ToTStr(dfn.getEntry(elm).getDefault()));
		}
	}
}

// ***************************************************************************

void CDfnDialog::setParentsToDocument ()
{
	CGeorgesEditDoc *doc = View->GetDocument ();
	if (doc)
	{
		// Build a vector of string
		vector<string> vectValue (Parents.ListCtrl.GetItemCount ());

		// For each string
		uint parent;
		for (parent=0; parent<(uint)Parents.ListCtrl.GetItemCount (); parent++)
		{
			// Add the label and value
			CString str = Parents.ListCtrl.GetItemText ( parent, 0);
			vectValue[parent] = tStrToUtf8(str);
		}

		// Modify the document
		if (!doc->modify (new CActionStringVector (IAction::DfnParents, vectValue, *doc, "", doc->getLeftView ()->getCurrentSelectionId (), 0)))
			getFromDocument (*doc->getDfnPtr ());
	}
}

// ***************************************************************************

void CDfnDialog::setStructToDocument ()
{
	CGeorgesEditDoc *doc = View->GetDocument ();
	if (doc)
	{
		// Add the entries
		vector<vector<string> > stringVector (Struct.ListCtrl.GetItemCount ());
		uint elm;
		for (elm=0; elm<stringVector.size (); elm++)
		{
			// Resize the array
			stringVector[elm].resize (5);

			uint subElm;
			for (subElm = 0; subElm<5; subElm++)
			{
				// Get the name
				CString name= Struct.ListCtrl.GetItemText (elm, subElm);
				stringVector[elm][subElm] = tStrToUtf8(name);
			}
		}
		doc->modify (new CActionStringVectorVector (IAction::DfnStructure, stringVector, *doc, doc->getLeftView ()->getCurrentSelectionId (), 0));
	}
}

// ***************************************************************************

LRESULT CDfnDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message)
	{
	case LC_CHANGE:
		if (wParam == LtParents)
			setParentsToDocument ();
		else 
			setStructToDocument ();
		break;
	}
	
	return CDialog::WindowProc(message, wParam, lParam);
}

// ***************************************************************************

CEditListCtrl::TItemEdit CDfnEditListCtrl::getItemEditMode (uint item, uint subItem)
{
	if (subItem == 0)
		return CEditListCtrl::EditMemCombo;
	else if (subItem == 1)
		return CEditListCtrl::EditFixedCombo;
	else if (subItem == 2)
	{
		string type = tStrToUtf8(ListCtrl.GetItemText (item, 1));
		if (type != "Virtual Dfn")
			return CEditListCtrl::EditMemCombo;
	}
	else if (subItem == 3)
	{
		string type = tStrToUtf8(ListCtrl.GetItemText (item, 1));
		if ((type == "Type") || (type == "Type array"))
			return CEditListCtrl::EditMemCombo;
	}
	else if (subItem == 4)
	{
		string type = tStrToUtf8(ListCtrl.GetItemText (item, 1));
		if ((type == "Type") || (type == "Type array"))
			return CEditListCtrl::EditMemCombo;
	}
	return CEditListCtrl::NoEdit;
}

// ***************************************************************************

void CDfnEditListCtrl::getComboBoxStrings (uint item, uint subItem, std::vector<std::string> &retStrings)
{
	if (subItem == 1)
	{
		retStrings.reserve (5);
		retStrings.push_back ("Type");
		retStrings.push_back ("Dfn");
		retStrings.push_back ("Virtual Dfn");
		retStrings.push_back ("Type array");
		retStrings.push_back ("Dfn array");
	}
}

// ***************************************************************************

void CDfnEditListCtrl::getMemComboBoxProp (uint item, uint subItem, std::string &regAdr, bool &browse)
{
	if (subItem == 0)
	{
		browse = false;
		regAdr = tStrToUtf8(GEORGES_EDIT_BASE_REG_KEY _T("\\Label MemCombo"));
	}
	else if (subItem == 2)
	{
		browse = true;

		// Get type string
		string type = tStrToUtf8(ListCtrl.GetItemText (item, 1));
		if ((type == "Type") || (type == "Type array"))
			regAdr = tStrToUtf8(GEORGES_EDIT_BASE_REG_KEY _T("\\Type MemCombo"));
		else if ((type == "Dfn") || (type == "Dfn array"))
			regAdr = tStrToUtf8(GEORGES_EDIT_BASE_REG_KEY _T("\\Dfn MemCombo"));
	}
	else if (subItem == 3)
	{
		browse = false;
		regAdr = tStrToUtf8(GEORGES_EDIT_BASE_REG_KEY _T("\\Default MemCombo"));
	}
	else if (subItem == 3)
	{
		browse = false;
		regAdr = tStrToUtf8(GEORGES_EDIT_BASE_REG_KEY _T("\\FilenameExt MemCombo"));
	}
}

// ***************************************************************************

void CDfnEditListCtrl::getNewItemText (uint item, uint subItem, std::string &ret)
{
	if (subItem == 0)
		ret = "new";
	else if (subItem == 1)
		ret = "Type";
	else if (subItem == 2)
		ret = theApp.DefaultType;
	else if (subItem == 3)
		ret.clear();
	else if (subItem == 4)
		ret.clear();
}

// ***************************************************************************

void CDfnEditListCtrl::getBrowseInfo (uint item, uint subItem, std::string &defExt, std::string &defFilename, std::string &defDir, NLMISC::tstring &filter)
{
	if (subItem == 2)
	{
		// Get type string
		string type = tStrToUtf8(ListCtrl.GetItemText (item, 1));
		if ((type == "Type") || (type == "Type array"))
		{
			filter = TypeFilter;
			defDir = theApp.RootSearchPath;
			defFilename = "*.typ";
			defExt = "*.typ";
		}
		else if ((type == "Dfn") || (type == "Dfn array"))
		{
			filter = DfnFilter;
			defDir = theApp.RootSearchPath;
			defFilename = "*.dfn";
			defExt = "*.dfn";
		}
	}
}

// ***************************************************************************

void CDfnEditListCtrl::onItemChanged (uint item, uint subItem)
{
	if (subItem == 1)
	{
		// Get type string
		string type = tStrToUtf8(ListCtrl.GetItemText (item, 1));
		if ((type == "Type") || (type == "Type array"))
		{
			CString str;
			str = Dialog->Struct.ListCtrl.GetItemText (item, 2);
			std::string ext = NLMISC::CFile::getExtension(tStrToUtf8(str));
			if (ext == "typ")
				Dialog->Struct.ListCtrl.SetItemText(item, 2, nlUtf8ToTStr(theApp.DefaultType));
		}
		else if ((type == "Dfn") || (type == "Dfn array"))
		{
			CString str;
			str = Dialog->Struct.ListCtrl.GetItemText (item, 2);
			std::string ext = NLMISC::CFile::getExtension(tStrToUtf8(str));
			if (ext == "dfn")
				Dialog->Struct.ListCtrl.SetItemText(item, 2, nlUtf8ToTStr(theApp.DefaultDfn));

			// Clear default value
			Dialog->Struct.ListCtrl.SetItemText (item, 3, _T(""));
		}
		else if (type == "Virtual Dfn")
		{
			// Clear the value
			Dialog->Struct.ListCtrl.SetItemText (item, 2, _T(""));

			// Clear default value
			Dialog->Struct.ListCtrl.SetItemText (item, 3, _T(""));
		}
	}
}

// ***************************************************************************

void CDfnDialog::onOpenSelected ()
{
	Parents.UpdateData ();
	Struct.UpdateData ();

	CWnd *focusWnd = CWnd::GetFocus ();
	if (focusWnd)
	{
		focusWnd = focusWnd->GetParent ();
		if (focusWnd)
		{
			if (focusWnd->m_hWnd == Parents.m_hWnd)
			{
				// For each selected
				POSITION pos = Parents.ListCtrl.GetFirstSelectedItemPosition();
				while (pos)
				{
					int nItem = Parents.ListCtrl.GetNextSelectedItem(pos);

					// Get the string
					CString str = Parents.ListCtrl.GetItemText (nItem, 0);
					if (str != "")
					{
						// Look for the file
						CString name = nlUtf8ToTStr(CPath::lookup(tStrToUtf8(str), false, false));
						if (name.IsEmpty())
							name = str;

						// Open the file
						theApp.OpenDocumentFile(name);
					}
				}
			}
			else if (focusWnd->m_hWnd == Struct.m_hWnd)
			{
				// For each selected
				POSITION pos = Struct.ListCtrl.GetFirstSelectedItemPosition();
				while (pos)
				{
					int nItem = Struct.ListCtrl.GetNextSelectedItem(pos);

					// Get the string
					CString str = Struct.ListCtrl.GetItemText (nItem, 2);
					if (str != "")
					{
						// Look for the file
						CString name = nlUtf8ToTStr(CPath::lookup(tStrToUtf8(str), false, false));
						if (name.IsEmpty())
							name = str;

						// Open the file
						theApp.OpenDocumentFile(name);
					}
				}
			}
		}
	}
}

// ***************************************************************************

void CDfnDialog::OnSetFocus(CWnd* pOldWnd) 
{
	CBaseDialog::OnSetFocus(pOldWnd);
}

// ***************************************************************************

CWnd* CDfnDialog::GetNextDlgTabItem( CWnd* pWndCtl, BOOL bPrevious) const
{
	return NULL;
}

// ***************************************************************************

void CDfnDialog::onFirstFocus ()
{
	View->SetFocus ();
}

// ***************************************************************************

void CDfnDialog::onLastFocus ()
{
	View->setFocusLeftView ();
}

// ***************************************************************************

void CDfnDialog::resizeWidgets ()
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
	setListSize (currentPos, SmallWidget, ParentHeight);
	getNextPos (currentPos);
	setStaticSize (currentPos);
	getNextPosLabel (currentPos);
	setListSize (currentPos, Width, DfnHeight);
	getNextPos (currentPos);

	// Refresh sizes
	CBaseDialog::resizeWidgets (virtualWidth, currentPos.bottom);

	// Get first item coordinate
	currentPos;
	getFirstItemPos (currentPos);

	// Create the type combo
	setStaticSize (currentPos);
	LabelParents.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
		currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	getNextPosLabel (currentPos);

	setListSize (currentPos, SmallWidget, ParentHeight);
	Parents.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
		currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	Parents.recalcColumn ();
	getNextPos (currentPos);

	// Create the type combo
	setStaticSize (currentPos);
	LabelStruct.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
		currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	getNextPosLabel (currentPos);

	setListSize (currentPos, Width, DfnHeight + AdjusteHeight);
	Struct.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
		currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	Struct.recalcColumn ();
	getNextPos (currentPos);
	
	// Resize the current view
	View->setViewSize (
		virtualWidth,
		currentPos.bottom+CGeorgesEditView::WidgetTopMargin+CGeorgesEditView::WidgetBottomMargin);
}

// ***************************************************************************
// CDfnParentEditListCtrl
// ***************************************************************************

CEditListCtrl::TItemEdit CDfnParentEditListCtrl::getItemEditMode (uint item, uint subItem)
{
	return CEditListCtrl::EditMemCombo;
}

// ***************************************************************************

void CDfnParentEditListCtrl::getMemComboBoxProp (uint item, uint subItem, std::string &regAdr, bool &browse)
{
	browse = true;
	regAdr = tStrToUtf8(GEORGES_EDIT_BASE_REG_KEY _T("\\Dfn MemCombo"));
}

// ***************************************************************************

void CDfnParentEditListCtrl::getNewItemText (uint item, uint subItem, std::string &ret)
{
	ret = theApp.DefaultDfn;
}

// ***************************************************************************

void CDfnParentEditListCtrl::getBrowseInfo (uint item, uint subItem, std::string &defExt, std::string &defFilename, std::string &defDir, NLMISC::tstring &filter)
{
	filter = DfnFilter;
	defDir = theApp.RootSearchPath;
	defFilename = "*.dfn";
	defExt = "*.dfn";
}
