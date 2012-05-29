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

// form_dialog.cpp : implementation file
//

#include "stdafx.h"
#include "georges_edit.h"
#include "georges_edit_view.h"
#include "georges_edit_doc.h"
#include "form_dialog.h"
#include "action.h"
#include "left_view.h"

#include "nel/misc/path.h"
#include "nel/georges/type.h"
#include "nel/georges/form_elm.h"

using namespace std;
using namespace NLMISC;
using namespace NLGEORGES;


// Define this to show Formula value evaluation intstead of value evaluation
// #define TEST_EVAL_FORMULA

// ***************************************************************************
// CFormDialog dialog
// ***************************************************************************

CFormDialog::CFormDialog () : CBaseDialog (IDR_MAINFRAME)
{
	//{{AFX_DATA_INIT(CFormDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	View = NULL;
	WidgetIndexCount = 0;
	WidgetFocused = 0xffffffff;
}

// ***************************************************************************

CFormDialog::~CFormDialog ()
{
	clear ();
}

// ***************************************************************************

void CFormDialog::clear ()
{
	unRegisterLastControl ();
	for (uint i=0; i<Widgets.size (); i++)
		delete Widgets[i];
	Widgets.clear ();
	WidgetFocused = 0xffffffff;
	WidgetIndexCount = 0;
}

// ***************************************************************************

void CFormDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFormDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

// ***************************************************************************

BEGIN_MESSAGE_MAP(CFormDialog, CDialog)
	//{{AFX_MSG_MAP(CFormDialog)
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ***************************************************************************
// CFormDialog message handlers

// ***************************************************************************

void CFormDialog::OnSize(UINT nType, int cx, int cy) 
{
	CBaseDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
}

// ***************************************************************************

