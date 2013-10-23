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

// find_primitive_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "world_editor.h"
#include "find_primitive_dlg.h"
#include "action.h"
#include "main_frm.h"
#include "editor_primitive.h"

using namespace NLLIGO;

CString CFindPrimitiveDlg::Property = _T("");
CString CFindPrimitiveDlg::Value = _T("");
CString CFindPrimitiveDlg::ReplaceText = _T("");
int		CFindPrimitiveDlg::SelectionOnly=0;	//	false;

// ***************************************************************************
// CFindPrimitiveDlg dialog
// ***************************************************************************

CFindPrimitiveDlg::CFindPrimitiveDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFindPrimitiveDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFindPrimitiveDlg)
	PrimitiveName = _T("");
	SelectionOnly=0;	//	false;
	//}}AFX_DATA_INIT
}

// ***************************************************************************

void CFindPrimitiveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFindPrimitiveDlg)
	DDX_Text(pDX, IDC_PROPERTY, Property);
	DDX_Text(pDX, IDC_VALUE, Value);
	DDX_Text(pDX, IDC_PRIMITIVE_NAME, PrimitiveName);
	DDX_Text(pDX, IDC_REPLACE_TEXT, ReplaceText);
	DDX_Check(pDX, IDC_SELECTION, SelectionOnly);
	//}}AFX_DATA_MAP
}

// ***************************************************************************

BEGIN_MESSAGE_MAP(CFindPrimitiveDlg, CDialog)
	//{{AFX_MSG_MAP(CFindPrimitiveDlg)
	ON_BN_CLICKED(ID_FIND_NEXT, OnFindNext)
	ON_BN_CLICKED(ID_REPLACE, OnReplace)
	ON_BN_CLICKED(ID_REPLACE_ALL, OnReplaceAll)
//	ON_BN_CLICKED(IDC_SELECTION, SetSelection)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ***************************************************************************
// CFindPrimitiveDlg message handlers
// ***************************************************************************

void CFindPrimitiveDlg::SetSelection() 
{
	SelectionOnly=1-SelectionOnly;
	_Locator.getRoot(0);
	UpdateData ();
}

void CFindPrimitiveDlg::OnFindNext() 
{
	CDatabaseLocatorPointer	TmpLocator;
	UpdateData ();

	// Find the primitive
	TmpLocator.getRoot(0);
	if (_Locator == TmpLocator)
	{
		_End = false;
	}
	else
	{
		_End = !_Locator.next ();
	}
	bool found = false;
	while (!_End)
	{
		// Primitive here ?
		if (_Locator.Primitive)
		{
			// Property here ?
			const IProperty *property;
			if (	(	!SelectionOnly
//					||	(	_Locator.Primitive->getPropertyByName ("selected", property)
					||	(	getPrimitiveEditor(_Locator.Primitive)->getSelected()))
//						&&	property)	)
				&&	_Locator.Primitive->getPropertyByName ((const char*)Property, property)
				&&	property)
			{
				// Kind of primitive ?
				const CPropertyString *propString = dynamic_cast<const CPropertyString *>(property);
				if (propString)
				{
					// Good value ?
					if	(propString->String.find(Value)!=std::string::npos)
					{
						found = true;
					}

				}
				else
				{
					const CPropertyStringArray *propStringArray = dynamic_cast<const CPropertyStringArray *>(property);
					if (propStringArray)
					{
						// For all the values
						uint i;
						for (i=0; i<propStringArray->StringArray.size (); i++)
						{
							if	(propStringArray->StringArray[i].find(Value)!=std::string::npos)
							{
								found = true;
							}

						}

					}

				}

			}

		}

		if (found) break;
		// Last one ?
		_End = !_Locator.next ();
	}

	// Found ?
	if (found)
	{
		// Primitive name
		PrimitiveName = ("Found : " + _Locator.getPathName ()).c_str ();

		// Get the document
		CWorldEditorDoc *doc = getDocument ();

		doc->beginModification ();
		// Unselect all
		doc->addModification (new CActionUnselectAll ());

		// Select the primitive
		doc->addModification (new CActionSelect (_Locator));
		doc->endModification ();
		
		// Update view
		getMainFrame ()->updateData ();
		getMainFrame ()->OnViewLocateselectedprimitives ();
		getMainFrame ()->OnViewLocateselectedprimitivesTree ();
	}
	else
	{
		MessageBox ("End of the document", "Find a primitive...", MB_OK|MB_ICONEXCLAMATION);

		// Init locator
		_Locator.getRoot (0);
		PrimitiveName = "";
	}
	UpdateData (FALSE);
}

