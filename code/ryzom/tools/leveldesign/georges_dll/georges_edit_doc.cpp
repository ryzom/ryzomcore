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

#include "stdafx.h"
#include "georges_edit.h"
#include "georges_edit_doc.h"
#include "georges_edit_view.h"
#include "main_frm.h"
#include "left_view.h"
#include "child_frm.h"
#include "action.h"
#include "reg_shell_ext.h"

#include "nel/misc/file.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/path.h"

#include "nel/georges/form.h"

using namespace NLMISC;
using namespace NLGEORGES;

using namespace std;
using namespace NLGEORGES;


// ***************************************************************************
// CGeorgesEditDoc
// ***************************************************************************

IMPLEMENT_DYNCREATE(CGeorgesEditDocType, CDocument)
IMPLEMENT_DYNCREATE(CGeorgesEditDocDfn, CDocument)
IMPLEMENT_DYNCREATE(CGeorgesEditDocForm, CDocument)

// ***************************************************************************

BEGIN_MESSAGE_MAP(CGeorgesEditDoc, CDocument)
	//{{AFX_MSG_MAP(CGeorgesEditDoc)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_ALL, OnUpdateFileSaveAll)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ***************************************************************************

BEGIN_MESSAGE_MAP(CGeorgesEditDocType, CDocument)
	//{{AFX_MSG_MAP(CGeorgesEditDocType)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ***************************************************************************

BEGIN_MESSAGE_MAP(CGeorgesEditDocDfn, CDocument)
	//{{AFX_MSG_MAP(CGeorgesEditDocDfn)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ***************************************************************************

BEGIN_MESSAGE_MAP(CGeorgesEditDocForm, CDocument)
	//{{AFX_MSG_MAP(CGeorgesEditDocForm)
	ON_COMMAND(ID_EDIT_FETCH1, OnEditFetch1)
	ON_COMMAND(ID_EDIT_FETCH3, OnEditFetch3)
	ON_COMMAND(ID_EDIT_FETCH4, OnEditFetch4)
	ON_COMMAND(ID_EDIT_FETCH2, OnEditFetch2)
	ON_COMMAND(ID_EDIT_HOLD1, OnEditHold1)
	ON_COMMAND(ID_EDIT_HOLD2, OnEditHold2)
	ON_COMMAND(ID_EDIT_HOLD3, OnEditHold3)
	ON_COMMAND(ID_EDIT_HOLD4, OnEditHold4)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ***************************************************************************
// CGeorgesEditDoc construction/destruction
// ***************************************************************************

CGeorgesEditDoc::CGeorgesEditDoc()
{
	NoModification = false;
	_UndoModify = 0;
}

// ***************************************************************************

CGeorgesEditDoc::~CGeorgesEditDoc()
{
	for (uint i=0; i<PluginArray.size (); i++)
	{
		delete PluginArray[i].PluginInterface;
	}
	PluginArray.clear ();
	
	// Clear actions
	clearUndo ();
	clearRedo ();
}

// ***************************************************************************

BOOL CGeorgesEditDocType::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// Tmp
	Type = new CType;

	RootObject.create (CGeorgesEditDocSub::Null, "Type", 0xffffffff, "NULL", 0xffffffff);
	RootObject.add (CGeorgesEditDocSub::Header, "Header", 0xffffffff, "NULL", 0xffffffff);
	RootObject.add (CGeorgesEditDocSub::Type, "Content", 0xffffffff, "NULL", 0xffffffff);
	updateDocumentStructure ();

	return TRUE;
}

// ***************************************************************************

BOOL CGeorgesEditDocDfn::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TMp
	Dfn = new CFormDfn;

	RootObject.create (CGeorgesEditDocSub::Null, "Dfn", 0xffffffff, "NULL", 0xffffffff);
	RootObject.add (CGeorgesEditDocSub::Header, "Header", 0xffffffff, "NULL", 0xffffffff);
	RootObject.add (CGeorgesEditDocSub::Dfn, "Content", 0xffffffff, "NULL", 0xffffffff);
	updateDocumentStructure ();

	return TRUE;
}

// ***************************************************************************

NLGEORGES::CFormElm *CGeorgesEditDoc::getRootNode (uint slot)
{
	if (slot == 0)
	{
		return (CFormElm*)&Form->getRootNode ();
	}
	else 
	{
		nlassert (slot < CForm::HeldElementCount+1);
		return ((CForm*)(UForm*)Form)->HeldElements[slot-1];
	}
}

// ***************************************************************************

bool CGeorgesEditDocForm::initDocument (const char *dfnName, bool newElement)
{
	// Load the DFN
	CFormDfn *dfn = FormLoader.loadFormDfn (dfnName, false);
	if (!dfn)
	{
		char msg[512];
		smprintf (msg, 512, "Can't load DFN '%s'", dfnName);
		theApp.outputError (msg);
		return false;
	}

	// Set file name and title
	char name[512];
	char ext[512];
	_splitpath (dfnName, NULL, NULL, name, ext);
	string name2 = (const char*)(name);
	name2 = strlwr (name2);
	SetPathName ( ("*."+name2).c_str(), FALSE);
	SetTitle ( ("New "+name2+" form").c_str() );

	// TMp
	if (newElement)
	{
		Form = new CForm;

		// Build the root element
		((CFormElmStruct*)getRootNode (0))->build (dfn);

		uint i;
		for (i=0; i<CForm::HeldElementCount; i++)
		((CFormElmStruct*)getRootNode (i+1))->build (dfn);
	}

	RootObject.create (CGeorgesEditDocSub::Null, "Form", 0xffffffff, "NULL", 0xffffffff);
	RootObject.add (CGeorgesEditDocSub::Header, "Header", 0xffffffff, "NULL", 0xffffffff);
	RootObject.add (CGeorgesEditDocSub::Form, "Content", 0xffffffff, "", 0);
	uint i;
	for (i=0; i<CForm::HeldElementCount; i++)
		RootObject.add (CGeorgesEditDocSub::Form, ("Hold " + toString (1+i)).c_str (), 0xffffffff, "", i+1);
	updateDocumentStructure ();

	return true;
}