BOOL CFormDialog::OnInitDialog() 
{
	CBaseDialog::OnInitDialog();

	SetDefID ( 0xffffffff );

	UpdateData (FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ***************************************************************************

void CFormDialog::OnOK ()
{
	for (uint i=0; i<Widgets.size(); i++)
	{
		Widgets[i]->updateData ();
		if (Widgets[i]->haveFocus ())
		{
			Widgets[i]->onOk ();
		}
	}
}

// ***************************************************************************

void CFormDialog::OnCancel ()
{
	for (uint i=0; i<Widgets.size(); i++)
	{
		Widgets[i]->updateData ();
		if (Widgets[i]->haveFocus ())
		{
			Widgets[i]->onCancel ();
			return;
		}
	}
	CBaseDialog::OnCancel ();
}

// ***************************************************************************

CWnd* CFormDialog::addTypeWidget (const NLGEORGES::CType &type, uint elmIndex, const char *title, const char *atomName, const char *typeFilename, RECT &currentPos, CForm &form, IFormWidget::TTypeSrc typeWidget, const char *filenameExt, uint slot)
{
	// What kind of UI ?
	switch (type.UIType)
	{
	case CType::FileBrowser:
	case CType::Edit:
	case CType::EditSpin:
		{
			// For edit type, use a memory combobox
			CFormMemCombo *memCombo = new CFormMemCombo (this, elmIndex, atomName, typeWidget, slot);
			Widgets.push_back (memCombo);

			// Create a reg key
			string tfn = typeFilename;
			string key = GEORGES_EDIT_BASE_REG_KEY"\\"+strlwr (typeFilename)+" MemCombo";

			// Create the widget
			memCombo->create (WS_CHILD|WS_TABSTOP, currentPos, this, WidgetIndexCount, title, key.c_str(), type.UIType==CType::EditSpin, type.UIType==CType::FileBrowser, filenameExt);

			// Get from document
			memCombo->getFromDocument (form);

			return &memCombo->Combo;
		}
		break;
	case CType::NonEditableCombo:
		{
			// For edit type, use a memory combobox
			CFormCombo *memCombo = new CFormCombo (this, elmIndex, atomName, typeWidget, slot);
			Widgets.push_back (memCombo);

			// Create the widget
			memCombo->create (WS_CHILD|WS_TABSTOP, currentPos, this, WidgetIndexCount, title);

			// Get from document
			memCombo->getFromDocument (form);

			return &memCombo->Combo;
		}
		break;
	case CType::BigEdit:
		{
			// For edit type, use a memory combobox
			CFormBigEdit *memCombo = new CFormBigEdit (this, elmIndex, atomName, typeWidget, slot);
			Widgets.push_back (memCombo);

			// Create the widget
			memCombo->create (WS_CHILD|WS_TABSTOP, currentPos, this, WidgetIndexCount, title);

			// Get from document
			memCombo->getFromDocument (form);

			return &memCombo->Edit;
		}
		break;
	case CType::ColorEdit:
		{
			// For edit type, use a memory combobox
			CColorEdit *memCombo = new CColorEdit (this, elmIndex, atomName, typeWidget, slot);
			Widgets.push_back (memCombo);

			// Create the widget
			memCombo->create (WS_CHILD|WS_TABSTOP, currentPos, this, WidgetIndexCount, title);

			// Get from document
			memCombo->getFromDocument (form);

			return &memCombo->Color;
		}
		break;
	}

	return NULL;
}

// ***************************************************************************

void CFormDialog::getVirtualDfnFromDocument (const NLGEORGES::CFormDfn *_dfn, const char *structName, uint slot)
{
	if (View)
	{
		CGeorgesEditDoc *doc = View->GetDocument ();
		if (doc)
		{
			// Clear the current dialog
			clear ();

			// Reserve some widget pointers
			Widgets.reserve (10);

			// Widget placement
			RECT currentPos;
			getFirstItemPos (currentPos);

			// For edit type, use a memory combobox
			CFormMemCombo *memCombo = new CFormMemCombo (this, 0xffffffff, structName, IFormWidget::TypeVirtualDfn, slot);
			Widgets.push_back (memCombo);

			// Create the widget
			memCombo->create (WS_CHILD|WS_TABSTOP, currentPos, this, WidgetIndexCount, "Dfn:", 
				GEORGES_EDIT_BASE_REG_KEY"\\Virtual Dfn MemCombo", false, true, "*.dfn");

			// Get from document
			memCombo->getFromDocument (*doc->getFormPtr ());

			// Dfn selected ?
			if (_dfn)
			{
				// Get the parent DFN
				std::vector<const CFormDfn*> arrayDfn;
				arrayDfn.reserve (_dfn->countParentDfn ());
				_dfn->getParentDfn (arrayDfn);

				// Element index
				uint elmIndex = 0;

				// For each DFN
				for (uint dfnIndex=0; dfnIndex<arrayDfn.size (); dfnIndex++)
				{
					// Form must be editable only if all DFN and TYPE have been found
					nlassert (arrayDfn[dfnIndex]);

					// Ref on the DFN
					const CFormDfn &dfn = *arrayDfn[dfnIndex];

					// For each structure element
					for (uint i=0; i<dfn.getNumEntry (); i++)
					{
						// Get a ref on the entry
						const CFormDfn::CEntry &entry = dfn.getEntry (i);

						// Is it an atom ?
						if (entry.getType () == UFormDfn::EntryType && !entry.getArrayFlag ())
						{
							// Some string
							string title = entry.getName() + ":";
							string atomName = string (structName)+"."+entry.getName();
							addTypeWidget (*entry.getTypePtr(), elmIndex, title.c_str (), atomName.c_str(), 
											entry.getFilename ().c_str (), currentPos, *doc->getFormPtr (), IFormWidget::TypeForm,
											entry.getFilenameExt ().c_str (), slot);
						}

						// Next form index
						elmIndex++;
					}
				}
			}

			// Register last control for tab selection
			registerLastControl ();
		}
	}
}

// ***************************************************************************
void CFormDialog::getDfnFromDocument (const NLGEORGES::CFormDfn &_dfn, const char *structName, uint slot)
{
	if (View)
	{
		CGeorgesEditDoc *doc = View->GetDocument ();
		if (doc)
		{
			// Clear the current dialog
			clear ();

			// Reserve some widget pointers
			Widgets.reserve (10);

			// Widget placement
			RECT currentPos;
			getFirstItemPos (currentPos);

			// Get the parent DFN
			std::vector<const CFormDfn*> arrayDfn;
			arrayDfn.reserve (_dfn.countParentDfn ());
			_dfn.getParentDfn (arrayDfn);

			// For edit type, use a memory combobox
			if (strcmp (structName, "") == 0)
			{
				CListWidget *listWidget = new CListWidget (this, 0xffffffff, "", IFormWidget::TypeFormParent, slot);
				Widgets.push_back (listWidget);

				// Create the widget
				listWidget->create (WS_CHILD|WS_TABSTOP, currentPos, this, WidgetIndexCount, "Parent Form:", 
					GEORGES_EDIT_BASE_REG_KEY"\\Parent Form MemCombo", 1);

				// Add one column
				listWidget->addColumn ("Parent filename");

				// Get from document
				listWidget->getFromDocument (*doc->getFormPtr ());
			}

			// store icon related infos
			CWnd *pWnd					= NULL;
			CWnd *pWndIcon				= NULL;
			CWnd *pWndIconColor			= NULL;
			CWnd *pWndIconBack			= NULL;
			CWnd *pWndIconBackColor		= NULL;
			CWnd *pWndIconOver			= NULL;
			CWnd *pWndIconOverColor		= NULL;
			CWnd *pWndIconOver2			= NULL;
			CWnd *pWndIconOver2Color	= NULL;

			// Element index
			uint elmIndex = 0;

			// For each DFN
			for (uint dfnIndex=0; dfnIndex<arrayDfn.size (); dfnIndex++)
			{
				// Form must be editable only if all DFN and TYPE have been found
				nlassert (arrayDfn[dfnIndex]);

				// Ref on the DFN
				const CFormDfn &dfn = *arrayDfn[dfnIndex];

				// For each structure element
				for (uint i=0; i<dfn.getNumEntry (); i++)
				{
					// Get a ref on the entry
					const CFormDfn::CEntry &entry = dfn.getEntry (i);

					// Is it an atom ?
					if (entry.getType () == UFormDfn::EntryType && !entry.getArrayFlag ())
					{
						// Some string
						string title = entry.getName() + ":";
						string atomName = string (structName)+"."+entry.getName();
						pWnd = addTypeWidget (*entry.getTypePtr(), elmIndex, title.c_str (), atomName.c_str(), 
											   entry.getFilename ().c_str (), currentPos, *doc->getFormPtr (), IFormWidget::TypeForm, 
											   entry.getFilenameExt ().c_str (), slot);

						if (entry.getName() == "Icon" || entry.getName() == "icon")
							pWndIcon = pWnd;
						else if (entry.getName() == "IconColor")
							pWndIconColor = pWnd;
						else if (entry.getName() == "IconBack" || entry.getName() == "icon background")
							pWndIconBack = pWnd;
						else if (entry.getName() == "IconBackColor")
							pWndIconBackColor = pWnd;
						else if (entry.getName() == "IconOver" || entry.getName() == "icon overlay")
							pWndIconOver = pWnd;
						else if (entry.getName() == "IconOverColor")
							pWndIconOverColor = pWnd;
						else if (entry.getName() == "IconOver2" || entry.getName() == "icon overlay2")
							pWndIconOver2 = pWnd;
						else if (entry.getName() == "IconOver2Color")
							pWndIconOver2Color = pWnd;
					}

					// Next form index
					elmIndex++;
				}
			}

			// Special case for ".Client" and ".3d" : add a widget to draw icons
			if ((string(structName) == ".Client") || (string(structName) == ".3d"))
			{
				string title = "Icon bitmap:";
				
				CIconWidget *w = new CIconWidget (this, elmIndex, "", IFormWidget::TypeFormParent, slot);
				Widgets.push_back (w);

				// Create the widget
				w->create (WS_CHILD|WS_TABSTOP, currentPos, this, WidgetIndexCount, title.c_str());

				//
				w->Icon.pWndIcon			= pWndIcon;
				w->Icon.pWndIconColor		= pWndIconColor;

				w->Icon.pWndIconBack		= pWndIconBack;
				w->Icon.pWndIconBackColor	= pWndIconBackColor;

				w->Icon.pWndIconOver		= pWndIconOver;
				w->Icon.pWndIconOverColor	= pWndIconOverColor;
				
				w->Icon.pWndIconOver2		= pWndIconOver2;
				w->Icon.pWndIconOver2Color	= pWndIconOver2Color;
			}

			// Register last control for tab selection
			registerLastControl ();
		}
	}
}

// ***************************************************************************

void CFormDialog::getArrayFromDocument (const char *structName, uint structId, uint slot)
{
	if (View)
	{
		CGeorgesEditDoc *doc = View->GetDocument ();
		if (doc)
		{
			// Clear the current dialog
			clear ();

			// Reserve some widget pointers
			Widgets.reserve (10);

			// Widget placement
			RECT currentPos;
			getFirstItemPos (currentPos);

			CFormMemCombo *memCombo = new CFormMemCombo (this, structId, structName, IFormWidget::TypeArray, slot);
			Widgets.push_back (memCombo);

			// Create the widget
			memCombo->create (WS_CHILD|WS_TABSTOP, currentPos, this, WidgetIndexCount, "Array size:", 
				GEORGES_EDIT_BASE_REG_KEY"\\Array Size MemCombo", true, false, NULL);

			// Get from document
			memCombo->getFromDocument (*doc->getFormPtr ());

			// Register last control for tab selection
			registerLastControl ();
		}
	}
}

// ***************************************************************************

void CFormDialog::getTypeFromDocument (const NLGEORGES::CType &_type, const char *name, const char *typeFilename, const char *structName, uint slot)
{
	if (View)
	{
		CGeorgesEditDoc *doc = View->GetDocument ();
		if (doc)
		{
			// Clear the current dialog
			clear ();

			// Reserve some widget pointers
			Widgets.reserve (10);

			// Widget placement
			RECT currentPos;
			getFirstItemPos (currentPos);

			// Some string
			addTypeWidget (_type, 0xffffffff, name, structName, typeFilename, currentPos, *doc->getFormPtr (), IFormWidget::TypeType, NULL, slot);

			// Register last control for tab selection
			registerLastControl ();
		}
	}
}

// ***************************************************************************

void CFormDialog::updateLabels ()
{
	for (uint i=0; i<Widgets.size (); i++)
	{
		Widgets[i]->updateLabel ();
	}
}

// ***************************************************************************

void CFormDialog::updateValues  ()
{
	for (uint i=0; i<Widgets.size (); i++)
	{
		Widgets[i]->updateLabel ();
		Widgets[i]->getFromDocument (*(View->GetDocument ()->getFormPtr ()));
	}
}

// ***************************************************************************

void CFormDialog::setToDocument (uint widget)
{
	CGeorgesEditDoc *doc = View->GetDocument ();
	if (doc)
	{
		// discard CIconWidget
		CIconWidget *iconWidget = dynamic_cast<CIconWidget*> (Widgets[widget]);
		if (iconWidget)
			return;

		// Check if this command will build a new array / virtual dfn that was inherited..
		if (Widgets[widget]->getFormName () != "NULL")
		{
			// Get current node
			const CFormDfn *parentDfn;
			uint indexDfn;
			const CFormDfn *nodeDfn;
			const CType *nodeType;
			CFormElm *node;
			UFormDfn::TEntryType type;
			bool array;
			bool parentVDfnArray;
			CForm *form=doc->getFormPtr ();
			CFormElm *elm = doc->getRootNode (Widgets[widget]->getSlot ());
			nlverify ( elm->getNodeByName (Widgets[widget]->getFormName ().c_str (), &parentDfn, indexDfn, 
				&nodeDfn, &nodeType, &node, type, array, parentVDfnArray, true, NLGEORGES_FIRST_ROUND) );

			// Must create array or virtual dfn ?
			if (parentVDfnArray)
			{
				// Warn the user
				if (!theApp.yesNo ("Warning, this action will create an array/virtual dfn over an inherited array/virtual dfn.\nDo you want to continue ?"))
					// Quit
					return;
			}
		}

		// Get the widget type
		IFormWidget::TTypeSrc typeSrc = Widgets[widget]->getSrcType ();
		if (typeSrc == IFormWidget::TypeForm)
		{
			// Get the value
			std::string result;
			Widgets[widget]->getValue (result);

			// Document modified
			doc->modify (new CActionString (IAction::FormValue, result.c_str (), *doc, Widgets[widget]->getFormName ().c_str (), "",
				doc->getLeftView ()->getCurrentSelectionId (), Widgets[widget]->getSlot ()));
		}
		else if (typeSrc == IFormWidget::TypeFormParent)
		{
			// The form
			CForm *form = doc->getFormPtr ();

			// Build an array of strings
			uint count = Widgets[widget]->getNumValue ();
			vector<string> stringVector (count);
			for (uint value = 0; value<count; value++)
			{
				// Get the result
				Widgets[widget]->getValue (stringVector[value], value);
			}

			// Modify document
			doc->modify (new CActionStringVector (IAction::FormParents, stringVector, *doc, "", 
				doc->getLeftView ()->getCurrentSelectionId (), Widgets[widget]->getSlot ()));
		}	
		else if (typeSrc == IFormWidget::TypeArray)
		{
			// Get the value
			std::string result;
			Widgets[widget]->getValue (result);

			// Modify document
			doc->modify (new CActionBuffer (IAction::FormArraySize, NULL, 0, *doc, Widgets[widget]->getFormName ().c_str(), 
				result.c_str (), doc->getLeftView ()->getCurrentSelectionId (), Widgets[widget]->getSlot ()));
		}
		else if (typeSrc == IFormWidget::TypeType)
		{
			// Get the result value
			std::string result;
			Widgets[widget]->getValue (result);

			// Document is modified by this view
			doc->modify (new CActionString (IAction::FormTypeValue, result.c_str (), *doc, Widgets[widget]->getFormName().c_str(), "",
				doc->getLeftView ()->getCurrentSelectionId (), Widgets[widget]->getSlot ()));
		}
		else if (typeSrc == IFormWidget::TypeVirtualDfn)
		{
			// Get the value
			std::string result;
			Widgets[widget]->getValue (result);

			// Modify the document
			doc->modify (new CActionBuffer (IAction::FormVirtualDfnName, NULL, 0, *doc, Widgets[widget]->getFormName ().c_str (), 
				result.c_str (), doc->getLeftView ()->getCurrentSelectionId (), Widgets[widget]->getSlot ()));
		}
	}

	// Notify the plugin that the value has changed
	if (!Widgets[widget]->getFormName ().empty ())
		doc->notifyPlugins (Widgets[widget]->getFormName ().c_str ());
}

// ***************************************************************************

LRESULT CFormDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message)
	{
	case CL_CHANGED:
		{
			uint widgetId = getWidget (wParam);
			CColorEdit *colorEdit = safe_cast<CColorEdit*> (Widgets[widgetId]);
			colorEdit->Empty = false;
			setToDocument (getWidget (wParam));
		}
		break;
	case MC_STRINGCHANGE:
		{
			setToDocument (getWidget (wParam));
		}
		break;

	case CBN_CHANGED:
		{
			for (uint i=0 ; i<Widgets.size() ; i++)
			{
				CIconWidget *iconWidget = dynamic_cast<CIconWidget*> (Widgets[i]);
				if (iconWidget)
					iconWidget->Icon.Invalidate();
			}
		}
		break;
	}
	
	return CDialog::WindowProc(message, wParam, lParam);
}

// ***************************************************************************

