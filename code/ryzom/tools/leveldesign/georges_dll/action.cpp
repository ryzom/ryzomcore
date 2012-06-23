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
#include "left_view.h"

#include "nel/georges/type.h"
#include "nel/georges/form_dfn.h"
#include "nel/georges/form_elm.h"

#include "action.h"

using namespace std;
using namespace NLMISC;
using namespace NLGEORGES;

#define _OldValue (_Value[0])
#define _NewValue (_Value[1])

// ***************************************************************************

IAction::IAction (TTypeAction type, uint selId, uint slot)
{
	_Type = type;
	_SelId = selId;
	_Slot = slot;
}

// ***************************************************************************

void IAction::setLabel (const char *logLabel, CGeorgesEditDoc &doc)
{
	_LogLabel = logLabel;

	// New log present
	_LogPresent[1] = true;

	// Find log..
	std::map<std::string, std::string>::iterator ite = doc._LastLogs.find (logLabel);
	if (ite != doc._LastLogs.end ())
	{
		_LogPresent[0] = true;
		_Log[0] = logLabel;
	}
	else
	{
		_LogPresent[0] = false;
	}
}

// ***************************************************************************

bool IAction::doAction (CGeorgesEditDoc &doc, bool redo, bool &modified, bool firstTime)
{
	uint index = (uint)redo;
	doc.logValueChange (_LogLabel.c_str (), _Log[index].c_str (), _LogPresent[index]);
	return true;
}

// ***************************************************************************

void IAction::update (bool updateLeftView, TUpdateRightView rightViewFlag, CGeorgesEditDoc &doc, const char *_FormName)
{
	// Right and left view
	CGeorgesEditView *rightView = doc.getRightView ();
	nlassert (rightView);
	CLeftView *leftView = doc.getLeftView ();
	nlassert (leftView);

	// Update left view ?
	if (updateLeftView)
		doc.updateDocumentStructure ();

	// Set the current view..
	uint subSelection = leftView->getCurrentSelectionId ();
	if (subSelection != _SelId)
	{
		doc.changeSubSelection (_SelId, NULL);
		return;
	}

	if (leftView->getCurrentSelectionId () == 1)
	{
		rightView->HeaderDialog.getFromDocument (*doc.getHeaderPtr ());
	}
	else if (doc.isType ())
	{
		rightView->TypeDialog.getFromDocument (*(doc.getTypePtr()));
	}
	else if (doc.isDfn ())
	{
		rightView->DfnDialog.getFromDocument (*(doc.getDfnPtr()));
	}
	else if (doc.isForm ())
	{
		if (rightViewFlag == DoNothing)
		{
		}
		else if (rightViewFlag == UpdateLabels)
		{
			rightView->FormDialog.updateLabels ();
		}
		else if (rightViewFlag == UpdateValues)
		{
			rightView->FormDialog.updateValues ();
		}
		else if (rightViewFlag == Redraw)
		{
			rightView->FormDialog.getFromDocument ();
		}
	}
}

// ***************************************************************************