// ***************************************************************************

BOOL CGeorgesEditDocForm::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// User mode ?
	if (theApp.Superuser)
	{
		// Choose a DFN for this form
		string defFilename = theApp.RootSearchPath;
		defFilename += "*.dfn";

		CFileDialog dlgFile (TRUE, "*.dfn", defFilename.c_str (), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, DfnFilter, theApp.m_pMainWnd);
		if (dlgFile.DoModal () == IDOK)
		{
			if (initDocument (dlgFile.GetFileName (), true))
				return TRUE;
		}
	}
	else
	{
		// Get the Dfn name
		CMyMultiDocTemplate *docTemplate = safe_cast<CMyMultiDocTemplate*> (GetDocTemplate ());
		string dfnName;
		docTemplate->getDfnName (dfnName);
		if (initDocument (dfnName.c_str (), true))
			return TRUE;
	}

	return FALSE;
}

// ***************************************************************************
// CGeorgesEditDoc diagnostics
// ***************************************************************************

#ifdef _DEBUG
void CGeorgesEditDoc::AssertValid() const
{
	CDocument::AssertValid();
}

// ***************************************************************************

void CGeorgesEditDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

// ***************************************************************************

bool CGeorgesEditDoc::isType () const
{
	return false;
}

// ***************************************************************************

bool CGeorgesEditDoc::isDfn () const
{
	return false;
}

// ***************************************************************************

bool CGeorgesEditDoc::isForm () const
{
	return false;
}

// ***************************************************************************

bool CGeorgesEditDocType::isType () const
{
	return true;
}

// ***************************************************************************

bool CGeorgesEditDocDfn::isDfn () const
{
	return true;
}

// ***************************************************************************

bool CGeorgesEditDocForm::isForm () const
{
	return true;
}

// ***************************************************************************

CGeorgesEditDocSub *CGeorgesEditDoc::addStruct (CGeorgesEditDocSub *parent, CFormElmStruct *_struct, CFormDfn *parentDfn,
												const char *name, uint structId, const char *formName, uint slot)
{
	// The form pointer
	CForm *formPtr = (CForm*)(UForm*)Form;

	// Add the new node
	CGeorgesEditDocSub *newNode = parent->add (CGeorgesEditDocSub::Form, name, structId, formName, slot);

	// Can be NULL in virtual DFN
	if (parentDfn)
	{
		// Get the parents
		std::vector<CFormDfn *> arrayDfn;
		arrayDfn.reserve (parentDfn->countParentDfn ());
		parentDfn->getParentDfn (arrayDfn);

		// For each child
		uint elm=0;
		for (uint dfn=0; dfn<arrayDfn.size(); dfn++)
		{
			for (uint i=0; i<arrayDfn[dfn]->getNumEntry (); i++)
			{
				// Get the entry ref
				CFormDfn::CEntry &entry = arrayDfn[dfn]->getEntry (i);

				// Form entry name
				string entryName = (string (formName)+"."+entry.getName ());

				// Is a struct ?
				if ( (entry.getType () == UFormDfn::EntryDfn) || (entry.getType () == UFormDfn::EntryVirtualDfn) )
				{
					// Is an array of struct ?
					if (entry.getArrayFlag ())
					{
						// Get it from the form
						CFormElmArray *nextArray = NULL;
						if (_struct && _struct->Elements[elm].Element)
							nextArray = safe_cast<CFormElmArray*> (_struct->Elements[elm].Element);

						// Else, get it from the parent if we are not a virtual DFN (don't inheritate)

						// todo array of virtual struct
						if (!nextArray && (entry.getType () != UFormDfn::EntryVirtualDfn) )
						{
							// For each parent form
							for (uint parent=0; parent<formPtr->getParentCount (); parent++)
							{
								// Get the node by name
								UFormElm *uNode;
								if (formPtr->getParent (parent)->getRootNode ().getNodeByName (&uNode, entryName.c_str(), NULL, false) && uNode)
								{
									nextArray = safe_cast<CFormElmArray*> (uNode);
								}
							}
						}

						// Add the new struct
						addArray (newNode, nextArray, entry.getDfnPtr (), entry.getName().c_str(), elm, entryName.c_str (), slot);
					}
					else
					{
						// Add it
						CFormElmStruct *nextForm = NULL;

						// Get it from the form
						if (_struct && _struct->Elements[elm].Element)
							nextForm = safe_cast<CFormElmStruct*> (_struct->Elements[elm].Element);

						// Else, get it from the parent
						if (!nextForm)
						{
							// For each parent form
							for (uint parent=0; parent<formPtr->getParentCount (); parent++)
							{
								// Get the node by name
								UFormElm *uNode;
								if (formPtr->getParent (parent)->getRootNode ().getNodeByName (&uNode, entryName.c_str(), NULL, false) && uNode)
								{
									nextForm = safe_cast<CFormElmStruct*> (uNode);
								}
							}
						}

						// Virtual Dfn pointer
						CFormElmVirtualStruct *vStruct = ((entry.getType () == UFormDfn::EntryVirtualDfn) && nextForm) ? 
							safe_cast<CFormElmVirtualStruct*> (nextForm) : NULL;

						// Add the new struct
						addStruct (newNode, nextForm, vStruct ? vStruct->FormDfn : entry.getDfnPtr (), entry.getName().c_str(), elm, entryName.c_str(), slot);
					}
				}
				// Array of type ?
				else if ( entry.getArrayFlag () )
				{
					CFormElmArray *nextArray = NULL;

					// Get it from the form
					if (_struct && _struct->Elements[elm].Element)
						nextArray = safe_cast<CFormElmArray*> (_struct->Elements[elm].Element);

					// Else, get it from the parent
					if (!nextArray)
					{
						// For each parent form
						for (uint parent=0; parent<formPtr->getParentCount (); parent++)
						{
							// Get the node by name
							UFormElm *uNode;
							if (formPtr->getParent (parent)->getRootNode ().getNodeByName (&uNode, entryName.c_str(), NULL, false) && uNode)
							{
								nextArray = safe_cast<CFormElmArray*> (uNode);
							}
						}
					}

					// Add the new array
					addArray ( newNode, nextArray, NULL, entry.getName().c_str(), elm, entryName.c_str(), slot );
				}

				// Next element
				elm++;
			}
		}
	}

	return newNode;
}