void CFormDialog::onOpenSelected ()
{
	// Get the focus windows
	for (uint i=0; i<Widgets.size(); i++)
	{
		if (Widgets[i]->haveFocus ())
		{
			Widgets[i]->onOpenSelected ();
			/*CFormMemCombo *combo = dynamic_cast<CFormMemCombo*> (Widgets[i]);
			if (combo)
			{
				CString str;
				combo->Combo.GetWindowText (str);
				if (combo->Browse && (str != ""))
				{
					std::string str2=CPath::lookup ((const char*)str, false, false);
					if (str2.empty())
						str2 = (const char*)str;
					theApp.OpenDocumentFile (str2.c_str ());
				}
			}*/
		}
	}
}

// ***************************************************************************

CWnd* CFormDialog::GetNextDlgTabItem( CWnd* pWndCtl, BOOL bPrevious) const
{
	return NULL;
}

// ***************************************************************************

void CFormDialog::onFirstFocus ()
{
	View->SetFocus ();
	WidgetFocused = 0xffffffff;
}

// ***************************************************************************

void CFormDialog::onLastFocus ()
{
	View->setFocusLeftView ();
	WidgetFocused = 0xffffffff;
}

// ***************************************************************************

int CFormDialog::getWidget (uint dialogId) const
{
	for (uint i=0; i<Widgets.size(); i++)
	{
		if (Widgets[i]->isDialog (dialogId))
			return i;
	}
	return -1;
}

// ***************************************************************************

BOOL CFormDialog::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch (HIWORD(wParam))
	{
	case CBN_SETFOCUS:
		{
			onGetSubFocus (LOWORD(wParam));
		}
		return TRUE;
	case EN_SETFOCUS:
		{
			onGetSubFocus (LOWORD(wParam));
		}
		return TRUE;
	/*case CBN_SELENDOK:
		{
			// Look for the widget
			int widgetId = getWidget (LOWORD(wParam));
			if (widgetId != -1)
				setToDocument (widgetId);
		}
		return TRUE;*/
	case CBN_SELCHANGE:
		{
			// Look for the widget
			int widgetId = getWidget (LOWORD(wParam));
			if (widgetId != -1)
				setToDocument (widgetId);
		}
		return TRUE;
	case EN_CHANGE:
		{
			// Look for the widget
			int widgetId = getWidget (LOWORD(wParam));
			if (widgetId != -1)
			{
				CColorEdit *colorEdit = dynamic_cast<CColorEdit*> (Widgets[widgetId]);
				if (colorEdit)
				{
					CString str;
					colorEdit->Edit.GetWindowText (str);

					sint r, g, b;
					if (sscanf (str, "%d,%d,%d", &r, &g, &b) == 3)
					{
						clamp (r, 0, 255);
						clamp (g, 0, 255);
						clamp (b, 0, 255);
						CRGBA color (r, g, b);
						colorEdit->Color.setColor (color);
						if (r != 255 && g != 255 && b != 255)
							colorEdit->Empty = false;
					}
				}
			}
		}
		return TRUE;
	case BN_CLICKED:
		{
			// Get the window
			int widgetId = getWidget (LOWORD(wParam));

			// Dialog Pointer
			if (widgetId != -1)
			{
				if ( (Widgets[widgetId]->getSrcType () == IFormWidget::TypeForm) || (Widgets[widgetId]->getSrcType () == IFormWidget::TypeType))
				{
					CFormMemCombo *combo = dynamic_cast<CFormMemCombo*> (Widgets[widgetId]);
					if (combo && IsWindow (combo->Browse))
					{
						CGeorgesEditDoc *doc = View->GetDocument ();
						if (doc)
						{
							// Get current node
							const CFormDfn *parentDfn;
							uint indexDfn;
							const CFormDfn *nodeDfn;
							const CType *nodeType;
							CFormElm *node;
							UFormDfn::TEntryType type;
							bool array;
							bool parentVDfnArray;
							CForm *form=doc->getFormPtr ();
							CFormElm *elm = doc->getRootNode (Widgets[widgetId]->getSlot ());
							nlverify ( elm->getNodeByName (Widgets[widgetId]->getFormName ().c_str (), &parentDfn, indexDfn, 
								&nodeDfn, &nodeType, &node, type, array, parentVDfnArray, true, NLGEORGES_FIRST_ROUND) );
							nlassert (parentDfn);

							// Get the current filename extension
							string ext = parentDfn->getEntry (indexDfn).getFilenameExt ();

							// Build a nice type name
							char typeName[512];
							smprintf (typeName, 512, "%s", strlwr (ext).c_str());
							uint i=0;
							while ((typeName[i] == '.') || (typeName[i] == '*'))
								i++;
							if (typeName[i])
								typeName[i] = toupper (typeName[i]);

							// Biuld the filter string
							char filter[512];
							smprintf (filter, 512, "%s Files (%s)|%s|All Files(*.*)|*.*|", typeName+i, ext.c_str(), ext.c_str());

							// Open the dialog
							CFileDialog dlgFile (TRUE, ext.c_str (), ext.c_str (), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter, theApp.m_pMainWnd);
							if (dlgFile.DoModal () == IDOK)
							{
								combo->Combo.UpdateData ();
								combo->Combo.SetWindowText (dlgFile.GetFileName ());
								combo->Combo.UpdateData (FALSE);
								setToDocument (widgetId);
								PostMessage (CBN_CHANGED, 0, 0);
							}
						}
					}
					else
					{
						// Reset button of color edit ?
						CColorEdit *colorEdit = dynamic_cast<CColorEdit*> (Widgets[widgetId]);
						if (colorEdit && IsWindow (colorEdit->Color))
						{
							colorEdit->Empty = true;
							colorEdit->Edit.SetWindowText("");
							setToDocument (getWidget (wParam));
							updateValues ();
						}
					}
				}
				else if (Widgets[widgetId]->getSrcType () == IFormWidget::TypeVirtualDfn)
				{
					CFormMemCombo *combo = dynamic_cast<CFormMemCombo*> (Widgets[widgetId]);
					if (combo && IsWindow (combo->Browse))
					{
						CGeorgesEditDoc *doc = View->GetDocument ();
						if (doc)
						{
							// Build the filter string
							char filter[512];
							smprintf (filter, 512, "Dfn Files (*.dfn)|*.dfn|All Files(*.*)|*.*|");

							// Open the dialog
							CFileDialog dlgFile (TRUE, "*.dfn", "*.dfn", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter, theApp.m_pMainWnd);
							if (dlgFile.DoModal () == IDOK)
							{
								combo->Combo.UpdateData ();
								combo->Combo.SetWindowText (dlgFile.GetFileName ());
								combo->Combo.UpdateData (FALSE);
								setToDocument (widgetId);
							}
						}
					}
				}
				else if (Widgets[widgetId]->getSrcType () == IFormWidget::TypeFormParent)
				{
					setToDocument (widgetId);
				}
			}
		}
		return TRUE;
	}
	
	return CWnd::OnCommand(wParam, lParam);
}

// ***************************************************************************

BOOL CFormDialog::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	LPNMHDR pnmh = (LPNMHDR) lParam;

	// Get the CTRL ID
	int idCtrl = (int) wParam;

	switch (pnmh->code)
	{
		case NM_SETFOCUS:
		{
			onGetSubFocus (idCtrl);
		}
		break;
		// Spinner control
		case UDN_DELTAPOS:
		{
			// Get the window
			for (uint i=0; i<Widgets.size(); i++)
			{
				if (Widgets[i]->isDialog (idCtrl))
				{
					// Get the node type
					Widgets[i]->getFormName ();
						
					// Get the structure
					LPNMUPDOWN lpnmud = (LPNMUPDOWN) lParam;

					// Get a good pointer
					CFormMemCombo *combo = (CFormMemCombo*)Widgets[i];

					// Get the widget value
					float value;
					CString str;
					combo->Combo.UpdateData ();
					combo->Combo.GetWindowText (str);
					if (sscanf (str, "%f", &value) == 1)
					{
						CGeorgesEditDoc *doc = View->GetDocument();
						if (doc)
						{
							// Get the node
							const CFormDfn *parentDfn;
							const CFormDfn *nodeDfn;
							const CType *nodeType;
							CFormElm *node;
							uint lastElement;
							bool array;
							bool parentVDfnArray;
							UFormDfn::TEntryType type;

							// Search for the node
							nlverify ((const CFormElm*)(doc->getRootNode (Widgets[i]->getSlot ()))->getNodeByName 
								(Widgets[i]->getFormName ().c_str (), &parentDfn, lastElement, &nodeDfn, &nodeType, 
								&node, type, array, parentVDfnArray, true, NLGEORGES_FIRST_ROUND));

							// Todo: multiply here by the spinner precision
							float increment = 1;
							if (nodeType)
								sscanf (nodeType->Increment.c_str (), "%f", &increment);

							value -= (float)(lpnmud->iDelta) * increment;

							// Print the result
							char result[512];
							sprintf (result, "%g", value);
							
							// Set the windnow text
							combo->Combo.SetWindowText (result);
							combo->Combo.UpdateData (FALSE);

							// Update the widget
							setToDocument (i);
						}
					}
					break;
				}
			}
		}
		break;
	}
	
	return CDialog::OnNotify(wParam, lParam, pResult);
}

// ***************************************************************************