void CFindPrimitiveDlg::OnReplaceAll()
{
	replace(true);
}

void CFindPrimitiveDlg::OnReplace()
{
	replace(false);
}


void CFindPrimitiveDlg::replace(bool all)
{
	CDatabaseLocatorPointer	TmpLocator;
	UpdateData ();

	if	(all)
		_Locator.getRoot(0);	//	starts at top of document.

	// Find the primitive
	TmpLocator.getRoot(0);
	if (_Locator == TmpLocator)
	{
		_End = false;
	}

	CWorldEditorDoc *doc = getDocument ();
	doc->beginModification ();

	bool	firstTime=true;

	while (!_End)
	{
		// Primitive here ?
		if	(_Locator.Primitive)
		{
			// Property here ?
			const IProperty *property;
			if	(	(	!SelectionOnly
//					||	(	_Locator.Primitive->getPropertyByName ("selected", property)
					||	(	getPrimitiveEditor(_Locator.Primitive)->getSelected()))
//						&&	property)	)
				&&	_Locator.Primitive->getPropertyByName ((const char*)Property, property)
				&&	property	)
			{
				// Kind of primitive ?
				const CPropertyString *propString = dynamic_cast<const CPropertyString *>(property);
				if	(propString)
				{
					// Good value ?
					if	(propString->String.find(Value)!=std::string::npos)
					{
						if	(!firstTime	&&	!all)
							break;

						CString	tmp(propString->String.c_str());
						tmp.Replace(Value, ReplaceText);
						doc->addModification (new CActionSetPrimitivePropertyString (_Locator,(const char*)Property,(const char*)tmp,false));
						doc->addModification (new CActionSelect (_Locator));
						
						firstTime=false;
					}

				}
				else
				{
					const CPropertyStringArray *propStringArray = dynamic_cast<const CPropertyStringArray *>(property);
					if (propStringArray)
					{
						bool	firstChange=true;
						std::vector<std::string>	newStrings;

						// For all the values
						uint i;
						for (i=0; i<propStringArray->StringArray.size (); i++)
						{
							//	todo.
							if	(propStringArray->StringArray[i].find(Value)!=std::string::npos)
							{
								if	(	!firstTime
									&&	!all)
									break;

								CString	tmp(propStringArray->StringArray[i].c_str());
								const	int nbChange=tmp.Replace(Value, ReplaceText);

								if	(nbChange>0)
								{
									if	(firstChange)
									{
										newStrings=propStringArray->StringArray;
										firstChange=false;
									}
									newStrings[i]=std::string((const char*)tmp);
								}
								firstTime=false;
							}

						}

						if (!firstChange)	//	have to make a change
						{
							doc->addModification (new CActionSetPrimitivePropertyStringArray (_Locator,(const char*)Property,newStrings,false));
							doc->addModification (new CActionSelect (_Locator));
						}

					}

				}

			}

		}
//		// Last one ?
		_End = !_Locator.next ();
	}

	if (!_End)
	{
		doc->addModification (new CActionSelect (_Locator));
	}

	doc->endModification ();
	
	// Update view
	getMainFrame ()->updateData ();
	getMainFrame ()->OnViewLocateselectedprimitives ();
	getMainFrame ()->OnViewLocateselectedprimitivesTree ();

	UpdateData (FALSE);

	if (_End)
	{
		MessageBox ("End of the document", "Find a primitive...", MB_OK|MB_ICONEXCLAMATION);
		_Locator.getRoot (0);
	}

}

// ***************************************************************************

BOOL CFindPrimitiveDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// Init locator

	_Locator.getRoot (0);

	// Get the document
	//CWorldEditorDoc *doc = getDocument ();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ***************************************************************************

void CFindPrimitiveDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// Get the document
	//CWorldEditorDoc *doc = getDocument ();
}

// ***************************************************************************