CActionString::CActionString (IAction::TTypeAction type, const char *newValue, CGeorgesEditDoc &doc, const char *formName, const char *userData, uint selId, uint slot) : IAction (type, selId, slot)
{
	// Set the new value
	_NewValue = newValue;
	_FormName = formName;
	_Log[1] = newValue;
	_UserData = userData;

	// Backup old value
	switch (_Type)
	{
	case TypeType:
		{
			CType *type = doc.getTypePtr ();
			_OldValue = toString ((int)(type->Type));
			setLabel ("Type Type", doc);
			_Log[1] = type->getTypeName ((UType::TType)atoi (newValue));
		}
		break;
	case TypeUI:
		{
			CType *type = doc.getTypePtr ();
			_OldValue = toString ((int)(type->UIType));
			setLabel ("Type UI", doc);
			_Log[1] = type->getUIName ((CType::TUI)atoi (newValue));
		}
		break;
	case TypeDefault:
		{
			CType *type = doc.getTypePtr ();
			_OldValue = type->Default;
			setLabel ("Type Default", doc);
		}
		break;
	case TypeMin:
		{
			CType *type = doc.getTypePtr ();
			_OldValue = type->Min;
			setLabel ("Type Min", doc);
		}
		break;
	case TypeMax:
		{
			CType *type = doc.getTypePtr ();
			_OldValue = type->Max;
			setLabel ("Type Max", doc);
		}
		break;
	case TypeIncrement:
		{
			CType *type = doc.getTypePtr ();
			_OldValue = type->Increment;
			setLabel ("Type Increment", doc);
		}
		break;
	case FormTypeValue:
	case FormValue:
		{
			// Form
			const NLGEORGES::CForm &form = *(doc.getFormPtr ());
			nlverify (doc.getRootNode (_Slot)->getValueByName (_OldValue, formName, UFormElm::NoEval, NULL));
			setLabel (formName, doc);
		}
		break;
	case HeaderVersion:
		{
			CFileHeader *header = doc.getHeaderPtr ();
			char versionText[512];
			smprintf (versionText, 512, "Version %d.%d", header->MajorVersion, header->MinorVersion);
			_OldValue = versionText;
			setLabel ("Header Version", doc);
		}
		break;
	case HeaderState:
		{
			CFileHeader *header = doc.getHeaderPtr ();
			_OldValue = toString ((int)(header->State)).c_str ();
			setLabel ("Header State", doc);
		}
		break;
	case HeaderComments:
		{
			_OldValue = doc.getHeaderPtr ()->Comments;
			setLabel ("Header Comments", doc);
		}
		break;
	case FormArrayRename:
		{
			setLabel ((formName+string (" Renamed")).c_str (), doc);
			int idInParent = atoi (_UserData.c_str ());

			// Get the parent node
			const CFormDfn *parentDfn;
			uint indexDfn;
			const CFormDfn *nodeDfn;
			const CType *nodeType;
			CFormElm *node;
			UFormDfn::TEntryType type;
			bool array;
			bool vdfnArray;
			CForm *form=doc.getFormPtr ();
			CFormElm *elm = doc.getRootNode (slot);
			nlverify ( elm->getNodeByName (_FormName.c_str (), &parentDfn, indexDfn, 
				&nodeDfn, &nodeType, &node, type, array, vdfnArray, true, NLGEORGES_FIRST_ROUND) );
			if (node)
			{
				CFormElmArray* array = safe_cast<CFormElmArray*> (node->getParent ());
				_OldValue = array->Elements[idInParent].Name;
			}
		}
		break;
	case FormArrayInsert:
		{
			// do nothing
			setLabel ("Array Insert", doc);
		}
		break;
	default:
		nlstop;
	}
}

// ***************************************************************************