void CFormDialog::resizeWidgets ()
{
	if (Widgets.size ())
	{
		RECT viewRect;
		View->GetClientRect (&viewRect);
		uint virtualWidth = std::max ((uint)MinViewWidth, (uint)(viewRect.right-viewRect.left));

		// Refresh sizes
		CBaseDialog::resizeWidgets (virtualWidth, 0);

		// Get first item coordinate
		RECT currentPos;
		getFirstItemPos (currentPos);
		
		// For each widgets
		uint bigWidgetCount[2] = {0, 0};
		uint i;
		uint biggestBottom[2] = {0, 0};
		uint nextSplit = Widgets.size ()/2;
		for (i=0; i<Widgets.size (); i++)
		{
			uint column = (i>nextSplit) ? 1:0;

			Widgets[i]->resizeScan (currentPos, bigWidgetCount[column], 0, false);

			if (currentPos.bottom > (int)biggestBottom[column])
			{
				biggestBottom[column] = currentPos.bottom;
			}
			
			if (nextSplit == i)
			{
				getNextColumn (currentPos);
				currentPos.top = 0;
			}
		}

		// Refresh sizes
		uint adjust[2];
		CBaseDialog::resizeWidgets (virtualWidth, biggestBottom[0]);
		adjust[0] = AdjusteHeight;
		CBaseDialog::resizeWidgets (virtualWidth, biggestBottom[1]);
		adjust[1] = AdjusteHeight;

		// Get first item coordinate
		currentPos;
		getFirstItemPos (currentPos);

		uint adjustSum[2] = { 
			bigWidgetCount[0] ? adjust[0] / bigWidgetCount[0] : 0,
				bigWidgetCount[1] ? adjust[1] / bigWidgetCount[1] : 0 };
		biggestBottom[0] = 0;
		biggestBottom[1] = 0;
		for (i=0; i<Widgets.size () - 1; i++)
		{
			uint column = (i>nextSplit) ? 1:0;

			if (Widgets[i]->extendableHeight ())
			{
				Widgets[i]->resizeScan (currentPos, bigWidgetCount[column], adjustSum[column], true);
				adjust[column] -= adjustSum[column];
			}
			else
				Widgets[i]->resizeScan (currentPos, bigWidgetCount[column], 0, true);

			if (currentPos.bottom > (int)biggestBottom[column])
			{
				biggestBottom[column] = currentPos.bottom;
			}
			
			if (nextSplit == i)
			{
				getNextColumn (currentPos);
				currentPos.top = 0;
			}
		}

		uint column = (i>nextSplit) ? 1:0;
		Widgets[i]->resizeScan (currentPos, bigWidgetCount[column], adjust[column], true);
	
		if (currentPos.bottom > (int)biggestBottom[column])
		{
			biggestBottom[column] = currentPos.bottom;
		}
			
		// Resize the current view
		View->setViewSize (virtualWidth, std::max (biggestBottom[0], biggestBottom[1])+CGeorgesEditView::WidgetTopMargin+CGeorgesEditView::WidgetBottomMargin);
	}
}

// ***************************************************************************

void CFormDialog::getFromDocument ()
{
	CGeorgesEditDoc *doc = View->GetDocument();
	if (doc)
	{
		// Save current focus
		uint widgetFocus;
		for (widgetFocus=0; widgetFocus<Widgets.size (); widgetFocus++)
		{
			if (Widgets[widgetFocus]->haveFocus ())
				break;
		}
		
		// Current selection
		CGeorgesEditDocSub *subObject = doc->getSelectedObject ();

		// Get the node
		const CFormDfn *parentDfn;
		const CFormDfn *nodeDfn;
		const CType *nodeType;
		CFormElm *node;
		uint lastElement;
		bool array;
		bool parentVDfnArray;
		UFormDfn::TEntryType type;

		// Search for the node
		nlverify (((const CFormElm*)(doc->getRootNode (subObject->getSlot ())))->getNodeByName (subObject->getFormName ().c_str (), &parentDfn, lastElement, &nodeDfn, &nodeType, &node, type, array, parentVDfnArray, true, NLGEORGES_FIRST_ROUND));

		// Should have a parent DFN, else it is the root element
		if (parentDfn)
		{
			// Is an array ?
			if ( array )
			{
				// Must be a dfn
				nlassert ( (type==UFormDfn::EntryDfn) || (type==UFormDfn::EntryType));
				getArrayFromDocument (subObject->getFormName ().c_str (), lastElement, subObject->getSlot ());
			}
			else
			{
				// Is a struct ?
				switch (parentDfn->getEntry (lastElement).getType ())
				{
				// Type
				case UFormDfn::EntryType:
					nlassert ( !array );
					nlassert ( nodeType );
					nlassert ( parentDfn );
					nlassert (type==UFormDfn::EntryType);
					getTypeFromDocument (*nodeType, 
													(parentDfn->getEntry(lastElement).getName()+":").c_str (), 
													parentDfn->getEntry(lastElement).getFilename().c_str(), 
													subObject->getFormName ().c_str (), subObject->getSlot ());
					break;
				// Dfn
				case UFormDfn::EntryDfn:
					nlassert ( !array );
					nlassert ((nodeDfn) && (type==UFormDfn::EntryDfn));
					getDfnFromDocument (*nodeDfn, subObject->getFormName ().c_str (), subObject->getSlot ());
					break;
				// Virtual Dfn
				case UFormDfn::EntryVirtualDfn:
					nlassert ( !array );
					getVirtualDfnFromDocument (nodeDfn, subObject->getFormName ().c_str (), subObject->getSlot ());
					break;
				}
			}
		}
		else
		{
			nlassert ( !array );
			nlassert ((nodeDfn) && (type==UFormDfn::EntryDfn));
			getDfnFromDocument (*nodeDfn, subObject->getFormName ().c_str (), subObject->getSlot ());
		}

		// Update labels
		for (uint i=0; i<Widgets.size (); i++)
		{
			// Update labels
			Widgets[i]->updateLabel ();
		}

		// Set the focus
		if (widgetFocus<Widgets.size ())
		{
			Widgets[widgetFocus]->setFocus ();
		}
		
		// Resize the widgets
		resizeWidgets ();
	}
}

// ***************************************************************************

void CFormDialog::getDfnName (string &result) const
{
	CGeorgesEditDoc *doc = View->GetDocument ();
	if (doc)
	{
		// Get the DFN filename
		CString str = doc->GetPathName ();
		char extension[512];
		_splitpath (str, NULL, NULL, NULL, extension);
		result = (*extension == '.') ? extension+1 : extension;
	}
	else
		result = "";
}

// ***************************************************************************

void CFormDialog::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// Get the focus
	View->TabCtrl.SetFocus ();
	
	CDialog::OnLButtonDown(nFlags, point);
}

// ***************************************************************************

void CFormDialog::onGetSubFocus (uint id)
{
	// Get the widget
	int widget = getWidget (id);
	WidgetFocused = widget;

	// Window view
	RECT widgetRect;
	if (Widgets[widget]->getWindowRect (widgetRect))
	{
		View->ScreenToClient (&widgetRect);

		// Scroll the view to be visible
		RECT viewRect;
		View->GetClientRect (&viewRect);
		int bottom = viewRect.bottom - viewRect.top;
		if (widgetRect.bottom > bottom)
		{
			CPoint pt = View->GetScrollPosition ();
			View->ScrollToPosition (CPoint (pt.x, pt.y + widgetRect.bottom - bottom + 10));
		}
		if (widgetRect.top < 0)
		{
			CPoint pt = View->GetScrollPosition ();
			View->ScrollToPosition (CPoint (pt.x, pt.y + widgetRect.top - 10));
		}
	}
}

// ***************************************************************************

void CFormDialog::OnSetFocus(CWnd* pNewWnd) 
{
	CDialog::OnSetFocus(pNewWnd);
	if (WidgetFocused != 0xffffffff)
		Widgets[WidgetFocused]->setFocus ();
}

// ***************************************************************************

void CFormDialog::OnKillFocus(CWnd* pNewWnd) 
{
	CDialog::OnKillFocus(pNewWnd);
}

// ***************************************************************************
// IFormWidget
// ***************************************************************************

IFormWidget::IFormWidget (CFormDialog *dialog, uint structId, const char *atomName, TTypeSrc typeSrc, uint slot) 
{ 
	FormName = atomName;
	Dialog = dialog; 
	StructId = structId;
	SrcType = typeSrc;
	Slot = slot;
}

// ***************************************************************************

bool IFormWidget::isDialog (uint id) const 
{ 
	return (id>=FirstId) && (id<=LastId); 
};

// ***************************************************************************

uint IFormWidget::getSlot () const 
{ 
	return Slot; 
};

// ***************************************************************************

uint IFormWidget::getStructId () const
{
	return StructId;
}

// ***************************************************************************

void IFormWidget::updateLabel ()
{
	// Does the node in the same form ?
	CGeorgesEditDoc *doc = Dialog->View->GetDocument ();
	if (doc)
	{
		// Is the label a window ?
		if (IsWindow (Label))
		{
			// No label for parent widget
			if (SrcType != TypeFormParent)
			{
				// Value type ?
				if ((SrcType == TypeForm) || (SrcType == TypeType))
				{
					// Get the value
					std::string result;
					UFormElm::TWhereIsValue where;
					CForm *form=doc->getFormPtr ();
					CFormElm *elm = doc->getRootNode (getSlot ());
					nlverify (elm->getValueByName (result, FormName.c_str (), UFormElm::NoEval, &where));

					// Get the value evaluated
					std::string resultEvaluated;
#ifdef TEST_EVAL_FORMULA
					bool error = !elm->getValueByName (resultEvaluated, FormName.c_str (), UFormElm::Formula, &where);
#else // TEST_EVAL_FORMULA
					bool error = !elm->getValueByName (resultEvaluated, FormName.c_str (), UFormElm::Eval, &where);
#endif // TEST_EVAL_FORMULA

					// Complete the array ?
					string comp;
					if (error)
						comp = " (Value = Error)";
					else
					{
						if (resultEvaluated != result)
							comp=" (Value = \""+resultEvaluated+"\")";
					}

					// Does the node exist ?
					switch (where)
					{
					case UFormElm::ValueForm:
						Label.SetWindowText ((SavedLabel+comp).c_str());
						break;
					case UFormElm::ValueParentForm:
						Label.SetWindowText ((SavedLabel+" (in parent form)"+comp).c_str());
						break;
					case UFormElm::ValueDefaultDfn:
						Label.SetWindowText ((SavedLabel+" (default DFN value)"+comp).c_str());
						break;
					case UFormElm::ValueDefaultType:
						Label.SetWindowText ((SavedLabel+" (default TYPE value)"+comp).c_str());
						break;
					}
				}
				else
				{
					// Get the node
					const CFormDfn *parentDfn;
					uint indexDfn;
					const CFormDfn *nodeDfn;
					const CType *nodeType;
					CFormElm *node;
					UFormDfn::TEntryType type;
					bool array;
					bool parentVDfnArray;
					CForm *form=doc->getFormPtr ();
					CFormElm *elm = doc->getRootNode (getSlot ());
					nlverify ( elm->getNodeByName (FormName.c_str (), &parentDfn, indexDfn, 
						&nodeDfn, &nodeType, &node, type, array, parentVDfnArray, true, NLGEORGES_FIRST_ROUND) );

					// Does the node exist ?
					if (node)
					{
						// Same form ?
						if (node->getForm () == doc->getFormPtr ())
						{
							// The node exist
							Label.SetWindowText (SavedLabel.c_str());
						}
						else
						{
							// The node exist in the parent form
							Label.SetWindowText ((SavedLabel+" (in parent form)").c_str());
						}
					}	
					else
					{
						// The node is empty
						Label.SetWindowText ((SavedLabel+" (undefined)").c_str());
					}
				}

				// Update widget
				Label.UpdateData (FALSE);
			}
		}
	}
}