// ***************************************************************************

CGeorgesEditDocSub *CGeorgesEditDoc::addArray (CGeorgesEditDocSub *parent, CFormElmArray *array, CFormDfn *rootDfn,
												const char *name, uint structId, const char *formName, uint slot)
{
	// Add the new node
	CGeorgesEditDocSub *newNode = parent->add (CGeorgesEditDocSub::Form, name, structId, formName, slot);

	// The array exist
	if (array)
	{
		// For each array element
		for (uint elm=0; elm<array->Elements.size(); elm++)
		{
			// The form name
			char formArrayElmName[512];
			smprintf (formArrayElmName, 512, "%s[%d]", formName, elm);

			// The name
			char formArrayName[512];
			if (array->Elements[elm].Name.empty ())
			{
				smprintf (formArrayName, 512, "#%d", elm);
			}
			else
			{
				smprintf (formArrayName, 512, "%s", array->Elements[elm].Name.c_str());
			}

			// Is a struct
			if (rootDfn)
			{
				// Get struct ptr
				CFormElmStruct *elmPtr =  array->Elements[elm].Element ? safe_cast<CFormElmStruct*>(array->Elements[elm].Element) : NULL;
				addStruct (newNode, elmPtr, rootDfn, formArrayName, elm, formArrayElmName, slot);
			}
			else
				newNode->add (CGeorgesEditDocSub::Form, formArrayName, elm, formArrayElmName, slot);
		}
	}

	return newNode;
}

// ***************************************************************************