bool CActionString::doAction (CGeorgesEditDoc &doc, bool redo, bool &modified, bool firstTime)
{
	IAction::doAction (doc, redo, modified, firstTime);

	modified = false;
	bool ok = true;
	uint index = (uint)redo;
	switch (_Type)
	{
	case TypeType:
		{
			CType *type = doc.getTypePtr ();
			type->Type = (CType::TType)(atoi (_Value[index].c_str ()));
			modified = true;
			update (false, Redraw, doc, _FormName.c_str ());
		}
		break;
	case TypeUI:
		{
			CType *type = doc.getTypePtr ();
			type->UIType = (CType::TUI)(atoi (_Value[index].c_str ()));
			modified = true;
			update (false, Redraw, doc, _FormName.c_str ());
		}
		break;
	case TypeDefault:
		{
			CType *type = doc.getTypePtr ();
			type->Default = _Value[index];
			modified = true;
			if (!firstTime)
				update (false, Redraw, doc, _FormName.c_str ());
		}
		break;
	case TypeMin:
		{
			CType *type = doc.getTypePtr ();
			type->Min = _Value[index];
			modified = true;
			if (!firstTime)
				update (false, Redraw, doc, _FormName.c_str ());
		}
		break;
	case TypeMax:
		{
			CType *type = doc.getTypePtr ();
			type->Max = _Value[index];
			modified = true;
			if (!firstTime)
				update (false, Redraw, doc, _FormName.c_str ());
		}
		break;
	case TypeIncrement:
		{
			CType *type = doc.getTypePtr ();
			type->Increment = _Value[index];
			modified = true;
			if (!firstTime)
				update (false, Redraw, doc, _FormName.c_str ());
		}
		break;
	case FormTypeValue:
	case FormValue:
		{
			// Empty ?
			if (!_Value[index].empty ())
			{
				// Get / create the node
				const CFormDfn *parentDfn;
				uint indexDfn;
				const CFormDfn *nodeDfn;
				const CType *nodeType;
				CFormElm *node;
				UFormDfn::TEntryType type;
				bool array;
				bool created;
				CForm *form=doc.getFormPtr ();
				CFormElm *elm = doc.getRootNode (_Slot);
				nlverify ( elm->createNodeByName (_FormName.c_str (), &parentDfn, indexDfn, 
					&nodeDfn, &nodeType, &node, type, array, created) );
				nlassert (node);

				// Set the atom value
				CFormElmAtom *atom = safe_cast<CFormElmAtom*> (node);
				atom->setValue (_Value[index].c_str ());
				modified = true;

				if (firstTime)
					update (created, UpdateLabels, doc, _FormName.c_str ());
				else
					update (created, UpdateValues, doc, _FormName.c_str ());
			}
			else
			{
				if (FormTypeValue == _Type)
				{
					// Get the node
					const CFormDfn *parentDfn;
					uint indexDfn;
					const CFormDfn *nodeDfn;
					const CType *nodeType;
					CFormElm *node;
					UFormDfn::TEntryType type;
					bool array;
					bool parentVDnfArray;
					CForm *form=doc.getFormPtr ();
					CFormElm *elm = doc.getRootNode (_Slot);
					nlverify ( elm->getNodeByName (_FormName.c_str (), &parentDfn, indexDfn, 
						&nodeDfn, &nodeType, &node, type, array, parentVDnfArray, true, NLGEORGES_FIRST_ROUND) );
					nlassert (node);
					CFormElmAtom *atom = safe_cast<CFormElmAtom*> (node);
					atom->setValue ("");
				}
				else
				{
					// Remove the node
					const CFormDfn *parentDfn;
					uint indexDfn;
					const CFormDfn *nodeDfn;
					const CType *nodeType;
					CFormElm *node;
					UFormDfn::TEntryType type;
					bool array;
					CForm *form=doc.getFormPtr ();
					CFormElm *elm = doc.getRootNode (_Slot);
					nlverify ( elm->deleteNodeByName (_FormName.c_str (), &parentDfn, indexDfn, 
						&nodeDfn, &nodeType, &node, type, array) );
					nlassert ((FormTypeValue == _Type)||(node == NULL));
				}

				modified = true;

				update (true, UpdateValues, doc, _FormName.c_str ());
			}
		}
		break;
	case HeaderVersion:
		{
			uint v0, v1;
			if (sscanf (_Value[index].c_str (), "Version %d.%d", &v0, &v1)==2)
			{
				CFileHeader *header = doc.getHeaderPtr ();
				header->MajorVersion = v0;
				header->MinorVersion = v1;
				modified = true;
				update (false, Redraw, doc, _FormName.c_str ());
			}
		}
		break;
	case HeaderState:
		{
			CFileHeader *header = doc.getHeaderPtr ();
			header->State = (CFileHeader::TState)atoi (_Value[index].c_str ());
			modified = true;
			update (false, Redraw, doc, _FormName.c_str ());
		}
		break;
	case HeaderComments:
		{
			doc.getHeaderPtr ()->Comments = _Value[index];
			modified = true;
			if (!firstTime)
				update (false, Redraw, doc, _FormName.c_str ());
		}
		break;
	case FormArrayRename:
		{
			int idInParent = atoi (_UserData.c_str ());

			// Get the parent node
			const CFormDfn *parentDfn;
			uint indexDfn;
			const CFormDfn *nodeDfn;
			const CType *nodeType;
			CFormElm *node;
			UFormDfn::TEntryType type;
			bool array;
			bool vdfnArray;
			CForm *form=doc.getFormPtr ();
			CFormElm *elm = doc.getRootNode (_Slot);
			nlverify ( elm->getNodeByName (_FormName.c_str (), &parentDfn, indexDfn, 
				&nodeDfn, &nodeType, &node, type, array, vdfnArray, true, NLGEORGES_FIRST_ROUND) );
			if (node)
			{
				CFormElmArray* array = safe_cast<CFormElmArray*> (node->getParent ());
				array->Elements[idInParent].Name = _Value[index];
				modified = true;
				update (true, DoNothing, doc, _FormName.c_str ());
			}
		}
		break;
	case FormArrayInsert:
		{
			int idInParent = atoi (_NewValue.c_str ());

			// Insert ?
			if (redo)
			{
				// Get the parent node
				const CFormDfn *parentDfn;
				uint indexDfn;
				const CFormDfn *nodeDfn;
				const CType *nodeType;
				CFormElm *parentNode;
				UFormDfn::TEntryType type;
				bool array;
				CForm *form=doc.getFormPtr ();
				CFormElm *elm = doc.getRootNode (_Slot);
				nlverify ( elm->arrayInsertNodeByName (_FormName.c_str (), &parentDfn, indexDfn, 
					&nodeDfn, &nodeType, &parentNode, type, array, true, idInParent) );
				modified = true;
			}
			else
			{
				// Get the parent node
				const CFormDfn *parentDfn;
				uint indexDfn;
				const CFormDfn *nodeDfn;
				const CType *nodeType;
				CFormElm *parentNode;
				UFormDfn::TEntryType type;
				bool array;
				CForm *form=doc.getFormPtr ();
				CFormElm *elm = doc.getRootNode (_Slot);
				nlverify ( elm->arrayDeleteNodeByName (_FormName.c_str (), &parentDfn, indexDfn, 
					&nodeDfn, &nodeType, &parentNode, type, array, true, idInParent) );
				modified = true;
			}

			update (true, Redraw, doc, _FormName.c_str ());
		}
		break;
	default:
		nlstop;
	}

	return ok;
}