// ***************************************************************************

IFormWidget::TTypeSrc IFormWidget::getSrcType () const
{
	return SrcType;
}

// ***************************************************************************

bool IFormWidget::extendableHeight () const
{
	return false;
}

// ***************************************************************************

bool IFormWidget::getNode (const CFormDfn **parentDfn, uint &lastElement, const CFormDfn **nodeDfn, const CType **nodeType, 
					CFormElm **node, UFormDfn::TEntryType &type, bool &array) const
{
	// Get the document
	CGeorgesEditDoc *doc = Dialog->View->GetDocument ();
	if (doc)
	{
		// Return the node
		bool parentVDfnArray;
		CForm *form=doc->getFormPtr ();
		CFormElm *elm = doc->getRootNode (getSlot ());
		return (elm->getNodeByName (FormName.c_str (), parentDfn, 
			lastElement, nodeDfn, nodeType, node, type, array, parentVDfnArray, true, NLGEORGES_FIRST_ROUND) );
	}
	return false;
}

// ***************************************************************************

CFormElm *IFormWidget::getFormElmNode () const
{
	const CFormDfn			*parentDfn;
	uint					parentDfnIndex;
	const CFormDfn			*nodeDfn;
	const CType				*nodeType;
	CFormElm				*node;
	UFormDfn::TEntryType	type;
	bool array;
	if (getNode (&parentDfn, parentDfnIndex, &nodeDfn, &nodeType, &node, type, array))
	{
		return node;
	}
	return NULL;
}

// ***************************************************************************

CFormElmStruct *IFormWidget::getFormElmStructNode () const
{
	CFormElm *elm = getFormElmNode ();
	return elm ? safe_cast<CFormElmStruct*> (elm) : NULL;
}

// ***************************************************************************

CFormElmVirtualStruct *IFormWidget::getFormElmVirtualStructNode () const
{
	CFormElm *elm = getFormElmNode ();
	return elm ? safe_cast<CFormElmVirtualStruct*> (elm) : NULL;
}

// ***************************************************************************

CFormElmArray *IFormWidget::getFormElmArrayNode () const
{
	CFormElm *elm = getFormElmNode ();
	return elm ? safe_cast<CFormElmArray*> (elm) : NULL;
}

// ***************************************************************************

CFormElmAtom *IFormWidget::getFormElmAtomNode () const
{
	CFormElm *elm = getFormElmNode ();
	return elm ? safe_cast<CFormElmAtom*> (elm) : NULL;
}

// ***************************************************************************

const string &IFormWidget::getFormName () const
{
	return FormName;
}

// ***************************************************************************

uint IFormWidget::getNumValue ()
{
	// Not implemented for this widget
	nlstop;
	return 0;
}

// ***************************************************************************

void IFormWidget::getValue (std::string &result)
{
	// Not implemented for this widget
	nlstop;
}

// ***************************************************************************

void IFormWidget::getValue (std::string &result, uint value)
{
	// Not implemented for this widget
	nlstop;
}

// ***************************************************************************

bool IFormWidget::getWindowRect (RECT &rect) const
{
	if (IsWindow (Label))
	{
		Label.GetWindowRect (&rect);
		return true;
	}
	else
		return false;
}

// ***************************************************************************

void IFormWidget::onOpenSelected ()
{
	string str;
	getValue (str);

	std::string str2=CPath::lookup (str.c_str (), false, false);
	if (str2.empty())
		str2 = str.c_str ();
	theApp.OpenDocumentFile (str2.c_str ());
}

// ***************************************************************************
// CFormMemCombo
// ***************************************************************************

CFormMemCombo::CFormMemCombo (CFormDialog *dialog, uint structId, const char *atomName, TTypeSrc typeSrc, uint slot) : IFormWidget (dialog, structId, atomName, typeSrc, slot)
{
	UseSpinner = false;
	FileBrowser = false;
}

// ***************************************************************************

CFormMemCombo::~CFormMemCombo ()
{
	if (IsWindow (Label))
		Label.DestroyWindow ();
	if (IsWindow (Combo))
		Combo.DestroyWindow ();
	if (IsWindow (Spin))
		Spin.DestroyWindow ();
	if (IsWindow (Browse))
		Browse.DestroyWindow ();
}

// ***************************************************************************

void CFormMemCombo::create (DWORD wStyle, RECT &currentPos, CFormDialog *parent, uint &dialog_index, const char *label, const char *reg, bool useSpinner, bool fileBrowser, const char *filenameExt)
{
	// Get the doc
	CGeorgesEditDoc *doc = Dialog->View->GetDocument ();
	nlassert (doc);

	// Get the node type (if any)
	const CFormDfn *parentDfn;
	uint indexDfn;
	const CFormDfn *nodeDfn;
	const CType *nodeType = NULL;
	CFormElm *node;
	UFormDfn::TEntryType type;
	bool array;
	bool parentVDfnArray;
	CForm *form=doc->getFormPtr ();
	CFormElm *elm = doc->getRootNode (getSlot ());
	nlverify ( elm->getNodeByName (FormName.c_str (), &parentDfn, indexDfn, 
		&nodeDfn, &nodeType, &node, type, array, parentVDfnArray, true, NLGEORGES_FIRST_ROUND) );

	FirstId = dialog_index;
	LastId = FirstId+1;

	// Save the label 
	SavedLabel = label;

	// Create the type combo
	parent->setStaticSize (currentPos);
	Label.Create (label, WS_VISIBLE, currentPos, parent);
	parent->initWidget (Label);
	parent->getNextPosLabel (currentPos);

	UseSpinner = useSpinner;
	FileBrowser = fileBrowser;

	// Create the spinner
	if (useSpinner)
	{
		// Create the mem combobox
		parent->setComboSpinSize (currentPos);
		Combo.create (WS_CHILD|WS_TABSTOP, currentPos, parent, dialog_index, reg, theApp.RememberListSize);
		parent->initWidget (Combo);

		// Create the spin
		RECT spinPos = currentPos;
		parent->getNextSpinPos (spinPos);
		parent->setSpinSize (spinPos);
		Spin.Create (WS_CHILD|WS_VISIBLE, spinPos, parent, dialog_index+1);
		//Spin.SetBuddy (&Combo.ComboBox);
		parent->getNextPos (currentPos);
	}
	else if (fileBrowser)
	{
		// Create the mem combobox
		parent->setComboBrowseSize (currentPos);
		Combo.create (WS_CHILD|WS_TABSTOP, currentPos, parent, dialog_index, reg, theApp.RememberListSize);
		parent->initWidget (Combo);

		// Create the spin
		RECT spinPos = currentPos;
		parent->getNextBrowsePos (spinPos);
		parent->setBrowseSize (spinPos);
		Browse.Create ("...", WS_CHILD|WS_VISIBLE|WS_TABSTOP, spinPos, parent, dialog_index+1);
		parent->initWidget (Browse);
		parent->getNextPos (currentPos);

		// Set autocomplete mode
		if (filenameExt)
		{
			if (strcmp (filenameExt, "*.*") != 0)
				Combo.enableAutoCompleteExtension (true, filenameExt);
		}
	}
	else
	{
		// Create the mem combobox
		parent->setComboSize (currentPos, parent->SmallWidget);
		Combo.create (WS_CHILD|WS_TABSTOP, currentPos, parent, dialog_index, reg, theApp.RememberListSize);
		parent->initWidget (Combo);
		parent->getNextPos (currentPos);
	}

	// Get predefs
	if (nodeType)
	{
		for (uint predef=0; predef<nodeType->Definitions.size(); predef++)
		{
			Combo.addStaticStrings (nodeType->Definitions[predef].Label.c_str());
		}
	}

	dialog_index += 2;
}

// ***************************************************************************

void CFormMemCombo::updateData (bool update)
{
	Combo.UpdateData (update?TRUE:FALSE);
}

// ***************************************************************************

bool CFormMemCombo::haveFocus ()
{
	return (Combo.haveFocus ());
}

// ***************************************************************************

void CFormMemCombo::setFocus ()
{
	Combo.SetFocus ();	
}

// ***************************************************************************

void CFormMemCombo::onOk ()
{
	Combo.onOK ();
}

// ***************************************************************************

void CFormMemCombo::getValue (std::string &result)
{
	Combo.UpdateData ();

	CString str;
	Combo.GetWindowText (str);

	Combo.UpdateData (FALSE);
	
	// Set the atom value
	result = (const char*)str;
}

// ***************************************************************************