BOOL CGeorgesEditDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	// Read the file
	if (isType())
	{
		try
		{
			// Read the form with the loader
			Type = FormLoader.loadType (lpszPathName);
			if (!Type)
			{
				char msg[512];
				smprintf (msg, 512, "Error while loading Type file %s", lpszPathName);
				theApp.outputError (msg);
				return FALSE;
			}

			updateDocumentStructure ();

			return TRUE;
		}
		catch (Exception &e)
		{
			char message[512];
			smprintf (message, 512, "Error while loading Type file: %s", e.what());
			theApp.outputError (message);
			return FALSE;
		}
	}
	else if (isDfn())
	{
		try
		{
			// Read the form with the loader
			Dfn = FormLoader.loadFormDfn (lpszPathName, true);
			if (!Dfn)
			{
				char msg[512];
				smprintf (msg, 512, "Error while loading Dfn file %s", lpszPathName);
				theApp.outputError (msg);
				return FALSE;
			}

			updateDocumentStructure ();

			return TRUE;
		}
		catch (Exception &e)
		{
			char message[512];
			smprintf (message, 512, "Error while loading Type file: %s", e.what());
			theApp.outputError (message);
			return FALSE;
		}
	}
	else
	{
		try
		{
			// Check form
			char ext[MAX_PATH];
			_splitpath (lpszPathName, NULL, NULL, NULL, ext);
			string extLower = strlwr (string (ext));
			if (!extLower.empty ())
			{
				string dfnName = extLower.substr (1, string::npos)  + ".dfn";

				// Check if the file is handled
				if (theApp.getFormDocTemplate (dfnName.c_str ()) == NULL)
				{
					char message[512];
					smprintf (message, 512, "Can't open the file '%s'.", lpszPathName);
					theApp.outputError (message);
					return FALSE;
				}
				
				// Read the form with the loader
				if (!loadFormFile (lpszPathName))
					return FALSE;

				if (theApp.ExeStandalone)
				{
					// Loaded ! Register type..

					// Does have an icon ?
// 					string iconPath = CPath::lookup (extLower.substr (1, string::npos)  + ".ico", false, false);
// 					string commandApp = "Georges.Form" + extLower;

					RegisterShellFileExt (extLower.c_str(), "Georges.Form");

/*
					if (!iconPath.empty ())
					{
						// Application path
						string appPath = "\"";
						appPath += theApp.ExePath;
						appPath += "\"";

						const char *open = "Open(\"%1\")";
						const char *copy = "Copy(\"%1\")";
						const char *derive = "Derive(\"%1\")";

						// Special type
						RegisterApp (commandApp.c_str (), "Georges Form Files", iconPath.c_str (), 0);
						RegisterAppCommand (commandApp.c_str (), "Open", (appPath+" /dde").c_str ());
						RegisterAppCommand (commandApp.c_str (), "Copy", (appPath+" /dde").c_str ());
						RegisterAppCommand (commandApp.c_str (), "Derive", (appPath+" /dde").c_str ());

						const char *notepad = "notepad.exe \"%1\"";
						RegisterAppCommand (commandApp.c_str (), "Open with notepad", notepad);

						RegisterDDECommand (commandApp.c_str (), "Open", open, theApp.m_pszExeName);
						RegisterDDECommand (commandApp.c_str (), "Copy", copy, theApp.m_pszExeName);
						RegisterDDECommand (commandApp.c_str (), "Derive", derive, theApp.m_pszExeName);
						RegisterShellFileExt (extLower.c_str (), commandApp.c_str ());
					}
					else
					{
						// Register with basic type
						RegisterShellFileExt (extLower.c_str (), "Georges.Form");

						// Unregister special type
						UnregisterApp (commandApp.c_str ());
					}
*/
				}

				updateDocumentStructure ();

				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
		catch (Exception &e)
		{
			char message[512];
			smprintf (message, 512, "Error while loading Type file: %s", e.what());
			theApp.outputError (message);
			return FALSE;
		}
	}

	return FALSE;
}

// ***************************************************************************

bool CGeorgesEditDoc::loadFormFile (const char *filename)
{
	// Read the form with the loader
	Form = FormLoader.loadForm (filename);
	if (!Form)
	{
		char msg[512];
		smprintf (msg, 512, "Error while loading Form file %s", filename);
		theApp.outputError (msg);
		return false;
	}
	return true;
}

// ***************************************************************************

bool CGeorgesEditDoc::addParent (const char *filename)
{
	nlassert (isForm());
	UForm *parent = FormLoader.loadForm (filename);
	if (!parent)
	{
		char msg[512];
		smprintf (msg, 512, "Can't load Form named '%s'", filename);
		theApp.outputError (msg);
		return false;
	}
	else
	{
		((CForm*)(UForm*)Form)->insertParent (((CForm*)(UForm*)Form)->getParentCount (), filename, (CForm*)parent);
		return true;
	}
}

// ***************************************************************************

void CGeorgesEditDoc::updateDocumentStructure ()
{
	RootObject.clean ();

	// Get left view
	CLeftView *leftView = getLeftView ();

	// Backup current selection
	uint selection = 0xffffffff;
	if (IsWindow (*leftView))
		selection = leftView->getCurrentSelectionId ();

	// Read the file
	if (isType())
	{
		// Setup sub object tree 
		RootObject.create (CGeorgesEditDocSub::Null, "Type", 0xffffffff, "NULL", 0xffffffff);
		RootObject.add (CGeorgesEditDocSub::Header, "Header", 0xffffffff, "NULL", 0xffffffff);
		RootObject.add (CGeorgesEditDocSub::Type, "Content", 0xffffffff, "NULL", 0xffffffff);
	}
	else if (isDfn ())
	{
		// Setup sub object tree 
		RootObject.create (CGeorgesEditDocSub::Null, "Dfn", 0xffffffff, "NULL", 0xffffffff);
		RootObject.add (CGeorgesEditDocSub::Header, "Header", 0xffffffff, "NULL", 0xffffffff);
		RootObject.add (CGeorgesEditDocSub::Dfn, "Content", 0xffffffff, "NULL", 0xffffffff);
	}
	else
	{
		// Setup sub object tree 
		RootObject.create (CGeorgesEditDocSub::Null, "Form", 0xffffffff, "NULL", 0xffffffff);
		RootObject.add (CGeorgesEditDocSub::Header, "Header", 0xffffffff, "NULL", 0xffffffff);

		// Get the parents
		CFormElmStruct *rootstruct = &((CForm*)(UForm*)Form)->Elements;
		addStruct (&RootObject, rootstruct, rootstruct->FormDfn, "Content", 0xffffffff, "", 0);

		// Get held objects

		// Write held elements
		uint i;
		for (i=0; i<CForm::HeldElementCount; i++)
		{
			rootstruct = ((CForm*)(UForm*)Form)->HeldElements[i];
			addStruct (&RootObject, rootstruct, rootstruct->FormDfn, ("Hold " + toString (1+i)).c_str (), 0xffffffff, "", i+1);
		}
	}

	// Update left view structure
	leftView->getFromDocument ();

	// Set the old selction flag
	if (selection != 0xffffffff)
	{
		// Reselect backuped node
		leftView->setCurrentSelectionId (selection);
	}
	else
	{
		// Select the content
		leftView->setCurrentSelectionId (2);
	}
}

// ***************************************************************************

BOOL CGeorgesEditDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	// Get focus
	CWnd *focus = CWnd::GetFocus ();
	::SetFocus (NULL);

	// Open the filt
	COFile file;
	if (file.open (lpszPathName))
	{
		try
		{
			// Xml stream
			COXml xmlStream;
			xmlStream.init (&file);

			if (isType())
			{
				nlassert (Type != NULL);
			
				// Write the file
				// Modified ?
				if (IsModified ())
				{
					Type->Header.MinorVersion++;
					flushValueChange ();
				}
				Type->write (xmlStream.getDocument (), theApp.Georges4CVS);
				modify (NULL, NULL, false);
				flushValueChange ();
				UpdateAllViews (NULL);
				return TRUE;
			}
			else if (isDfn ())
			{
				nlassert (Dfn != NULL);

				// Write the file
				if (IsModified ())
				{
					Dfn->Header.MinorVersion++;
					flushValueChange ();
				}
				Dfn->write (xmlStream.getDocument (), lpszPathName, theApp.Georges4CVS);
				modify (NULL, NULL, false);
				UpdateAllViews (NULL);
				return TRUE;
			}
			else
			{
				nlassert (Form != NULL);

				// Write the file
				if (IsModified ())
				{
					((CForm*)(UForm*)Form)->Header.MinorVersion++;				
					flushValueChange ();
				}
				((CForm*)(UForm*)Form)->write (xmlStream.getDocument (), lpszPathName, theApp.Georges4CVS);
				if (strcmp (xmlStream.getErrorString (), "") != 0)
				{
					char message[512];
					smprintf (message, 512, "Error while saving file: %s", xmlStream.getErrorString ());
					theApp.outputError (message);
				}
				modify (NULL, NULL, false);
				flushValueChange ();
				UpdateAllViews (NULL);

				// Get the left view
				CView* pView = getLeftView ();

				return TRUE;
			}

		}
		catch (Exception &e)
		{
			char message[512];
			smprintf (message, 512, "Error while loading file: %s", e.what());
			theApp.outputError (message);
			return FALSE;
		}
	}
	else
	{
		char message[512];
		smprintf (message, 512, "Can't open the file %s for writing.", lpszPathName);
		theApp.outputError (message);
		return FALSE;
	}

	// Set focus
	focus->SetFocus ();

	return FALSE;
	//return CDocument::OnSaveDocument(lpszPathName);
}