// ***************************************************************************

CActionStringVector::CActionStringVector (IAction::TTypeAction type, const std::vector<std::string> &stringVector, CGeorgesEditDoc &doc, const char *formName, uint selId, uint slot) : IAction (type, selId, slot)
{
	// Set the new value
	_FormName = formName;
	_NewValue = stringVector;

	// Backup old value
	switch (_Type)
	{
	case DfnParents:
		{
			// Dfn
			const NLGEORGES::CFormDfn &dfn = *(doc.getDfnPtr ());

			// Add the parents
			_OldValue.resize (dfn.getNumParent ());
			uint parent;
			for (parent=0; parent<dfn.getNumParent (); parent++)
			{
				// Add the label and value
				_OldValue[parent] = dfn.getParentFilename (parent);
			}
			setLabel ("Dfn Parents", doc);
		}
		break;
	case FormParents:
		{
			// Get the form
			const NLGEORGES::CForm &form = *(doc.getFormPtr ());

			// Resize old string array
			_OldValue.resize (form.getParentCount ());

			// For each parent
			uint parent;
			for (parent=0; parent<form.getParentCount (); parent++)
			{
				// Get the parent filename
				_OldValue[parent] = form.getParentFilename (parent);
			}
			setLabel ("Form Parents", doc);
		}
		break;
/*	case FormArrayReplace:
	case FormArrayAppend:
		{
			// Get the form
			const NLGEORGES::CForm &form = *(doc.getFormPtr ());

			// Get the node
			const CFormDfn *parentDfn;
			uint indexDfn;
			const CFormDfn *nodeDfn;
			const CType *nodeType;
			CFormElm *node;
			UFormDfn::TEntryType type;
			bool array;
			bool parentVDfnArray;
			CFormElm *elm = doc.getRootNode (slot);
			nlverify ( elm->getNodeByName (formName, &parentDfn, indexDfn, &nodeDfn, &nodeType, &node, type, array, parentVDfnArray, true) );

			// Get the atom
			_OldValue.clear ();
			if (node)
			{
				// Get the atom
				CFormElmArray *arrayPtr = safe_cast<CFormElmArray*>(node);

				uint size;
				nlverify (arrayPtr->getArraySize (size));
				_OldValue.resize (size);
				uint elm;
				for (elm=0; elm<_OldValue.size (); elm++)
				{
					if (arrayPtr->Elements[elm])
					{
						CFormElmAtom *atom = safe_cast<CFormElmAtom*>(arrayPtr->Elements[elm]);
						atom->getValue (_OldValue[elm], false);
					}
				}
			}
			if (_Type == FormArrayReplace)
				setLabel ("Form Array Replace", doc);
			else 
				setLabel ("Form Array Append", doc);
		}
		break;*/
	default:
		nlstop;
	}
}

// ***************************************************************************