void CFormMemCombo::getFromDocument (CForm &form)
{
	CGeorgesEditDoc *doc = Dialog->View->GetDocument ();
	nlassert (doc);

	if ((SrcType == TypeForm) || (SrcType == TypeType))
	{
		string result;
		if (doc->getRootNode(getSlot ())->getValueByName (result, FormName.c_str(), UFormElm::NoEval, NULL))
		{
			Combo.UpdateData ();
			Combo.SetWindowText (result.c_str());
			Combo.UpdateData (FALSE);
			updateLabel ();
		}
		else
		{
			nlstop;
		}
	}
	else if (SrcType == TypeArray)
	{
		const CFormDfn *parentDfn;
		uint lastElement;
		const CFormDfn *nodeDfn;
		const CType *nodeType;
		CFormElm *node;
		UFormDfn::TEntryType type;
		bool array;
		bool parentVDfnArray;
		nlverify (((const CFormElm*)doc->getRootNode(getSlot ()))->getNodeByName (FormName.c_str(), &parentDfn, lastElement, &nodeDfn, &nodeType, &node, type, array, parentVDfnArray, true, NLGEORGES_FIRST_ROUND));
		nlassert (array);

		// Node exist ?
		Combo.UpdateData ();
		if (node)
		{
			CFormElmArray *arrayNode = safe_cast<CFormElmArray*> (node);
			char label[512];
			smprintf (label, 512, "%d", arrayNode->Elements.size ());
			Combo.SetWindowText (label);

			if (arrayNode->getForm () == &form)
				Label.SetWindowText ("Array size:");
			else
				Label.SetWindowText ("Array size: (in parent form)");
		}
		else
		{
			Combo.SetWindowText ("0");
		}
		Combo.UpdateData (FALSE);
	}
	else if (SrcType == TypeVirtualDfn)
	{
		const CFormDfn *parentDfn;
		uint lastElement;
		const CFormDfn *nodeDfn;
		const CType *nodeType;
		CFormElm *node;
		UFormDfn::TEntryType type;
		bool array;
		bool parentVDfnArray;
		nlverify (((const CFormElm*)doc->getRootNode (getSlot ()))->getNodeByName (FormName.c_str(), &parentDfn, lastElement, &nodeDfn, &nodeType, &node, type, array, parentVDfnArray, true, NLGEORGES_FIRST_ROUND));
		nlassert (!array);

		// Node exist ?
		Combo.UpdateData ();
		if (node)
		{
			CFormElmVirtualStruct *virtualNode = safe_cast<CFormElmVirtualStruct*> (node);
			Combo.SetWindowText (virtualNode->DfnFilename.c_str());
		}
		else
		{
			Combo.SetWindowText ("");
		}
		Combo.UpdateData (FALSE);
	}
}

// ***************************************************************************

bool CFormMemCombo::isWnd (const CWnd *wnd) const
{
	return Combo.isWnd (wnd);
}

// ***************************************************************************

