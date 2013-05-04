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
	LabelParents.Create ("Parents:", WS_VISIBLE, currentPos, this);
	initWidget (LabelParents);
	getNextPosLabel (currentPos);

	setListSize (currentPos, SmallWidget, ParentHeight);
	Parents.create (WS_TABSTOP, currentPos, this, LtParents);
	Parents.insertColumn (0, "Parent Dfn");
	Parents.Dialog = this;
	Parents.recalcColumn ();
	initWidget (Parents);
	getNextPos (currentPos);


	// Create the type combo
	setStaticSize (currentPos);
	LabelStruct.Create ("Structure:", WS_VISIBLE, currentPos, this);
	initWidget (LabelStruct);
	getNextPosLabel (currentPos);

	setListSize (currentPos, Width, DfnHeight);
	Struct.create (WS_TABSTOP, currentPos, this, LtStruct);
	Struct.insertColumn (0, "Name");
	Struct.insertColumn (1, "Type");
	Struct.insertColumn (2, "Value");
	Struct.insertColumn (3, "Default");
	Struct.insertColumn (4, "FilenameExt");
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
			Parents.ListCtrl.InsertItem (parent, dfn.getParentFilename (parent).c_str ());
		}

		// Add the struct element
		Struct.ListCtrl.DeleteAllItems ();
		uint elm;
		for (elm=0; elm<dfn.getNumEntry (); elm++)
		{
			// Add the label and value
			Struct.ListCtrl.InsertItem (elm, dfn.getEntry (elm).getName ().c_str());
			switch (elm, dfn.getEntry (elm).getType ())
			{
			case UFormDfn::EntryType:
				Struct.ListCtrl.SetItemText (elm, 1, dfn.getEntry (elm).getArrayFlag () ? "Type array" : "Type");
				Struct.ListCtrl.SetItemText (elm, 4, dfn.getEntry (elm).getFilenameExt ().c_str());
				break;
			case UFormDfn::EntryDfn:
				Struct.ListCtrl.SetItemText (elm, 1, dfn.getEntry (elm).getArrayFlag () ? "Dfn array" : "Dfn");
				break;
			case UFormDfn::EntryVirtualDfn:
				Struct.ListCtrl.SetItemText (elm, 1, "Virtual Dfn");
				break;
			}
			Struct.ListCtrl.SetItemText (elm, 2, dfn.getEntry (elm).getFilename ().c_str());
			Struct.ListCtrl.SetItemText (elm, 3, dfn.getEntry (elm).getDefault ().c_str());
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
			vectValue[parent] = str;
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
				stringVector[elm][subElm] = (const char*)name;
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
		string type = ListCtrl.GetItemText (item, 1);
		if (type != "Virtual Dfn")
			return CEditListCtrl::EditMemCombo;
	}
	else if (subItem == 3)
	{
		string type = ListCtrl.GetItemText (item, 1);
		if ((type == "Type") || (type == "Type array"))
			return CEditListCtrl::EditMemCombo;
	}
	else if (subItem == 4)
	{
		string type = ListCtrl.GetItemText (item, 1);
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
		regAdr = GEORGES_EDIT_BASE_REG_KEY"\\Label MemCombo";
	}
	else if (subItem == 2)
	{
		browse = true;

		// Get type string
		string type = ListCtrl.GetItemText (item, 1);
		if ((type == "Type") || (type == "Type array"))
			regAdr = GEORGES_EDIT_BASE_REG_KEY"\\Type MemCombo";
		else if ((type == "Dfn") || (type == "Dfn array"))
			regAdr = GEORGES_EDIT_BASE_REG_KEY"\\Dfn MemCombo";
	}
	else if (subItem == 3)
	{
		browse = false;
		regAdr = GEORGES_EDIT_BASE_REG_KEY"\\Default MemCombo";
	}
	else if (subItem == 3)
	{
		browse = false;
		regAdr = GEORGES_EDIT_BASE_REG_KEY"\\FilenameExt MemCombo";
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
		ret = "";
	else if (subItem == 4)
		ret = "";
}

// ***************************************************************************

void CDfnEditListCtrl::getBrowseInfo (uint item, uint subItem, std::string &defExt, std::string &defFilename, std::string &defDir, std::string &filter)
{
	if (subItem == 2)
	{
		// Get type string
		string type = ListCtrl.GetItemText (item, 1);
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
		string type = ListCtrl.GetItemText (item, 1);
		if ((type == "Type") || (type == "Type array"))
		{
			CString str;
			str = Dialog->Struct.ListCtrl.GetItemText (item, 2);
			char ext[MAX_PATH];
			_splitpath (str, NULL, NULL, NULL, ext);
			if (stricmp (ext, ".typ") != 0)
				Dialog->Struct.ListCtrl.SetItemText (item, 2, theApp.DefaultType.c_str ());
		}
		else if ((type == "Dfn") || (type == "Dfn array"))
		{
			CString str;
			str = Dialog->Struct.ListCtrl.GetItemText (item, 2);
			char ext[MAX_PATH];
			_splitpath (str, NULL, NULL, NULL, ext);
			if (stricmp (ext, ".dfn") != 0)
				Dialog->Struct.ListCtrl.SetItemText (item, 2, theApp.DefaultDfn.c_str ());

			// Clear default value
			Dialog->Struct.ListCtrl.SetItemText (item, 3, "");
		}
		else if (type == "Virtual Dfn")
		{
			// Clear the value
			Dialog->Struct.ListCtrl.SetItemText (item, 2, "");

			// Clear default value
			Dialog->Struct.ListCtrl.SetItemText (item, 3, "");
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
						string name = CPath::lookup ((const char*)str, false, false);
						if (name.empty ())
							name = str;

						// Open the file
						theApp.OpenDocumentFile (name.c_str ());
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
						string name = CPath::lookup ((const char*)str, false, false);
						if (name.empty ())
							name = str;

						// Open the file
						theApp.OpenDocumentFile (name.c_str ());
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
	regAdr = GEORGES_EDIT_BASE_REG_KEY"\\Dfn MemCombo";
}

// ***************************************************************************

void CDfnParentEditListCtrl::getNewItemText (uint item, uint subItem, std::string &ret)
{
	ret = theApp.DefaultDfn;
}

// ***************************************************************************

void CDfnParentEditListCtrl::getBrowseInfo (uint item, uint subItem, std::string &defExt, std::string &defFilename, std::string &defDir, std::string &filter)
{
	filter = DfnFilter;
	defDir = theApp.RootSearchPath;
	defFilename = "*.dfn";
	defExt = "*.dfn";
}