bool CActionStringVector::doAction (CGeorgesEditDoc &doc, bool redo, bool &modified, bool firstTime)
{
	IAction::doAction (doc, redo, modified, firstTime);

	modified = false;
	bool ok = true;
	uint index = (uint)redo;
	switch (_Type)
	{
	case DfnParents:
		{
			// Get document pointer
			CFormDfn *dfn = doc.getDfnPtr ();
			nlassert (dfn);

			// Add the parents
			dfn->setNumParent (_Value[index].size ());

			uint parent;
			for (parent=0; parent<_Value[index].size (); parent++)
			{
				try
				{
					// Set the parent
					dfn->setParent (parent, doc.FormLoader, _Value[index][parent].c_str ());

					modified = true;
				}
				catch (Exception &e)
				{
					ok = false;
					char message[512];
					smprintf (message, 512, "Error while loading Dfn file (%s): %s", _Value[index][parent].c_str (), e.what());
					theApp.outputError (message);

					// Next parent
					parent--;
				}
			}
			modified = true;
			if (!firstTime)
				update (false, Redraw, doc, "");
		}
		break;
	case FormParents:
		{
			// Get the form
			NLGEORGES::CForm &form = *(doc.getFormPtr ());

			// Remove parent
			form.clearParents ();

			// Get the result value
			uint count = _Value[index].size ();
			uint value;
			for (value = 0; value<count; value++)
			{
				// Load the parent
				if (!_Value[index].empty ())
				{
					// Try to load the form
					NLMISC::CSmartPtr<CForm> parentForm = (CForm*)doc.FormLoader.loadForm (_Value[index][value].c_str ());
					if (parentForm)
					{
						if ((&form) != parentForm)
						{
							// Check it is the same dfn
							if (parentForm->Elements.FormDfn == form.Elements.FormDfn)
							{
								// This is the parent form selector
								if (form.insertParent (value, _Value[index][value].c_str(), parentForm))
								{
								}
								else
								{
									ok = false;
									char msg[512];
									smprintf (msg, 512, "Internal error while assign the form (%s) as parent.", _Value[index][value].c_str());
									theApp.outputError (msg);
								}
							}
							else
							{
								ok = false;
								char msg[512];
								smprintf (msg, 512, "The parent form (%s) doesn't use the same DFN than the current form.", _Value[index][value].c_str());
								theApp.outputError (msg);
							}
						}
						else
						{
							ok = false;
							char msg[512];
							smprintf (msg, 512, "Can't assign a form it self as parent.");
							theApp.outputError (msg);
						}
					}
					else
					{
						ok = false;
						char msg[512];
						smprintf (msg, 512, "Can't read the form %s.", _Value[index][value].c_str());
						theApp.outputError (msg);
					}
				}
			}
			modified = true;
			update (true, Redraw, doc, _FormName.c_str ());
		}
		break;
	/*case FormArrayReplace:
	case FormArrayAppend:
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
			CForm *form=doc.getFormPtr ();
			CFormElm *elm = doc.getRootNode (slot);
			nlverify ( elm->getNodeByName (_FormName.c_str (), &parentDfn, indexDfn, &nodeDfn, &nodeType, &node, type, array, parentVDfnArray, true) );

			// Is a type entry ?
			if ((type == UFormDfn::EntryType) && array)
			{
				// Create the array
				bool created;
				nlverify ( elm->createNodeByName (_FormName.c_str (), &parentDfn, indexDfn, &nodeDfn, &nodeType, &node, type, array, created) );
				nlassert (node);

				// Get the atom
				CFormElmArray *arrayPtr = safe_cast<CFormElmArray*>(node);

				// Replace ?
				if (_Type == FormArrayReplace)
				{
					arrayPtr->clean ();
				}

				// For each element
				uint arraySize = arrayPtr->Elements.size();
				for (uint i=0; i<_Value[index].size (); i++)
				{
					// Name
					char name[512];
					smprintf (name, 512, "[%d]", i+arraySize);

					// Create the atom node
					nlverify ( arrayPtr->createNodeByName (name, &parentDfn, indexDfn, &nodeDfn, &nodeType, &node, type, array, created) );
					nlassert (node);

					// Get the atom
					CFormElmAtom *atom = safe_cast<CFormElmAtom*>(node);

					// Set the value
					atom->setValue (_Value[index][i].c_str());
				}

				modified = true;
				update (true, Redraw, doc, _FormName.c_str ());
			}
		}
		break;*/
	}

	return ok;
}

// ***************************************************************************