void CGeorgesEditDoc::SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU)
{
	CDocument::SetPathName(lpszPathName, bAddToMRU);

	// Notify the plugin
	uint i;
	for (i=0; i<PluginArray.size (); i++)
	{
		// Activate / desactivate plugin
		PluginArray[i].PluginInterface->activate(false);
		PluginArray[i].PluginInterface->activate(true);
	}

}


// ***************************************************************************

bool CGeorgesEditDoc::modify (class IAction *action, bool modified, bool setHeaderStateToModified)
{
	if (!NoModification)
	{
		if (modified)
		{
			// Do the modification
			bool reallyModified;
			if ((action == NULL) || action->doAction (*this, true, reallyModified, true))
			{
				// Action has modified the docment ?
				if (reallyModified)
				{
					// Is modified
					setModifiedState (true);

					if (setHeaderStateToModified)
					{
						if (isType())
							Type->Header.State = CFileHeader::Modified;
						if (isDfn ())
							Dfn->Header.State = CFileHeader::Modified;
						if (isForm())
							((CForm*)(UForm*)Form)->Header.State = CFileHeader::Modified;
					}

					// Add the action
					if (action)
					{
						_UndoBuffer.push_back (action);
						_UndoModify++;
						clearRedo ();

						if (_UndoModify == 0)
						{
							setModifiedState (false);
						}

						// Resize the undo buffer
						nlassert (theApp.MaxUndo > 0);
						if (_UndoBuffer.size () > theApp.MaxUndo)
						{
							// Number of element to remove from undo list
							uint toRemove = _UndoBuffer.size () - theApp.MaxUndo;

							// Delete each elements
							uint i;
							for (i=0; i<toRemove; i++)
								delete _UndoBuffer[i];

							// Resize the array
							_UndoBuffer.erase (_UndoBuffer.begin(), _UndoBuffer.begin()+toRemove);
						}
					}
				}
				else
				{
					delete action;
				}

				return true;
			}
			else
				return false;
		}
		else
		{
			// Is modified
			setModifiedState (false);
			_UndoModify = 0;
		}
	}
	return true;
}

// ***************************************************************************

CView* CGeorgesEditDoc::switchToView(CView* pNewView)
{
	CMDIFrameWnd* pMainWnd = (CMDIFrameWnd*)AfxGetMainWnd();

	// Get the active MDI child window.
	CMDIChildWnd* pChild = (CMDIChildWnd*)pMainWnd->MDIGetActive();

	// Get the active view attached to the active MDI child window.
	CView* pOldActiveView = pChild->GetActiveView();

	pChild->RecalcLayout();
	pNewView->UpdateWindow();
	pChild->SetActiveView(pNewView);

	return pOldActiveView;
}

// ***************************************************************************

CGeorgesEditDocSub *CGeorgesEditDoc::getSelectedObject ()
{
	CLeftView *leftView = getLeftView ();
	return leftView->getSelectedObject ();
}

// ***************************************************************************

void CGeorgesEditDoc::changeSubSelection (CGeorgesEditDocSub *subSelection, CView *view)
{
	if (subSelection == NULL)
		getLeftView ()->changeSubSelection (RootObject.getChild (1));
	else
		getLeftView ()->changeSubSelection (subSelection);

	// Save modified state
	NoModification = true;

	UpdateAllViews (view);

	// Save modified state
	NoModification = false;

	// Notify the plugin
	uint i;
	for (i=0; i<PluginArray.size (); i++)
	{
		// Activate / desactivate plugin
		PluginArray[i].PluginInterface->onNodeChanged ();
	}
}

// ***************************************************************************