void CFormMemCombo::resizeScan (RECT &currentPos, uint &widgetCount, uint adjust, bool resize)
{
	// Create the type combo
	Dialog->setStaticSize (currentPos);
	if (resize)
	{
		Label.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
			currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	}
	Dialog->getNextPosLabel (currentPos);

	// Create the spinner
	if (UseSpinner)
	{
		// Create the mem combobox
		Dialog->setComboSpinSize (currentPos);
		if (resize)
		{
			Combo.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
				currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
		}

		// Create the spin
		RECT spinPos = currentPos;
		Dialog->getNextSpinPos (spinPos);
		Dialog->setSpinSize (spinPos);
		if (resize)
		{
			Spin.SetWindowPos (NULL, spinPos.left, spinPos.top, spinPos.right - spinPos.left, 
				spinPos.bottom - spinPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
		}
	}
	else if (FileBrowser)
	{
		// Create the mem combobox
		Dialog->setComboBrowseSize (currentPos);
		if (resize)
		{
			Combo.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
				currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
		}

		// Create the spin
		RECT spinPos = currentPos;
		Dialog->getNextBrowsePos (spinPos);
		Dialog->setBrowseSize (spinPos);
		if (resize)
		{
			Browse.SetWindowPos (NULL, spinPos.left, spinPos.top, spinPos.right - spinPos.left, 
				spinPos.bottom - spinPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
		}
	}
	else
	{
		// Create the mem combobox
		Dialog->setComboSize (currentPos, Dialog->SmallWidget);
		if (resize)
		{
			Combo.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
				currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
		}
	}
	Dialog->getNextPos (currentPos);
}

// ***************************************************************************

bool CFormMemCombo::getWindowRect (RECT &rect) const
{
	if (Combo)
	{
		// Get parent rect
		RECT parentRect;
		IFormWidget::getWindowRect (parentRect);

		// Add my rect
		RECT myRect;
		Combo.GetWindowRect (&myRect);
		UnionRect(&rect, &myRect, &parentRect);
		return true;
	}
	else
		return false;
}

// ***************************************************************************
// CFormCombo
// ***************************************************************************

CFormCombo::CFormCombo (CFormDialog *dialog, uint structId, const char *atomName, IFormWidget::TTypeSrc typeForm, uint slot) : IFormWidget (dialog, structId, atomName, typeForm, slot)
{
}

// ***************************************************************************

CFormCombo::~CFormCombo ()
{
	if (IsWindow (Label))
		Label.DestroyWindow ();
	if (IsWindow (Combo))
		Combo.DestroyWindow ();
}

// ***************************************************************************

void CFormCombo::create (DWORD wStyle, RECT &currentPos, CFormDialog *parent, uint &dialog_index, const char *label)
{
	// Get the doc
	CGeorgesEditDoc *doc = Dialog->View->GetDocument ();
	nlassert (doc);

	FirstId = dialog_index;
	LastId = FirstId;

	// Get the value for the type pointer (if any)
	const CFormDfn *parentDfn;
	uint indexDfn;
	const CFormDfn *nodeDfn;
	const CType *nodeType;
	CFormElm *node;
	UFormDfn::TEntryType type;
	bool array;
	bool parentVDfnArray;
	CForm *form=doc->getFormPtr ();
	CFormElm *elm = doc->getRootNode (getSlot ());
	nlverify ( elm->getNodeByName (FormName.c_str (), &parentDfn, indexDfn, 
		&nodeDfn, &nodeType, &node, type, array, parentVDfnArray, true, NLGEORGES_FIRST_ROUND) );

	// Save the label 
	SavedLabel = label;

	// Create the type combo
	parent->setStaticSize (currentPos);
	Label.Create (label, WS_VISIBLE, currentPos, parent);
	parent->initWidget (Label);
	parent->getNextPosLabel (currentPos);

	// Create the mem combobox
	parent->setComboSize (currentPos, parent->SmallWidget);
	RECT comboPos = currentPos;
	parent->adjusteComboSize (comboPos);
	Combo.Create (WS_CHILD|WS_VSCROLL|WS_VISIBLE|CBS_DROPDOWNLIST|CBS_HASSTRINGS|WS_CHILD|WS_TABSTOP, comboPos, parent, dialog_index);
	parent->initWidget (Combo);
	parent->getNextPos (currentPos);

	// Get predefs
	if (nodeType)
	{
		Combo.InsertString (0, "");
		for (uint predef=0; predef<nodeType->Definitions.size(); predef++)
		{
			Combo.InsertString (predef+1, nodeType->Definitions[predef].Label.c_str());
		}
	}

	dialog_index += 1;
}

// ***************************************************************************

void CFormCombo::updateData (bool update)
{
	Combo.UpdateData (update?TRUE:FALSE);
}

// ***************************************************************************

bool CFormCombo::haveFocus ()
{
	CWnd *focus = CWnd::GetFocus ();
	if (focus)
	{
		return (focus == &Combo);
	}
	return false;
}

// ***************************************************************************

void CFormCombo::setFocus ()
{
	Combo.SetFocus ();
}

// ***************************************************************************

void CFormCombo::onOk ()
{
	//Combo.onOK ();
}

// ***************************************************************************

void CFormCombo::getValue (std::string &result)
{
	Combo.UpdateData ();

	CString str;
	Combo.GetWindowText (str);

	Combo.UpdateData (FALSE);

	// Set the atom value
	result = (const char*)str;
}

// ***************************************************************************

void CFormCombo::getFromDocument (CForm &form)
{
	CGeorgesEditDoc *doc = Dialog->View->GetDocument ();
	nlassert (doc);
	string result;
	if (doc->getRootNode (getSlot ())->getValueByName (result, FormName.c_str(), UFormElm::NoEval, NULL))
	{
		Combo.UpdateData ();
		uint itemCount = Combo.GetCount ();
		for (uint i=0; i<itemCount; i++)
		{
			CString item;
			Combo.GetLBText (i, item);
			if (item == result.c_str())
			{
				Combo.SetCurSel (i);
				break;
			}
		}
		Combo.UpdateData (FALSE);
		updateLabel ();
	}
	else
	{
		nlstop;
	}
}

// ***************************************************************************

bool CFormCombo::isWnd (const CWnd *wnd) const
{
	return &Combo == (const CWnd*)wnd;
}

// ***************************************************************************

void CFormCombo::resizeScan (RECT &currentPos, uint &widgetCount, uint adjust, bool resize)
{
	// Create the type combo
	Dialog->setStaticSize (currentPos);
	if (resize)
	{
		Label.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
			currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	}
	Dialog->getNextPosLabel (currentPos);

	// Create the mem combobox
	Dialog->setComboSize (currentPos, Dialog->SmallWidget);
	if (resize)
	{
		RECT comboPos = currentPos;
		Dialog->adjusteComboSize (comboPos);
		Combo.SetWindowPos (NULL, comboPos.left, comboPos.top, comboPos.right - comboPos.left, 
			comboPos.bottom - comboPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	}
	Dialog->getNextPos (currentPos);
}

// ***************************************************************************

bool CFormCombo::getWindowRect (RECT &rect) const
{
	if (IsWindow (Combo))
	{
		// Get parent rect
		RECT parentRect;
		IFormWidget::getWindowRect (parentRect);

		// Add my rect
		RECT myRect;
		Combo.GetWindowRect (&myRect);
		UnionRect(&rect, &myRect, &parentRect);
		return true;
	}
	else
		return false;
}

// ***************************************************************************
// CFormBigEdit
// ***************************************************************************

CFormBigEdit::CFormBigEdit (CFormDialog *dialog, uint structId, const char *atomName, IFormWidget::TTypeSrc typeForm, uint slot) : IFormWidget (dialog, structId, atomName, typeForm, slot)
{
}

// ***************************************************************************

CFormBigEdit::~CFormBigEdit ()
{
	if (IsWindow (Label))
		Label.DestroyWindow ();
	if (IsWindow (Edit))
		Edit.DestroyWindow ();
}

// ***************************************************************************

void CFormBigEdit::create (DWORD wStyle, RECT &currentPos, CFormDialog *parent, uint &dialog_index, const char *label)
{
	FirstId = dialog_index;
	LastId = FirstId;

	// Save the label 
	SavedLabel = label;

	// Create the type combo
	parent->setStaticSize (currentPos);
	Label.Create (label, WS_VISIBLE, currentPos, parent);
	parent->initWidget (Label);
	parent->getNextPosLabel (currentPos);

	// Create the mem combobox
	parent->setBigEditSize (currentPos, parent->SmallWidget);
	Edit.CreateEx (WS_EX_CLIENTEDGE, _T("EDIT"), "", WS_VSCROLL|ES_OEMCONVERT|ES_MULTILINE|ES_WANTRETURN|WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL|ES_AUTOVSCROLL, currentPos, parent, dialog_index);
	parent->initWidget (Edit);
	parent->getNextPos (currentPos);

	dialog_index += 1;
}

// ***************************************************************************

void CFormBigEdit::updateData (bool update)
{
	Edit.UpdateData (update?TRUE:FALSE);
}

// ***************************************************************************

bool CFormBigEdit::haveFocus ()
{
	CWnd *focus = CWnd::GetFocus ();
	if (focus)
	{
		return (focus == &Edit);
	}
	return false;
}

// ***************************************************************************

void CFormBigEdit::setFocus ()
{
	Edit.SetFocus ();
}

// ***************************************************************************

void CFormBigEdit::onOk ()
{
	//Combo.onOK ();
}

// ***************************************************************************

void CFormBigEdit::getValue (std::string &result)
{
	Edit.UpdateData ();

	CString str;
	Edit.GetWindowText (str);

	Edit.UpdateData (FALSE);
	
	// Set the atom value
	result = (const char*)str;
}

// ***************************************************************************

void CFormBigEdit::getFromDocument (CForm &form)
{
	CGeorgesEditDoc *doc = Dialog->View->GetDocument ();
	nlassert (doc);
	string result;
	if (doc->getRootNode (getSlot ())->getValueByName (result, FormName.c_str(), UFormElm::NoEval, NULL))
	{
		Edit.UpdateData ();
		Dialog->setEditTextMultiLine (Edit, result.c_str());
		Edit.UpdateData (FALSE);
		updateLabel ();
	}
	else
	{
		nlstop;
	}
}

// ***************************************************************************

bool CFormBigEdit::isWnd (const CWnd *wnd) const
{
	return &Edit == wnd;
}

// ***************************************************************************

void CFormBigEdit::resizeScan (RECT &currentPos, uint &widgetCount, uint adjust, bool resize)
{
	if (!resize)
		widgetCount++;

	// Create the type combo
	Dialog->setStaticSize (currentPos);
	if (resize)
	{
		Label.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
			currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	}
	Dialog->getNextPosLabel (currentPos);

	// Create the mem combobox
	Dialog->setBigEditSize (currentPos, Dialog->SmallWidgetNotLimited, Dialog->BigEditHeight + adjust);
	if (resize)
	{
		Edit.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
			currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	}
	Dialog->getNextPos (currentPos);
}

// ***************************************************************************

bool CFormBigEdit::extendableHeight () const
{
	return true;
}

// ***************************************************************************

bool CFormBigEdit::getWindowRect (RECT &rect) const
{
	if (IsWindow (Edit))
	{
		// Get parent rect
		RECT parentRect;
		IFormWidget::getWindowRect (parentRect);

		// Add my rect
		RECT myRect;
		Edit.GetWindowRect (&myRect);
		UnionRect(&rect, &myRect, &parentRect);
		return true;
	}
	else
		return false;
}

// ***************************************************************************
// CColorEdit
// ***************************************************************************

CColorEdit::CColorEdit (CFormDialog *dialog, uint structId, const char *atomName, IFormWidget::TTypeSrc typeForm, uint slot) : IFormWidget (dialog, structId, atomName, typeForm, slot)
{
	Empty = true;
}

// ***************************************************************************

CColorEdit::~CColorEdit ()
{
	if (IsWindow (Label))
		Label.DestroyWindow ();
	if (IsWindow (Color))
		Color.DestroyWindow ();
	if (IsWindow (Edit))
		Edit.DestroyWindow ();
}

// ***************************************************************************

void CColorEdit::create (DWORD wStyle, RECT &currentPos, CFormDialog *parent, uint &dialog_index, const char *label)
{
	FirstId = dialog_index;
	LastId = FirstId+2;

	// Save the label 
	SavedLabel = label;

	// Create the type combo
	parent->setStaticSize (currentPos);
	Label.Create (label, WS_VISIBLE, currentPos, parent);
	parent->initWidget (Label);
	parent->getNextPosLabel (currentPos);

	// Create the mem combobox
	parent->setColorSize (currentPos, parent->SmallWidget);
	Color.create (WS_CHILD|WS_VISIBLE|WS_TABSTOP, currentPos, parent, dialog_index);
	parent->initWidget (Color);

	// Create the reset button
	RECT resetPos = currentPos;
	parent->getNextColorPos (resetPos);
	parent->setResetColorSize (resetPos);
	Reset.Create ("Reset", WS_CHILD|WS_VISIBLE|WS_TABSTOP, resetPos, parent, dialog_index+1);
	parent->initWidget (Reset);
	parent->getNextPosLabel (currentPos);

	// Create the Edit
	parent->setBigEditSize (currentPos, parent->SmallWidget);
	Edit.CreateEx (WS_EX_CLIENTEDGE, _T("EDIT"), "", ES_OEMCONVERT|ES_WANTRETURN|WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL, currentPos, parent, dialog_index+2);
	parent->initWidget (Edit);
	parent->getNextPos (currentPos);

	Color.setEdit(&Edit);

	dialog_index += 3;
}

// ***************************************************************************

void CColorEdit::updateData (bool update)
{
	Color.UpdateData (update?TRUE:FALSE);
	Edit.UpdateData (update?TRUE:FALSE);
}

// ***************************************************************************

bool CColorEdit::haveFocus ()
{
	CWnd *focus = CWnd::GetFocus ();
	if (focus)
	{
		return (focus == &Color);
	}
	return false;
}

// ***************************************************************************

void CColorEdit::setFocus ()
{
	Color.SetFocus ();
}

// ***************************************************************************

void CColorEdit::onOk ()
{
	// Color.onOk ();
}

// ***************************************************************************

void CColorEdit::getValue (std::string &result)
{
	if (!Empty)
	{
		Color.UpdateData ();

		// Get the color
		CRGBA color = Color.getColor ();

		// Make a string
		char colorName[512];
		smprintf (colorName, 512, "%d,%d,%d", color.R, color.G, color.B);

		// Set the atom value
		result = colorName;
	}
	else
	{
		result = "";
	}
}

// ***************************************************************************

void CColorEdit::getFromDocument (CForm &form)
{
	CGeorgesEditDoc *doc = Dialog->View->GetDocument ();
	nlassert (doc);
	string result;
	if (doc->getRootNode (getSlot ())->getValueByName (result, FormName.c_str(), UFormElm::NoEval, NULL))
	{
		Color.UpdateData ();

		// Make a color
		sint r, g, b;
		if (sscanf (result.c_str(), "%d,%d,%d", &r, &g, &b) == 3)
		{
			clamp (r, 0, 255);
			clamp (g, 0, 255);
			clamp (b, 0, 255);
			CRGBA color (r, g, b);
			Color.setColor (color);
			if (r != 255 && g != 255 && b != 255)
				Color.updateEdit();
		}
		else
		{
			Color.setColor (CRGBA::Black);
			Color.updateEdit();
		}
		Color.UpdateData (FALSE);
		updateLabel ();
	}
	else
	{
		nlstop;
	}
}


// ***************************************************************************

bool CColorEdit::isWnd (const CWnd *wnd) const
{
	return &Color == wnd;
}

// ***************************************************************************

void CColorEdit::resizeScan (RECT &currentPos, uint &widgetCount, uint adjust, bool resize)
{
	// Create the type combo
	Dialog->setStaticSize (currentPos);
	if (resize)
	{
		Label.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
			currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	}
	Dialog->getNextPosLabel (currentPos);

	// Create the mem combobox
	Dialog->setColorSize (currentPos, Dialog->SmallWidget);
	if (resize)
	{
		Color.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
			currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	}

	// Create the spin
	RECT resetPos = currentPos;
	Dialog->getNextColorPos (resetPos);
	Dialog->setResetColorSize (resetPos);
	if (resize)
	{
		Reset.SetWindowPos (NULL, resetPos.left, resetPos.top, resetPos.right - resetPos.left, 
			resetPos.bottom - resetPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	}
	Dialog->getNextPosLabel (currentPos);

	// Create the Edit
	Dialog->setEditSize (currentPos, Dialog->SmallWidget, Dialog->EditHeight);
	if (resize)
	{
		Edit.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
			currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	}

	Dialog->getNextPos (currentPos);
}

// ***************************************************************************

bool CColorEdit::getWindowRect (RECT &rect) const
{
	if (IsWindow (Color))
	{
		// Get parent rect
		RECT parentRect;
		IFormWidget::getWindowRect (parentRect);

		// Add my rect
		RECT myRect;
		Color.GetWindowRect (&myRect);
		UnionRect(&rect, &myRect, &parentRect);
		return true;
	}
	else
		return false;
}

// ***************************************************************************
// CListWidget
// ***************************************************************************

CListWidget::CListWidget (CFormDialog *dialog, uint structId, const char *atomName, IFormWidget::TTypeSrc typeForm, uint slot)  : IFormWidget (dialog, structId, atomName, typeForm, slot)
{
	ListCtrl.Ctrl = this;
}

// ***************************************************************************

CListWidget::~CListWidget ()
{
	if (IsWindow (Label))
		Label.DestroyWindow ();
	if (IsWindow (ListCtrl))
		ListCtrl.DestroyWindow ();
	if (IsWindow (Button))
		Button.DestroyWindow ();
}

// ***************************************************************************

void CListWidget::create (DWORD wStyle, RECT &currentPos, CFormDialog *parent, uint &dialog_index, const char *label, const char *reg, uint divid)
{
	FirstId = dialog_index;
	LastId = FirstId+1;

	Divid = divid;
	RegAdr = reg;

	// Save the label 
	SavedLabel = label;

	// Create the type combo
	parent->setStaticSize (currentPos);
	Label.Create (label, WS_VISIBLE, currentPos, parent);
	parent->initWidget (Label);
	parent->getNextPosLabel (currentPos);

	// Create the mem combobox
	parent->setListSize (currentPos, parent->SmallWidget);
	ListCtrl.create (WS_CHILD|WS_VISIBLE|WS_TABSTOP, currentPos, parent, dialog_index);
	parent->initWidget (ListCtrl);
	parent->getNextPos (currentPos);

	// Create the assign parent
	parent->setButtonSize (currentPos, parent->SmallWidget);
	Button.Create ("Assign parents", WS_CHILD|WS_VISIBLE|WS_TABSTOP, currentPos, parent, dialog_index+1);
	parent->initWidget (Button);
	parent->getNextPos (currentPos);

	dialog_index += 2;
}

// ***************************************************************************

void CListWidget::addColumn (const char *name)
{
	ListCtrl.insertColumn (0, name);
	ListCtrl.recalcColumn ();
}

// ***************************************************************************

void CListWidget::onOk ()
{
	ListCtrl.onOK ();
}

// ***************************************************************************

void CListWidget::updateData (bool update)
{
	ListCtrl.UpdateData (update);
}

// ***************************************************************************

bool CListWidget::haveFocus ()
{
	CWnd *wnd = Dialog->GetFocus ();
	if (wnd)
	{
		return (wnd->GetParent () == &ListCtrl);
	}
	return false;
}

// ***************************************************************************

void CListWidget::setFocus ()
{
	ListCtrl.SetFocus ();
}

// ***************************************************************************

void CListWidget::getFromDocument (NLGEORGES::CForm &form)
{
	// Erase the list
	ListCtrl.ListCtrl.DeleteAllItems ();
	
	// For each parent
	for (uint parent=0; parent<form.getParentCount (); parent++)
	{
		// Get the parent filename
		string filename = form.getParentFilename (parent);

		// Insert in the list
		ListCtrl.ListCtrl.InsertItem (parent, filename.c_str ());

		ListCtrl.ListCtrl.UpdateData (FALSE);
		updateLabel ();
	}
}

// ***************************************************************************

uint CListWidget::getNumValue ()
{
	return ListCtrl.ListCtrl.GetItemCount ();
}

// ***************************************************************************

void CListWidget::getValue (std::string &result, uint value)
{
	CString str = ListCtrl.ListCtrl.GetItemText (value, 0);
	result = str;
}

// ***************************************************************************

bool CListWidget::isWnd (const CWnd *wnd) const
{
	return (((&ListCtrl) == wnd) || ((&ListCtrl.ListCtrl) == wnd));
}

// ***************************************************************************

void CListWidget::resizeScan (RECT &currentPos, uint &widgetCount, uint adjust, bool resize)
{
	if (!resize)
		widgetCount++;

	// Create the type combo
	Dialog->setStaticSize (currentPos);
	if (resize)
	{
		Label.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
			currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	}
	Dialog->getNextPosLabel (currentPos);

	// Create the mem combobox
	Dialog->setListSize (currentPos, Dialog->SmallWidgetNotLimited / Divid, Dialog->ListHeight + adjust);
	if (resize)
	{
		ListCtrl.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
			currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
		ListCtrl.recalcColumn ();
	}
	Dialog->getNextPos (currentPos);

	// Create the mem combobox
	Dialog->setButtonSize (currentPos, Dialog->SmallWidget);
	if (resize)
	{
		Button.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
			currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	}
	Dialog->getNextPos (currentPos);
}

// ***************************************************************************

bool CListWidget::extendableHeight () const
{
	return true;
}

// ***************************************************************************

CEditListCtrl::TItemEdit CListWidget::CMyEditListCtrl::getItemEditMode (uint item, uint subItem)
{
	return CEditListCtrl::EditMemCombo;
}

// ***************************************************************************

void CListWidget::CMyEditListCtrl::getMemComboBoxProp (uint item, uint subItem, std::string &regAdr, bool &browse)
{
	regAdr = Ctrl->RegAdr;
	browse = true;
}

// ***************************************************************************

void CListWidget::CMyEditListCtrl::getNewItemText (uint item, uint subItem, std::string &ret)
{
	Ctrl->Dialog->getDfnName (ret);
	ret = "default." + ret;
}

// ***************************************************************************

void CListWidget::CMyEditListCtrl::getBrowseInfo (uint item, uint subItem, std::string &defExt, std::string &defFilename, std::string &defDir, std::string &filter)
{
	string ret;
	Ctrl->Dialog->getDfnName (ret);
	defExt = "*." + ret;
	defFilename = defExt;

	filter = "Form Files (*." + ret + ")|*." + ret + "|All Files (*.*)|*.*||";
	defDir = theApp.RootSearchPath;
}

// ***************************************************************************

bool CListWidget::getWindowRect (RECT &rect) const
{
	if (IsWindow (ListCtrl))
	{
		// Get parent rect
		RECT parentRect;
		IFormWidget::getWindowRect (parentRect);

		// Add my rect
		RECT myRect;
		ListCtrl.ListCtrl.GetWindowRect (&myRect);
		UnionRect(&rect, &myRect, &parentRect);
		return true;
	}
	else
		return false;
}

// ***************************************************************************

void CListWidget::onOpenSelected ()
{
	// For each selected
	POSITION pos = ListCtrl.ListCtrl.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nItem = ListCtrl.ListCtrl.GetNextSelectedItem(pos);

		// Get the string
		CString str = ListCtrl.ListCtrl.GetItemText (nItem, 0);
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

// ***************************************************************************

// ***************************************************************************
// CIconWidget
// ***************************************************************************

CIconWidget::CIconWidget (CFormDialog *dialog, uint structId, const char *atomName, IFormWidget::TTypeSrc typeForm, uint slot) : IFormWidget (dialog, structId, atomName, typeForm, slot)
{

}

// ***************************************************************************

CIconWidget::~CIconWidget ()
{
	if (IsWindow (Icon))
		Icon.DestroyWindow ();
}

// ***************************************************************************

void CIconWidget::create (DWORD wStyle, RECT &currentPos, CFormDialog *parent, uint &dialog_index, const char *label)
{
	FirstId = dialog_index;
	LastId = FirstId;

	// Save the label 
	SavedLabel = label;

	// Create the type combo
	parent->setStaticSize (currentPos);
	Label.Create (label, WS_VISIBLE, currentPos, parent);
	parent->initWidget (Label);
	parent->getNextPosLabel (currentPos);

	// Create the mem combobox
	parent->setEditSize (currentPos, parent->IconHeight, parent->IconHeight);
	Icon.create (WS_CHILD|WS_VISIBLE|WS_TABSTOP, currentPos, parent, dialog_index);
	parent->initWidget (Icon);

	parent->getNextPos (currentPos);

	dialog_index += 1;
}

// ***************************************************************************

void CIconWidget::updateData (bool update)
{
	Icon.UpdateData (update?TRUE:FALSE);
}

// ***************************************************************************

bool CIconWidget::haveFocus ()
{
	CWnd *focus = CWnd::GetFocus ();
	if (focus)
	{
		return (focus == &Icon);
	}
	return false;
}

// ***************************************************************************

void CIconWidget::setFocus ()
{
	Icon.SetFocus ();
}

// ***************************************************************************

void CIconWidget::onOk ()
{

}

// ***************************************************************************

void CIconWidget::getValue (std::string &result)
{
	result = "";
}

// ***************************************************************************

void CIconWidget::getFromDocument (CForm &form)
{

}


// ***************************************************************************

bool CIconWidget::isWnd (const CWnd *wnd) const
{
	return &Icon == wnd;
}

// ***************************************************************************

void CIconWidget::resizeScan (RECT &currentPos, uint &widgetCount, uint adjust, bool resize)
{
	// Create the type combo
	Dialog->setStaticSize (currentPos);
	if (resize)
	{
		Label.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
			currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	}
	Dialog->getNextPosLabel (currentPos);
	
	// Create the icon
	Dialog->setEditSize (currentPos, Dialog->IconHeight, Dialog->IconHeight);
	if (resize)
	{
		Icon.SetWindowPos (NULL, currentPos.left, currentPos.top, currentPos.right - currentPos.left, 
			currentPos.bottom - currentPos.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	}

	Dialog->getNextPos (currentPos);
}

// ***************************************************************************

bool CIconWidget::getWindowRect (RECT &rect) const
{
	if (IsWindow (Icon))
	{
		// Get parent rect
		RECT parentRect;
		IFormWidget::getWindowRect (parentRect);

		// Add my rect
		RECT myRect;
		Icon.GetWindowRect (&myRect);
		UnionRect(&rect, &myRect, &parentRect);
		return true;
	}
	else
		return false;
}