CActionStringVectorVector::CActionStringVectorVector (IAction::TTypeAction type, const std::vector<std::vector<std::string> > &stringVector, 
													  CGeorgesEditDoc &doc, uint selId, uint slot) : IAction (type, selId, slot)
{
	// Set the new value
	_NewValue = stringVector;

	// Backup old value
	switch (_Type)
	{
	case DfnStructure:
		{
			// Dfn
			const NLGEORGES::CFormDfn &dfn = *(doc.getDfnPtr ());

			// Add the struct element
			_OldValue.resize (dfn.getNumEntry ());
			uint elm;
			for (elm=0; elm<_OldValue.size (); elm++)
			{
				// Resize the entry
				_OldValue[elm].resize (5);

				// Add the label and value
				_OldValue[elm][0] = dfn.getEntry (elm).getName ();
				switch (elm, dfn.getEntry (elm).getType ())
				{
				case UFormDfn::EntryType:
					_OldValue[elm][1] = dfn.getEntry (elm).getArrayFlag () ? "Type array" : "Type";
					_OldValue[elm][4] = dfn.getEntry (elm).getFilenameExt ();
					break;
				case UFormDfn::EntryDfn:
					_OldValue[elm][1] = dfn.getEntry (elm).getArrayFlag () ? "Dfn array" : "Dfn";
					break;
				case UFormDfn::EntryVirtualDfn:
					_OldValue[elm][1] = "Virtual Dfn";
					break;
				}
				_OldValue[elm][2] = dfn.getEntry (elm).getFilename ();
				_OldValue[elm][3] = dfn.getEntry (elm).getDefault ();
			}
			setLabel ("Dfn Structure", doc);
		}
		break;
	case TypePredef:
		{
			// Type
			const NLGEORGES::CType &type = *(doc.getTypePtr ());

			uint predef;
			_OldValue.resize (type.Definitions.size());
			for (predef=0; predef<_OldValue.size(); predef++)
			{
				// Add the label and value
				_OldValue[predef].resize (2);
				_OldValue[predef][0] = type.Definitions[predef].Label;
				_OldValue[predef][1] = type.Definitions[predef].Value;
			}
			setLabel ("Type Predef", doc);
		}
		break;
	}
}

// ***************************************************************************

bool CActionStringVectorVector::doAction (CGeorgesEditDoc &doc, bool redo, bool &modified, bool firstTime)
{
	IAction::doAction (doc, redo, modified, firstTime);

	modified = false;
	bool ok = true;
	uint index = (uint)redo;

	// Backup old value
	switch (_Type)
	{
	case DfnStructure:
		{
			// Dfn
			NLGEORGES::CFormDfn &dfn = *(doc.getDfnPtr ());

			// Add the entries
			dfn.setNumEntry (_Value[index].size ());
			uint elm;
			for (elm=0; elm<_Value[index].size (); elm++)
			{
				// Ref on the entry
				CFormDfn::CEntry &entry = dfn.getEntry (elm);

				// Get the name
				entry.setName (_Value[index][elm][0].c_str ());

				// Get the filename
				string &filename = _Value[index][elm][2];

				// Get the type
				string &type= _Value[index][elm][1];
				if ((type == "Type") || (type == "Type array"))
				{
					// Set the type
					entry.setType (doc.FormLoader, filename.c_str ());

					// Set the default value
					string &def = _Value[index][elm][3];
					entry.setDefault (def.c_str ());

					// Set the default extension
					string &ext = _Value[index][elm][4];
					entry.setFilenameExt (ext.c_str ());
				}
				else if ((type == "Dfn") || (type == "Dfn array"))
				{
					// Set the type
					entry.setDfn (doc.FormLoader, filename.c_str ());
				}
				else if (type == "Virtual Dfn")
				{
					// Set the type
					entry.setDfnPointer ();
				}
				entry.setArrayFlag ((type == "Type array") || (type == "Dfn array"));
			}
			modified = true;
			if (!firstTime)
				update (false, Redraw, doc, "");
		}
		break;
	case TypePredef:
		{
			// Type
			CType &type = *(doc.getTypePtr ());

			// Add the predef
			type.Definitions.resize (_Value[index].size ());
			uint predef;
			for (predef=0; predef<type.Definitions.size(); predef++)
			{
				// Add the label and value
				type.Definitions[predef].Label = _Value[index][predef][0];
				type.Definitions[predef].Value = _Value[index][predef][1];
			}
			modified = true;
			if (!firstTime)
				update (false, Redraw, doc, "");
		}
		break;
	default:
		nlstop;
	}

	return ok;
}

// ***************************************************************************