void CGeorgesEditDoc::changeSubSelection (uint subSelection, CView *view)
{
	getLeftView ()->changeSubSelection (subSelection);

	// Save modified state
	NoModification = true;

	UpdateAllViews (view);

	// Save modified state
	NoModification = false;

	// Notify the plugin
	uint i;
	for (i=0; i<PluginArray.size (); i++)
	{
		// Activate / desactivate plugin
		PluginArray[i].PluginInterface->onNodeChanged ();
	}
}

// ***************************************************************************

CLeftView *CGeorgesEditDoc::getLeftView ()
{
	// Get the left view
	POSITION pos = GetFirstViewPosition ();
	if (pos)
	{
		CLeftView *pView = (CLeftView*)GetNextView(pos);
		nlassert (pView);
		return pView;
	}
	return NULL;
}

// ***************************************************************************

CGeorgesEditView *CGeorgesEditDoc::getRightView ()
{
	// Get the left view
	POSITION pos = GetFirstViewPosition ();
	if (pos)
	{
		nlverify (GetNextView (pos));
		CGeorgesEditView *pView = (CGeorgesEditView*)GetNextView(pos);
		nlassert (pView);
		return pView;
	}
	return NULL;
}

// ***************************************************************************

CFileHeader	*CGeorgesEditDoc::getHeaderPtr ()
{
	if (isType ())
		return &(Type->Header);
	if (isDfn ())
		return &(Dfn->Header);
	if (isForm ())
		return &(((CForm*)(UForm*)Form)->Header);
	nlstop;
	return NULL;
}

// ***************************************************************************

CType *CGeorgesEditDoc::getTypePtr ()
{
	return Type;
}

// ***************************************************************************

CFormDfn *CGeorgesEditDoc::getDfnPtr ()
{
	return Dfn;
}

// ***************************************************************************

CForm *CGeorgesEditDoc::getFormPtr ()
{
	return (CForm*)(UForm*)Form;
}

// ***************************************************************************

UForm* CGeorgesEditDoc::getForm ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return Form;
}

// ***************************************************************************

void CGeorgesEditDoc::getDfnFilename (std::string &dfnName)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Get the DFN filename
	CString str = GetPathName ();
	char extension[512];
	_splitpath (str, NULL, NULL, NULL, extension);
	dfnName = extension+1;
	dfnName += ".dfn";
}

// ***************************************************************************

bool CGeorgesEditDoc::getActiveNode (std::string &dfnName)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CGeorgesEditDocSub *sub = getSelectedObject ();
	if (sub && (sub->getFormName () != "NULL"))
	{
		dfnName = getSelectedObject ()->getFormName ();
		return true;
	}
	else
	{
		return false;
	}
}

// ***************************************************************************

void CGeorgesEditDoc::refreshView ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	getRightView ()->PostMessage (WM_UPDATE_ALL_VIEWS, 0, 0);
}

// ***************************************************************************

void CGeorgesEditDoc::getFilename (std::string &pathname)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	pathname = (const char*)GetPathName ();
}

// ***************************************************************************

void CGeorgesEditDoc::getTitle (std::string &title)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	title = (const char*)GetTitle ();
}

// ***************************************************************************

void CGeorgesEditDoc::bind (NLGEORGES::IEditPlugin *plugin, IEditDocumentPlugin *docInterface)
{
	// Add the plugin to the list
	PluginArray.push_back (CPlugin (plugin, docInterface));

	docInterface->dialogInit (*getLeftView ());
	docInterface->activate (theApp.isPluginActivated (plugin));
}

// ***************************************************************************

void CGeorgesEditDoc::onActivateView (bool activate)
{
	uint i;
	for (i=0; i<PluginArray.size (); i++)
	{
		// Activate / desactivate plugin
		PluginArray[i].PluginInterface->activate (activate && theApp.isPluginActivated (PluginArray[i].Plugin));
	}
}

// ***************************************************************************

void CGeorgesEditDoc::notifyPlugins (const char *valueName)
{
	uint i;
	for (i=0; i<PluginArray.size (); i++)
	{
		// Activate / desactivate plugin
		PluginArray[i].PluginInterface->onValueChanged (valueName);
	}
}

// ***************************************************************************

void CGeorgesEditDoc::logValueChange (const char *valueName, const char *newValue, bool present)
{
	if (present)
	{
		_LastLogs[valueName] = newValue;
	}
	else
	{
		_LastLogs.erase (valueName);
	}
}

// ***************************************************************************

void CGeorgesEditDoc::flushValueChange ()
{
	CFileHeader *header = NULL;
	if (isForm ())
		header = &((CForm*)(UForm*)Form)->Header;
	else if (isType ())
		header = &(Type->Header);
	else if (isDfn ())
		header = &(Dfn->Header);
	nlassert (header);

	std::map<std::string, std::string>::iterator ite = _LastLogs.begin ();
	while (ite != _LastLogs.end ())
	{
		header->addLog ((ite->first + " = " + ite->second).c_str ());
		ite++;
	}
	_LastLogs.clear ();
}

// ***************************************************************************

void CGeorgesEditDoc::clearUndo ()
{
	uint i;
	for (i=0; i<_UndoBuffer.size (); i++)
		delete _UndoBuffer[i];
	_UndoBuffer.clear ();
}

// ***************************************************************************

void CGeorgesEditDoc::clearRedo ()
{
	uint i;
	for (i=0; i<_RedoBuffer.size (); i++)
		delete _RedoBuffer[i];
	_RedoBuffer.clear ();
}

// ***************************************************************************