CActionBuffer::CActionBuffer (IAction::TTypeAction type, const uint8 *buffer, uint bufferSize, CGeorgesEditDoc &doc, const char *formName, const char *userData, uint selId, uint slot) : IAction (type, selId, slot)
{
	// New value
	_FormName = formName;
	_UserData = userData;

	// Backup old value
	switch (_Type)
	{
	case FormPaste:
		{
			// Backup buffer
			_NewValue.resize (bufferSize);
			memcpy (&_NewValue[0], buffer, bufferSize);

			// Serial formname in the memBuffer
			nlverify (theApp.SerialIntoMemStream (formName, &doc, _Slot, false));
			_OldValue.resize (theApp.MemStream.length());
			memcpy (&_OldValue[0], theApp.MemStream.buffer (), theApp.MemStream.length());
			setLabel (("formName"+string (" Pasted")).c_str (), doc);
		}
		break;
	case FormArrayDelete:
		{
			// Serial formname in the memBuffer
			nlverify (theApp.SerialIntoMemStream (formName, &doc, _Slot, false));
			_OldValue.resize (theApp.MemStream.length());
			memcpy (&_OldValue[0], theApp.MemStream.buffer (), theApp.MemStream.length());
			setLabel (("formName"+string (" Deleted")).c_str (), doc);
		}
		break;
	case FormArraySize:
		{
			// Serial formname in the memBuffer
			nlverify (theApp.SerialIntoMemStream (formName, &doc, _Slot, false));
			_OldValue.resize (theApp.MemStream.length());
			memcpy (&_OldValue[0], theApp.MemStream.buffer (), theApp.MemStream.length());
			setLabel (("formName"+string (" Resized")).c_str (), doc);
			_Log[1] = _UserData;
		}
		break;
	case FormVirtualDfnName:
		{
			// Serial formname in the memBuffer
			nlverify (theApp.SerialIntoMemStream (formName, &doc, _Slot, false));
			_OldValue.resize (theApp.MemStream.length());
			memcpy (&_OldValue[0], theApp.MemStream.buffer (), theApp.MemStream.length());
			setLabel (formName, doc);
			_Log[1] = _UserData;
		}
		break;
	default:
		nlstop;
	}	
}

// ***************************************************************************