void CGeorgesEditDoc::OnUpdateFileSaveAll(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ();
}

// ***************************************************************************

void CGeorgesEditDoc::OnEditUndo() 
{
	if (!_UndoBuffer.empty ())
	{
		// Get an action
		IAction *action = _UndoBuffer.back ();
		_UndoBuffer.pop_back ();

		// Undo it
		bool modified;
		action->doAction (*this, false, modified, false);

		// Put in the redo list
		_RedoBuffer.push_back (action);

		CMDIFrameWnd* pMainWnd = (CMDIFrameWnd*)AfxGetMainWnd();

		// Get the active MDI child window.
		CMDIChildWnd* pChild = (CMDIChildWnd*)pMainWnd->MDIGetActive();
		pChild->RecalcLayout();
		pChild->UpdateWindow();

		_UndoModify--;
		if ( (_UndoModify == 0) && IsModified () )
		{
			setModifiedState (false);
		}
		else if ( (_UndoModify != 0) && !IsModified () )
		{
			setModifiedState (true);
		}
	}
}

// ***************************************************************************

void CGeorgesEditDoc::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (!_UndoBuffer.empty ());
}

// ***************************************************************************

void CGeorgesEditDoc::OnEditRedo() 
{
	if (!_RedoBuffer.empty ())
	{
		// Get an action
		IAction *action = _RedoBuffer.back ();
		_RedoBuffer.pop_back ();

		// Undo it
		bool modified;
		action->doAction (*this, true, modified, false);

		// Put in the redo list
		_UndoBuffer.push_back (action);

		((CMainFrame*)(theApp.m_pMainWnd))->RecalcLayout ();

		_UndoModify++;
		if ( (_UndoModify == 0) && IsModified () )
		{
			setModifiedState (false);
		}
		else if ( (_UndoModify != 0) && !IsModified () )
		{
			setModifiedState (true);
		}
	}
}

// ***************************************************************************

void CGeorgesEditDoc::OnUpdateEditRedo (CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (!_RedoBuffer.empty ());
}

// ***************************************************************************

void CGeorgesEditDoc::setModifiedState (bool modified)
{
	SetModifiedFlag (modified?TRUE:FALSE);

	if (modified)
	{
		CString title = GetTitle ();
		if ( (title.GetLength()<2) || (title[title.GetLength()-1] != '*') || (title[title.GetLength()-2] != ' ') )
			SetTitle (title+" *");
	}
	else
	{
		string title = (const char*)GetTitle ();
		if ( (title.size ()>=2) && (title[title.size()-1] == '*') && (title[title.size()-2] == ' ') )
		{
			title.resize (title.size () - 2);
			SetTitle (title.c_str());
		}
	}
}

// ***************************************************************************

void CGeorgesEditDoc::setValue (const char *value, const char *name, uint slot)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Document modified
	modify (new CActionString (IAction::FormValue, value, *this, name, "", 0xffffffff, slot));

	notifyPlugins (name);
}

// ***************************************************************************
// CGeorgesEditDocSub
// ***************************************************************************

CGeorgesEditDocSub::~CGeorgesEditDocSub ()
{
	clean ();
}

// ***************************************************************************

void CGeorgesEditDocSub::clean ()
{
	for (uint i=0; i<_Children.size(); i++)
		delete _Children[i];
	_Children.clear ();
}

// ***************************************************************************

bool CGeorgesEditDocSub::isEditable () const
{
	switch (_Type)
	{
	case Header:
	case Type:
	case Dfn:
	case Form:
		return true;
	}
	return false;
}

// ***************************************************************************

CGeorgesEditDocSub::TSub CGeorgesEditDocSub::getType () const
{
	return _Type;
}

// ***************************************************************************

uint CGeorgesEditDocSub::getChildrenCount ()
{
	return _Children.size ();
}

// ***************************************************************************

CGeorgesEditDocSub *CGeorgesEditDocSub::getChild (uint child)
{
	return _Children[child];
}

// ***************************************************************************

void CGeorgesEditDocSub::create (TSub type, const char *name, uint structId, const char *formName, uint slot)
{
	_StructId = structId;
	_Type = type;
	_Name = name;
	_FormName = formName;
	_Slot = slot;
}

// ***************************************************************************

CGeorgesEditDocSub *CGeorgesEditDocSub::add (TSub type, const char *name, uint structId, const char *formName, uint slot)
{
	// Add at the end
	uint index = _Children.size();
	_Children.push_back (new CGeorgesEditDocSub);

	_Children[index]->_Type = type;
	_Children[index]->_Name = name;
	_Children[index]->_Parent = this;
	_Children[index]->_StructId = structId;
	_Children[index]->_FormName = formName;
	_Children[index]->_Slot  = slot;
	return _Children[index];
}

// ***************************************************************************

CGeorgesEditDocSub::CGeorgesEditDocSub ()
{
	_Parent = NULL;
}

// ***************************************************************************

const std::string& CGeorgesEditDocSub::getName () const
{
	return _Name;
}

// ***************************************************************************

// ***************************************************************************

CGeorgesEditDocSub	*CGeorgesEditDocSub::getParent ()
{
	return _Parent;
}

// ***************************************************************************

uint CGeorgesEditDocSub::getIdInParent () const
{
	return _StructId;
}

// ***************************************************************************

const std::string& CGeorgesEditDocSub::getFormName () const
{
	return _FormName;
}

// ***************************************************************************

void CGeorgesEditDocSub::removeChildren (uint child)
{
	_Children.erase (_Children.begin()+child);
}

// ***************************************************************************

uint CGeorgesEditDocSub::getSlot () const
{
	return _Slot;
}

// ***************************************************************************

int CGeorgesEditDocSub::getItemImage (CGeorgesEditDoc *doc) const
{
	switch (_Type)
	{
	case Null:
		nlassert (_Children.size ()>1);
		if (_Children[1]->_Type == Type)
			return theApp.ImageList.getImage (IDR_TYPETYPE);
		if (_Children[1]->_Type == Dfn)
			return theApp.ImageList.getImage (IDR_TYPEDFN);
		if (_Children[1]->_Type == Form)
			return theApp.ImageList.getImage (IDR_TYPEFORM);
	case Header:
		{
			int image = theApp.ImageList.getImage ("header");
			if (image == -1)
				return theApp.ImageList.getImage (IDR_HEADER);
			return image;
		}
	case Type:
		{
			int image = theApp.ImageList.getImage ("type");
			if (image == -1)
				return theApp.ImageList.getImage (IDR_TYPETYPE);
			return image;
		}
	case Dfn:
		{
			int image = theApp.ImageList.getImage ("dfn");
			if (image == -1)
				return theApp.ImageList.getImage (IDR_TYPEDFN);
			return image;
		}
	case Form:
		{
			// Root ?
			if ((_Parent->_Parent == NULL) && (_Parent->_Children[1] != this))
			{
				int image = theApp.ImageList.getImage ("hold");
				if (image == -1)
					return theApp.ImageList.getImage (IDR_HOLD);
				return image;
			}

			if ((_Parent->_Parent == NULL) && (_Parent->_Children[1] == this))
			{
				int image = theApp.ImageList.getImage ("root");
				if (image == -1)
					return theApp.ImageList.getImage (IDR_ROOT);
				return image;
			}

			// What kind of node ?
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
			nlverify ( elm->getNodeByName (getFormName ().c_str (), &parentDfn, indexDfn, &nodeDfn, &nodeType, &node, type, array, parentVDfnArray, true, NLGEORGES_FIRST_ROUND) );
			
			if (array)
			{
				int image = theApp.ImageList.getImage ("array");
				if (image == -1)
					return theApp.ImageList.getImage (IDR_ARRAY);
				return image;
			}
			else
			{
				if (type == UFormDfn::EntryType)
				{
					if (parentDfn)
					{					
						int image = theApp.ImageList.getImage (parentDfn->getEntry(indexDfn).getFilename ().c_str ());
						if (image != -1)
							return image;
					}
					int image = theApp.ImageList.getImage ("typedoc");
					if (image == -1)
						return theApp.ImageList.getImage (IDR_TYPETYPE);
					return image;
				}
				else if (type == UFormDfn::EntryDfn)
				{
					if (parentDfn)
					{					
						int image = theApp.ImageList.getImage (parentDfn->getEntry(indexDfn).getFilename ().c_str ());
						if (image != -1)
							return image;
					}
					int image = theApp.ImageList.getImage ("struct");
					if (image == -1)
						return theApp.ImageList.getImage (IDR_STRUCT);
					return image;
				}
				else if (type == UFormDfn::EntryVirtualDfn)
				{
					if (node)
					{				
						string dfnName;
						safe_cast<CFormElmVirtualStruct*> (node)->getDfnName (dfnName);
						int image = theApp.ImageList.getImage (dfnName.c_str ());
						if (image != -1)
							return image;
					}
					int image = theApp.ImageList.getImage ("vstruct");
					if (image == -1)
						return theApp.ImageList.getImage (IDR_VSTRUCT);
					return image;
				}
			}
		}
	default:
		nlstop;
	}
	return 0;
}

// ***************************************************************************
// CPlugin
// ***************************************************************************

CGeorgesEditDoc::CPlugin::CPlugin (NLGEORGES::IEditPlugin *plugin, NLGEORGES::IEditDocumentPlugin *pluginInterface)
{
	Plugin = plugin;
	PluginInterface = pluginInterface;
}

void CGeorgesEditDocForm::OnEditFetch1() 
{
	fetch (1);
}

void CGeorgesEditDocForm::OnEditFetch3() 
{
	fetch (3);
}

void CGeorgesEditDocForm::OnEditFetch4() 
{
	fetch (1);
}

void CGeorgesEditDocForm::OnEditFetch2() 
{
	fetch (2);
}

void CGeorgesEditDocForm::OnEditHold1() 
{
	hold (1);
}

void CGeorgesEditDocForm::OnEditHold2() 
{
	hold (2);
}

void CGeorgesEditDocForm::OnEditHold3() 
{
	hold (3);
}

void CGeorgesEditDocForm::OnEditHold4() 
{
	hold (4);
}

void CGeorgesEditDocForm::fetch (uint buffer)
{
	if (theApp.yesNo (("Are you sure you want to get back hold buffer #" + toString (buffer) + " ?").c_str()))
	{
		theApp.SerialIntoMemStream ("", this, buffer, false);
		modify (new CActionBuffer (IAction::FormPaste, theApp.MemStream.buffer (), theApp.MemStream.length(), 
			*this, "", "", 2, 0), NULL);
	}
}

void CGeorgesEditDocForm::hold (uint buffer)
{
	theApp.SerialIntoMemStream ("", this, 0, false);
		modify (new CActionBuffer (IAction::FormPaste, theApp.MemStream.buffer (), theApp.MemStream.length(), 
			*this, "", "", 2, buffer), NULL);
}