bool CActionBuffer::doAction (CGeorgesEditDoc &doc, bool redo, bool &modified, bool firstTime)
{
	IAction::doAction (doc, redo, modified, firstTime);

	modified = false;
	bool ok = true;
	uint index = (uint)redo;

	// Backup old value
	switch (_Type)
	{
	case FormPaste:
		{
			// Fill memstream with the new buffer
			theApp.FillMemStreamWithBuffer (&_Value[index][0], _Value[index].size ());

			// Reserial the document
			nlverify (theApp.SerialFromMemStream (_FormName.c_str (), &doc, _Slot));
			modified = true;
			update (true, Redraw, doc, _FormName.c_str ());
		}
		break;
	case FormArrayDelete:
		{
			if (redo)
			{
				// Get the parent node
				const CFormDfn *parentDfn;
				uint indexDfn;
				const CFormDfn *nodeDfn;
				const CType *nodeType;
				CFormElm *parentNode;
				UFormDfn::TEntryType type;
				bool array;
				CForm *form=doc.getFormPtr ();
				CFormElm *elm = doc.getRootNode (_Slot);
				nlverify ( elm->arrayDeleteNodeByName (_FormName.c_str (), &parentDfn, indexDfn, 
					&nodeDfn, &nodeType, &parentNode, type, array, true, atoi (_UserData.c_str ())) );
				modified = true;
			}
			else
			{
				// Insert a node
				const CFormDfn *parentDfn;
				uint indexDfn;
				const CFormDfn *nodeDfn;
				const CType *nodeType;
				CFormElm *parentNode;
				UFormDfn::TEntryType type;
				bool array;
				CForm *form=doc.getFormPtr ();
				CFormElm *elm = doc.getRootNode (_Slot);
				nlverify ( elm->arrayInsertNodeByName (_FormName.c_str (), &parentDfn, indexDfn, 
					&nodeDfn, &nodeType, &parentNode, type, array, true, atoi (_UserData.c_str ())) );

				// Paste the node

				// Fill memstream with the new buffer
				theApp.FillMemStreamWithBuffer (&_OldValue[0], _OldValue.size ());

				// Reserial the document
				nlverify (theApp.SerialFromMemStream (_FormName.c_str (), &doc, _Slot));
				modified = true;
			}

			update (true, Redraw, doc, _FormName.c_str ());
		}
		break;
	case FormArraySize:
		{
			if (redo)
			{
				// Get the size
				int size = 0;
				if ( _UserData.empty() || (sscanf (_UserData.c_str(), "%d", &size) == 1) )
				{
					if (size < 0)
						size = 0;
					
					// Array exist ?
					if (size > 0)
					{
						// Get / create the node
						const CFormDfn *parentDfn;
						uint indexDfn;
						const CFormDfn *nodeDfn;
						const CType *nodeType;
						CFormElm *node;
						UFormDfn::TEntryType type;
						bool array;
						bool created;
						CForm *form=doc.getFormPtr ();
						CFormElm *elm = doc.getRootNode (_Slot);
						nlverify ( elm->createNodeByName (_FormName.c_str (), &parentDfn, indexDfn, 
							&nodeDfn, &nodeType, &node, type, array, created) );
						nlassert (node);

						// Get the array node
						CFormElmArray *arrayPtr = safe_cast<CFormElmArray*> (node);

						// Backup old size
						uint oldSize = arrayPtr->Elements.size ();

						// Erase old element
						uint i;
						for (i=size; i<oldSize; i++)
						{
							// Delete the element
							if (arrayPtr->Elements[i].Element)
								delete arrayPtr->Elements[i].Element;
						}

						// Resize the array
						arrayPtr->Elements.resize (size);

						// Insert element
						for (i=oldSize; i<(uint)size; i++)
						{
							// Create empty sub node
							char index[512];
							smprintf (index, 512, "[%d]", i);
							nlverify ( arrayPtr->createNodeByName (index, &parentDfn, indexDfn, 
								&nodeDfn, &nodeType, &node, type, array, created) );
						}

						// Document modified
						modified = true;
						update (true, UpdateValues, doc, _FormName.c_str ());
					}
					else
					{
						// Remove the node
						const CFormDfn *parentDfn;
						uint indexDfn;
						const CFormDfn *nodeDfn;
						const CType *nodeType;
						CFormElm *node;
						UFormDfn::TEntryType type;
						bool array;
						CForm *form=doc.getFormPtr ();
						CFormElm *elm = doc.getRootNode (_Slot);
						nlverify ( elm->deleteNodeByName (_FormName.c_str (), &parentDfn, indexDfn, 
							&nodeDfn, &nodeType, &node, type, array) );
						nlassert ( (node == NULL) || (node->getForm () != doc.getFormPtr ()) );

						// Document modified
						modified = true;
						update (true, UpdateValues, doc, _FormName.c_str ());
					}
				}
			}
			else
			{
				// Fill memstream with the new buffer
				theApp.FillMemStreamWithBuffer (&_OldValue[0], _OldValue.size ());

				// Reserial the document
				nlverify (theApp.SerialFromMemStream (_FormName.c_str (), &doc, _Slot));
				modified = true;
				update (true, UpdateValues, doc, _FormName.c_str ());
			}
		}
		break;
	case FormVirtualDfnName:
		{
			if (redo)
			{
				// Result ok ?
				if (!_UserData.empty())
				{
					// Try to load the DFN
					CSmartPtr<CFormDfn> newDfn = doc.FormLoader.loadFormDfn (_UserData.c_str (), false);
					if (newDfn)
					{
						// Get / create the node
						const CFormDfn *parentDfn;
						uint indexDfn;
						const CFormDfn *nodeDfn;
						const CType *nodeType;
						CFormElm *node;
						UFormDfn::TEntryType type;
						bool array;
						bool created;
						CForm *form=doc.getFormPtr ();
						CFormElm *elm = doc.getRootNode (_Slot);
						nlverify ( elm->createNodeByName (_FormName.c_str (), &parentDfn, indexDfn, 
							&nodeDfn, &nodeType, &node, type, array, created) );
						nlassert (node);

						// Get the atom
						CFormElmVirtualStruct *vStruct = safe_cast<CFormElmVirtualStruct*> (node);

						// Assign it
						vStruct->DfnFilename = _UserData.c_str ();

						// Build the dfn
						vStruct->build (newDfn);
						modified = true;

						update (true, Redraw, doc, _FormName.c_str ());
					}
					else
					{
						ok = false;

						// Output error
						char msg[512];
						smprintf (msg, 512, "Can't read the dfn %s.", _UserData.c_str());
						theApp.outputError (msg);
						update (false, Redraw, doc, _FormName.c_str ());
					}
				}
				else
				{
					// Delete the node
					const CFormDfn *parentDfn;
					uint indexDfn;
					const CFormDfn *nodeDfn;
					const CType *nodeType;
					CFormElm *node;
					UFormDfn::TEntryType type;
					bool array;
					CForm *form=doc.getFormPtr ();
					CFormElm *elm = doc.getRootNode (_Slot);
					nlverify ( elm->deleteNodeByName (_FormName.c_str (), &parentDfn, indexDfn, 
						&nodeDfn, &nodeType, &node, type, array) );

					// Document is modified by this view
					modified = true;
					update (true, Redraw, doc, _FormName.c_str ());
				}
			}
			else
			{
				// Fill memstream with the new buffer
				theApp.FillMemStreamWithBuffer (&_OldValue[0], _OldValue.size ());

				// Reserial the document
				nlverify (theApp.SerialFromMemStream (_FormName.c_str (), &doc, _Slot));
				modified = true;
				update (true, Redraw, doc, _FormName.c_str ());
			}
		}
		break;
	default:
		nlstop;
	}	

	return ok;
}

// ***************************************************************************


