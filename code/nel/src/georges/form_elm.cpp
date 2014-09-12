// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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


#include "stdgeorges.h"

#include "nel/misc/o_xml.h"
#include "nel/misc/i_xml.h"

#include "nel/georges/form.h"
#include "nel/georges/form_elm.h"
#include "nel/georges/form_loader.h"
#include "nel/georges/type.h"

using namespace NLMISC;
using namespace std;

namespace NLGEORGES
{

// ***************************************************************************
// class CFormElm
// ***************************************************************************

// ***************************************************************************

void warning (bool exception, const char *format, ... );

// ***************************************************************************

bool CFormElm::isArray () const
{
	return false;
}

// ***************************************************************************

bool CFormElm::getArraySize (uint &/* size */) const
{
	warning (false, "getArraySize", "This node is not an array.");
	return false;
}

// ***************************************************************************

bool CFormElm::getArrayNode (const UFormElm ** /* result */, uint /* arrayIndex */) const
{
	warning (false, "getArrayNode", "This node is not an array.");
	return false;
}

// ***************************************************************************

bool CFormElm::getArrayNode (UFormElm ** /* result */, uint /* arrayIndex */)
{
	warning (false, "getArrayNode", "This node is not an array.");
	return false;
}

// ***************************************************************************

bool CFormElm::getArrayNodeName (std::string &/* result */, uint /* arrayIndex */) const
{
	warning (false, "getArrayNodeName", "This node is not an array.");
	return false;
}

// ***************************************************************************

bool CFormElm::getArrayValue (std::string &/* result */, uint /* arrayIndex */, TEval /* evaluate */, TWhereIsValue * /* where */) const
{
	warning (false, "getArrayNode", "This node is not an array.");
	return false;
}

// ***************************************************************************

bool CFormElm::getArrayValue (sint8 &/* result */, uint /* arrayIndex */, TEval /* evaluate */, TWhereIsValue * /* where */) const
{
	warning (false, "getArrayValue", "This node is not an array.");
	return false;
}

// ***************************************************************************

bool CFormElm::getArrayValue (uint8 &/* result */, uint /* arrayIndex */, TEval /* evaluate */, TWhereIsValue * /* where */) const
{
	warning (false, "getArrayValue", "This node is not an array.");
	return false;
}

// ***************************************************************************

bool CFormElm::getArrayValue (sint16 &/* result */, uint /* arrayIndex */, TEval /* evaluate */, TWhereIsValue * /* where */) const
{
	warning (false, "getArrayValue", "This node is not an array.");
	return false;
}

// ***************************************************************************

bool CFormElm::getArrayValue (uint16 &/* result */, uint /* arrayIndex */, TEval /* evaluate */, TWhereIsValue * /* where */) const
{
	warning (false, "getArrayValue", "This node is not an array.");
	return false;
}

// ***************************************************************************

bool CFormElm::getArrayValue (sint32 &/* result */, uint /* arrayIndex */, TEval /* evaluate */, TWhereIsValue * /* where */) const
{
	warning (false, "getArrayValue", "This node is not an array.");
	return false;
}

// ***************************************************************************

bool CFormElm::getArrayValue (uint32 &/* result */, uint /* arrayIndex */, TEval /* evaluate */, TWhereIsValue * /* where */) const
{
	warning (false, "getArrayValue", "This node is not an array.");
	return false;
}

// ***************************************************************************

bool CFormElm::getArrayValue (float &/* result */, uint /* arrayIndex */, TEval /* evaluate */, TWhereIsValue * /* where */) const
{
	warning (false, "getArrayValue", "This node is not an array.");
	return false;
}

// ***************************************************************************

bool CFormElm::getArrayValue (double &/* result */, uint /* arrayIndex */, TEval /* evaluate */, TWhereIsValue * /* where */) const
{
	warning (false, "getArrayValue", "This node is not an array.");
	return false;
}

// ***************************************************************************

bool CFormElm::getArrayValue (bool &/* result */, uint /* arrayIndex */, TEval /* evaluate */, TWhereIsValue * /* where */) const
{
	warning (false, "getArrayValue", "This node is not an array.");
	return false;
}

// ***************************************************************************

bool CFormElm::getArrayValue (NLMISC::CRGBA &/* result */, uint /* arrayIndex */, TEval /* evaluate */, TWhereIsValue * /* where */) const
{
	warning (false, "getArrayValue", "This node is not an array.");
	return false;
}

// ***************************************************************************

bool CFormElm::isStruct () const
{
	return false;
}

// ***************************************************************************

bool CFormElm::isVirtualStruct () const
{
	return false;
}

// ***************************************************************************

bool CFormElm::getDfnName (std::string &/* dfnName */ ) const
{
	return false;
}

// ***************************************************************************

bool CFormElm::getStructSize (uint &/* size */) const
{
	warning (false, "getStructSize", "This node is not a struct.");
	return false;
}

// ***************************************************************************

bool CFormElm::getStructNodeName (uint /* element */, string &/* result */) const
{
	warning (false, "getStructNodeName", "This node is not a struct.");
	return false;
}

// ***************************************************************************

bool CFormElm::getStructNode (uint /* element */, const UFormElm ** /* result */) const
{
	warning (false, "getStructNode", "This node is not a struct.");
	return false;
}

// ***************************************************************************

bool CFormElm::getStructNode (uint /* element */, UFormElm ** /* result */)
{
	warning (false, "getStructNode", "This node is not a struct.");
	return false;
}

// ***************************************************************************

bool CFormElm::isAtom () const
{
	return false;
}

// ***************************************************************************

const CType* CFormElm::getType ()
{
	warning (false, "getType", "This node is not an atom.");
	return 0;
}

// ***************************************************************************

bool CFormElm::getValue (string &/* result */, TEval /* evaluate */) const
{
	warning (false, "getValue", "This node is not an atom.");
	return false;
}

// ***************************************************************************

bool CFormElm::getValue (sint8 &/* result */, TEval /* evaluate */) const
{
	warning (false, "getValue", "This node is not an atom.");
	return false;
}

// ***************************************************************************

bool CFormElm::getValue (uint8 &/* result */, TEval /* evaluate */) const
{
	warning (false, "getValue", "This node is not an atom.");
	return false;
}

// ***************************************************************************

bool CFormElm::getValue (sint16 &/* result */, TEval /* evaluate */) const
{
	warning (false, "getValue", "This node is not an atom.");
	return false;
}

// ***************************************************************************

bool CFormElm::getValue (uint16 &/* result */, TEval /* evaluate */) const
{
	warning (false, "getValue", "This node is not an atom.");
	return false;
}

// ***************************************************************************

bool CFormElm::getValue (sint32 &/* result */, TEval /* evaluate */) const
{
	warning (false, "getValue", "This node is not an atom.");
	return false;
}

// ***************************************************************************

bool CFormElm::getValue (uint32 &/* result */, TEval /* evaluate */) const
{
	warning (false, "getValue", "This node is not an atom.");
	return false;
}

// ***************************************************************************

bool CFormElm::getValue (float &/* result */, TEval /* evaluate */) const
{
	warning (false, "getValue", "This node is not an atom.");
	return false;
}

// ***************************************************************************

bool CFormElm::getValue (double &/* result */, TEval /* evaluate */) const
{
	warning (false, "getValue", "This node is not an atom.");
	return false;
}

// ***************************************************************************

bool CFormElm::getValue (bool &/* result */, TEval /* evaluate */) const
{
	warning (false, "getValue", "This node is not an atom.");
	return false;
}

// ***************************************************************************

bool CFormElm::getValue (NLMISC::CRGBA &/* result */, TEval /* evaluate */) const
{
	warning (false, "getValue", "This node is not an atom.");
	return false;
}

// ***************************************************************************

CFormElm::CFormElm (CForm *form, CFormElm *parentNode, const CFormDfn *parentDfn, uint parentIndex)
{
	Form = form;
	ParentNode = parentNode;
	ParentDfn = parentDfn;
	ParentIndex = parentIndex;
	Round = 0xffffffff;
}

// ***************************************************************************

CFormElm::~CFormElm ()
{
	clean(); // it's virtual
}

// ***************************************************************************

bool CFormElm::isUsed (const CForm *form) const
{
	return form == Form;
}

// ***************************************************************************

CForm *CFormElm::getForm () const
{
	return Form;
}

// ***************************************************************************

bool CFormElm::getNodeByName (UFormElm **result, const char *name, TWhereIsNode *where, bool verbose, uint32 round)
{
	const UFormElm *resultConst = NULL;
	if (((const UFormElm*)this)->getNodeByName (&resultConst, name, where, verbose, round))
	{
		*result = const_cast<UFormElm*> (resultConst);
		return true;
	}
	return false;
}

// ***************************************************************************

bool CFormElm::getNodeByName (const UFormElm **result, const char *name, TWhereIsNode *where, bool verbose, uint32 round) const
{
	// The parent Dfn
	const CFormDfn *parentDfn;
	const CFormDfn *nodeDfn;
	const CType *nodeType;
	CFormElm *node;
	uint indexDfn;
	bool array;
	bool parentVDfnArray;
	UFormDfn::TEntryType type;

	// Search for the node
	if (getNodeByName (name, &parentDfn, indexDfn, &nodeDfn, &nodeType, &node, type, array, parentVDfnArray, verbose, round))
	{
		// Set the result
		*result = node;

		// Where ?
		if (where && node)
		{
			*where = (node->getForm () == Form) ? NodeForm : NodeParentForm;
		}

		// Ok
		return true;
	}

	return false;
}

// ***************************************************************************

bool CFormElm::getValueByName (string& result, const char *name, TEval evaluate, TWhereIsValue *where, uint32 round) const
{
	// The parent Dfn
	const CFormDfn *parentDfn;
	const CFormDfn *nodeDfn;
	const CType *nodeType;
	CFormElm *node;
	uint parentIndex;
	bool array;
	bool parentVDfnArray;
	UFormDfn::TEntryType type;

	// Search for the node
	if (getNodeByName (name, &parentDfn, parentIndex, &nodeDfn, &nodeType, &node, type, array, parentVDfnArray, true, round))
	{
		// End, return the current index
		if (type == UFormDfn::EntryType)
		{
			// The atom
			const CFormElmAtom *atom = node ? safe_cast<const CFormElmAtom*> (node) : NULL;

			// Evale
			nlassert (nodeType);
			return (nodeType->getValue (result, Form, atom, *parentDfn, parentIndex, evaluate, (uint32*)where, round, name));
		}
		else
		{
			// Error message
			warning (false, "getValueByName", "The node (%s) is not an atom element. Can't return a value.", name);
		}
	}
	else
	{
		// Error message
		warning (false, "getValueByName", "Can't find the node (%s).", name);
	}

	// Error
	return false;
}

// ***************************************************************************

bool CFormElm::getValueByName (sint8 &result,	const char *name, TEval evaluate, TWhereIsValue *where, uint32 round) const
{
	// Get the string value
	string value;
	if (getValueByName (value, name, evaluate, where, round))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElm::getValueByName (uint8 &result,	const char *name, TEval evaluate, TWhereIsValue *where, uint32 round) const
{
	// Get the string value
	string value;
	if (getValueByName (value, name, evaluate, where, round))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElm::getValueByName (sint16 &result,	const char *name, TEval evaluate, TWhereIsValue *where, uint32 round) const
{
	// Get the string value
	string value;
	if (getValueByName (value, name, evaluate, where, round))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElm::getValueByName (uint16 &result,	const char *name, TEval evaluate, TWhereIsValue *where, uint32 round) const
{
	// Get the string value
	string value;
	if (getValueByName (value, name, evaluate, where, round))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElm::getValueByName (sint32 &result,	const char *name, TEval evaluate, TWhereIsValue *where, uint32 round) const
{
	// Get the string value
	string value;
	if (getValueByName (value, name, evaluate, where, round))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElm::getValueByName (uint32 &result,	const char *name, TEval evaluate, TWhereIsValue *where, uint32 round) const
{
	// Get the string value
	string value;
	if (getValueByName (value, name, evaluate, where, round))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElm::getValueByName (float &result,	const char *name, TEval evaluate, TWhereIsValue *where, uint32 round) const
{
	// Get the string value
	string value;
	if (getValueByName (value, name, evaluate, where, round))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElm::getValueByName (double &result, const char *name, TEval evaluate, TWhereIsValue *where, uint32 round) const
{
	// Get the string value
	string value;
	if (getValueByName (value, name, evaluate, where, round))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElm::getValueByName (bool &result,	const char *name, TEval evaluate, TWhereIsValue *where, uint32 round) const
{
	// Get the string value
	string value;
	if (getValueByName (value, name, evaluate, where, round))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElm::getValueByName (NLMISC::CRGBA &result,	const char *name, TEval evaluate, TWhereIsValue *where, uint32 round) const
{
	// Get the string value
	string value;
	if (getValueByName (value, name, evaluate, where, round))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

UFormElm *CFormElm::getParent () const
{
	return ParentNode;
}

// ***************************************************************************

bool CFormElm::createNodeByName (const char *name, const CFormDfn **parentDfn, uint &indexDfn,
									const CFormDfn **nodeDfn, const CType **nodeType,
									CFormElm **node, UFormDfn::TEntryType &type,
									bool &array, bool &created)
{
	*parentDfn = ParentDfn;
	indexDfn = ParentIndex;
	*nodeDfn = NULL;
	*nodeType = NULL;
	*node = this;
	bool parentVDfnArray;
	return getInternalNodeByName (Form, name, parentDfn, indexDfn, nodeDfn, nodeType, node, type, array, Create, created, parentVDfnArray, true, NLGEORGES_FIRST_ROUND);
}

// ***************************************************************************

bool CFormElm::deleteNodeByName (const char *name, const CFormDfn **parentDfn, uint &indexDfn,
									const CFormDfn **nodeDfn, const CType **nodeType,
									CFormElm **node, UFormDfn::TEntryType &type,
									bool &array)
{
	*parentDfn = ParentDfn;
	indexDfn = ParentIndex;
	*nodeDfn = NULL;
	*nodeType = NULL;
	*node = this;
	bool created;
	bool parentVDfnArray;
	return getInternalNodeByName (Form, name, parentDfn, indexDfn, nodeDfn, nodeType, node, type, array, Delete, created, parentVDfnArray, true, NLGEORGES_FIRST_ROUND);
}

// ***************************************************************************

bool CFormElm::getNodeByName (const char *name, const CFormDfn **parentDfn, uint &indexDfn,
									const CFormDfn **nodeDfn, const CType **nodeType,
									CFormElm **node, UFormDfn::TEntryType &type,
									bool &array, bool &parentVDfnArray, bool verbose, uint32 round) const
{
	*parentDfn = ParentDfn;
	indexDfn = ParentIndex;
	*nodeDfn = NULL;
	*nodeType = NULL;
	*node = (CFormElm*)this;
	bool created;
	return getInternalNodeByName (Form, name, parentDfn, indexDfn, nodeDfn, nodeType, node, type, array, Return, created, parentVDfnArray, verbose, round);
}

// ***************************************************************************

bool CFormElm::arrayInsertNodeByName (const char *name, const CFormDfn **parentDfn, uint &indexDfn,
									const CFormDfn **nodeDfn, const CType **nodeType,
									CFormElm **node, UFormDfn::TEntryType &type,
									bool &array, bool verbose, uint arrayIndex) const
{
	// Get the node by name
	*parentDfn = ParentDfn;
	indexDfn = ParentIndex;
	*nodeDfn = NULL;
	*nodeType = NULL;
	*node = (CFormElm*)this;
	bool created;
	bool parentVDfnArray;
	if (getInternalNodeByName (Form, name, parentDfn, indexDfn, nodeDfn, nodeType, node, type, array, Create, created, parentVDfnArray, verbose, NLGEORGES_FIRST_ROUND))
	{
		// Must be in the same form
		nlassert ((*node) && ((*node)->Form == Form));

		// Get its parent
		CFormElm *parentNode = (*node)->ParentNode;
		if (parentNode->isArray ())
		{
			// Cast pointer
			CFormElmArray *array = safe_cast<CFormElmArray*>(parentNode);

			// Valid index ?
			if (arrayIndex<array->Elements.size ())
			{
				// Insert the element
				array->Elements.insert (array->Elements.begin() + arrayIndex, CFormElmArray::CElement());

				// Create a new element

				// The new element
				CFormElm *newelm = NULL;
				switch (type)
				{
				case UFormDfn::EntryType:
					{
						// Create an atom
						CFormElmAtom *atom = new CFormElmAtom (Form, array, *parentDfn, indexDfn);
						newelm = atom;
					}
					break;
				case UFormDfn::EntryDfn:
					{
						CFormElmStruct *_struct = new CFormElmStruct (Form, array, *parentDfn, indexDfn);
						_struct->build (*nodeDfn);
						newelm = _struct;
					}
					break;
				case UFormDfn::EntryVirtualDfn:
					// todo array of virtual struct
					//newelm = new CFormElmVirtualStruct (Form, array, *parentDfn, indexDfn);
					break;
				default:
					nlstop;
				}

				nlassert (newelm);

				// Set the element pointer
				array->Elements[arrayIndex].Element = newelm;

				// Ok
				return true;
			}
		}
	}
	return false;
}

// ***************************************************************************

bool CFormElm::arrayDeleteNodeByName (const char *name, const CFormDfn **parentDfn, uint &indexDfn,
									const CFormDfn **nodeDfn, const CType **nodeType,
									CFormElm **node, UFormDfn::TEntryType &type,
									bool &array, bool verbose, uint arrayIndex) const
{
	// Get the node by name
	*parentDfn = ParentDfn;
	indexDfn = ParentIndex;
	*nodeDfn = NULL;
	*nodeType = NULL;
	*node = (CFormElm*)this;
	bool created;
	bool parentVDfnArray;
	if (getInternalNodeByName (Form, name, parentDfn, indexDfn, nodeDfn, nodeType, node, type, array, Create, created, parentVDfnArray, verbose, NLGEORGES_FIRST_ROUND))
	{
		// Must be in the same form
		nlassert ((*node) && ((*node)->Form == Form));

		// Get its parent
		CFormElm *parentNode = (*node)->ParentNode;
		if (parentNode->isArray ())
		{
			// Cast pointer
			CFormElmArray *array = safe_cast<CFormElmArray*>(parentNode);

			// Valid index ?
			if (arrayIndex<array->Elements.size ())
			{
				// Insert the element
				if (array->Elements[arrayIndex].Element)
					delete array->Elements[arrayIndex].Element;

				// Erase the entry
				array->Elements.erase (array->Elements.begin () + arrayIndex);

				// Ok
				return true;
			}
		}
	}
	return false;
}

// ***************************************************************************

bool CFormElm::getInternalNodeByName (CForm *form, const char *name, const CFormDfn **parentDfn, uint &indexDfn, const CFormDfn **nodeDfn, const CType **nodeType, CFormElm **node, UFormDfn::TEntryType &type, bool &array, TNodeAction action, bool &created, bool &parentVDfnArray, bool verbose, uint32 round)
{
	// *** Init output variables
	created = false;
	parentVDfnArray = false;

	// ParentDfn or Node..
	nlassert ( (*parentDfn) || (*node) );

	// Error message
	char error[512];

	// Parent exist ?
	if (*parentDfn)
	{
		// Get the entry
		const CFormDfn::CEntry &theEntry = (*parentDfn)->getEntry (indexDfn);

		// Get the type
		type = theEntry.getType ();
		*nodeType = theEntry.getTypePtr ();
		if (type == UFormDfn::EntryVirtualDfn)
		{
			if (*node)
				*nodeDfn = safe_cast <CFormElmVirtualStruct*> (*node)->FormDfn;
			else
				*nodeDfn = NULL;
		}
		else
			*nodeDfn = theEntry.getDfnPtr ();
		array = theEntry.getArrayFlag ();
	}
	else if (*node)
	{
		nlassert (!(*node)->isArray ());
		indexDfn = 0xffffffff;
		*nodeType = (*node)->isAtom () ? safe_cast<CFormElmAtom*>(*node)->Type : NULL;
		*nodeDfn = (*node)->isStruct () ? (const CFormDfn *)(safe_cast<CFormElmStruct*>(*node)->FormDfn) : NULL;
		type = (*node)->isAtom () ? UFormDfn::EntryType : (*node)->isVirtualStruct () ? UFormDfn::EntryVirtualDfn : UFormDfn::EntryDfn;
		array = false;
	}

	// Check node pointer
	if (action == Create)
	{
		nlassert (*node);
		nlassert ((*node)->getForm () == form);
	}

	// Backup current node
	CFormElm *backupFirstElm = *node;

	// *** Parsing variables

	// Current token start and end
	const char *startToken = name;
	const char *endToken;

	// Current token start
	string token;

	// Current form name
	string currentName;
	if (*node)
		(*node)->getFormName (currentName);

	// Error
	uint errorIndex;

	// Token code
	uint code;

	// Are we parsing an array ?
	bool inArrayIndex = false;

	// Index in the array
	uint arrayIndex;

	// Bool next token must be an array index
	bool wantArrayIndex = false;

	// Last struct elm
	CFormElmStruct *lastStructElm = ((*node)->ParentNode && (*node)->ParentNode->isStruct ()) ? safe_cast<CFormElmStruct*> ((*node)->ParentNode) : NULL;
	uint lastStructIndex = 0;
	if (lastStructElm)
	{
		// Look for node in the parent
		for (; lastStructIndex<lastStructElm->Elements.size (); lastStructIndex++)
		{
			// The same node ?
			if (lastStructElm->Elements[lastStructIndex].Element == (*node))
				break;
		}

		// Must have been found
		nlassert (lastStructIndex<lastStructElm->Elements.size ());
	}

	// While there is tokens
   	while ((endToken = tokenize (startToken, token, errorIndex, code)))
	{
		// Ready an array index ?
		if (!inArrayIndex)
		{
			// For each code
			switch (code)
			{
			case TokenString:
				{
					// Need an array index array ?
					if (wantArrayIndex)
					{
						// Error message
						smprintf (error, 512, "Token (%s) should be an array index.", token.c_str());
						goto exit;
					}

					// Are we a struct ?
					if ( ((type == UFormDfn::EntryDfn) || (type == UFormDfn::EntryVirtualDfn)) /*&& (!array)*/ )
					{
						// Check the virtual DFN is not empty..
						if ( (type == UFormDfn::EntryVirtualDfn) && (*nodeDfn == NULL) )
						{
							// Is it a parent virtual DFN ?
							if ( (type == UFormDfn::EntryVirtualDfn) && (*node == NULL) )
								parentVDfnArray = true;

							// Create mode ?
							if (action == Create)
							{
								// Should have a valid node
								nlassert (*node && lastStructElm);

								// Get the current virtual dfn
								CFormElmVirtualStruct *vStruct = safe_cast<CFormElmVirtualStruct*> (*node);

								// Get the form name of the current node
								string formName;
								vStruct->getFormName (formName, NULL);

								// Get the parent node if available
								for (uint parent=0; parent<form->getParentCount (); parent++)
								{
									// Get the parent
									CForm *parentPtr = form->getParent (parent);
									nlassert (parentPtr);

									// Get the virtual node by name
									UFormElm *uelm;
									if (parentPtr->getRootNode ().getNodeByName (&uelm, formName.c_str (), NULL, verbose, round+1) && uelm)
									{
										// Value node ?
										if (uelm->isVirtualStruct ())
										{
											// Get a virtual struct pointer
											CFormElmVirtualStruct *vStructParent = safe_cast<CFormElmVirtualStruct*> (uelm);

											// Copy the DFN filename
											vStruct->DfnFilename = vStructParent->DfnFilename;

											// Build it
											vStruct->build (vStructParent->FormDfn);

											// Set the current DFN
											*nodeDfn = vStruct->FormDfn;

											// Stop looking for parent
											break;
										}
										else
										{
											// Error message
											smprintf (error, 512, "Internal node parsing error.");
											goto exit;
										}
									}
								}
							}

							// Still no DFN ?
							if (*nodeDfn == NULL)
							{
								// Error message
								smprintf (error, 512, "Empty virtual struct element. Can't look into it while it is not defined.");
								goto exit;
							}
						}

						// Must have a nodeDfn here
						nlassert (*nodeDfn);

						// Look for the element
						//						uint elementCount = (*nodeDfn)->getNumEntry ();

						// Get the parents
						vector<const CFormDfn*> arrayDfn;
						arrayDfn.reserve ((*nodeDfn)->countParentDfn ());
						(*nodeDfn)->getParentDfn (arrayDfn);

						// For each parent
						uint i;
						uint formElm = 0;
						for (i=0; i<arrayDfn.size(); i++)
						{
							// The dfn
							const CFormDfn &dfn = *(arrayDfn[i]);

							// For each elements
							uint element;
							for (element=0; element<dfn.Entries.size(); element++)
							{
								// Good name ?
								if (dfn.Entries[element].Name == token)
								{
									// Good one.
									*parentDfn = &dfn;
									indexDfn = element;
									*nodeDfn = dfn.Entries[element].Dfn;
									*nodeType = dfn.Entries[element].Type;
									type = dfn.Entries[element].TypeElement;
									array = dfn.Entries[element].Array;
									wantArrayIndex = array;

									// Next node
									if (*node)
									{
										// Get next node
										CFormElmStruct *nodeStruct = safe_cast<CFormElmStruct*> (*node);
										CFormElm *nextElt = nodeStruct->Elements[formElm].Element;

										// If no next node, watch for parent node
										*node = nextElt;

										// Create node
										if ( (action == Create) && (*node == NULL) )
										{
											// Is an array ?
											if (array)
											{
												// Create an atom
												CFormElmArray *atom = new CFormElmArray (form, *nodeDfn, *nodeType, nodeStruct, *parentDfn, indexDfn);
												*node = atom;
											}
											else
											{
												// What kind of node ?
												switch (type)
												{
												case UFormDfn::EntryType:
													{
														// Create an atom
														CFormElmAtom *atom = new CFormElmAtom (form, nodeStruct, *parentDfn, indexDfn);
														*node = atom;
													}
													break;
												case UFormDfn::EntryDfn:
													{
														CFormElmStruct *_struct = new CFormElmStruct (form, nodeStruct, *parentDfn, indexDfn);
														_struct->build (*nodeDfn);
														*node = _struct;
													}
													break;
												case UFormDfn::EntryVirtualDfn:
													*node = new CFormElmVirtualStruct (form, nodeStruct, *parentDfn, indexDfn);
													break;
												default:
													nlstop;
												}
											}

											// Node created
											created = true;

											// Set the node in parent
											nodeStruct->Elements[formElm].Element = *node;
										}

										// Is a virtual DFN ?
										if ((*node) && (*node)->isVirtualStruct ())
										{
											// Should be NULL
											nlassert (*nodeDfn == NULL);

											// Set the current dfn
											*nodeDfn = safe_cast<const CFormElmVirtualStruct*> (*node)->FormDfn;
										}

										// Save last struct
										lastStructElm = nodeStruct;
										lastStructIndex = formElm;
									}
									else
									{
										// Save last struct
									  //										CFormElmStruct *lastStructElm = NULL;
									  //uint lastStructIndex = 0xffffffff;

										*node = NULL;
									}

									break;
								}
								formElm++;
							}

							// Breaked ?
							if (element!=dfn.Entries.size())
								break;
						}

						// Breaked ?
						if (i==arrayDfn.size())
						{
							// Not found
							smprintf (error, 512, "Struct does not contain element named (%s).", token.c_str());
							goto exit;
						}
					}
					else
					{
						// Error message
						smprintf (error, 512, "Not a struct element. Can't open the node (%s).", token.c_str());
						goto exit;
					}
				}
				break;
			case TokenPoint:
				{
					// Need an array index array ?
					if (wantArrayIndex)
					{
						// Error message
						smprintf (error, 512, "Token (%s) should be an array index.", token.c_str());
						goto exit;
					}

					// Are we a struct ?
					if ((type != UFormDfn::EntryDfn) && (type != UFormDfn::EntryVirtualDfn))
					{
						// Error message
						smprintf (error, 512, "Not a struct element. Can't open the node (%s).", token.c_str());
						goto exit;
					}
				}
				break;
			case TokenArrayBegin:
				{
					// Are we an array ?
					if (!array)
					{
						// Error message
						smprintf (error, 512, "Not an array element. Can't open the node (%s).", token.c_str());
						goto exit;
					}
					inArrayIndex = true;
					arrayIndex = 0xffffffff;
				}
				break;
			default:
				{
					// Error message
					smprintf (error, 512, "Syntax error at keyword (%s).", token.c_str ());
					goto exit;
				}
				break;
			}
		}
		else
		{
			switch (code)
			{
			case TokenString:
				{
					// To int
					if (sscanf (token.c_str(), "%d", &arrayIndex)!=1)
					{
						// Error message
						smprintf (error, 512, "Keyword (%s) is not an array index.", token.c_str());
						goto exit;
					}

					// Is it a parent virtual DFN ?
					if (*node == NULL)
						parentVDfnArray = true;

					// Should have an array defined
					if (*node)
					{
						// Check index
						uint arraySize;
						nlverify ((*node)->getArraySize (arraySize));
						if (arrayIndex>=arraySize)
						{
							// Create mode ?
							if (action == Create)
							{
								// Must be in the same form
								nlassert ((*node)->Form == form);

								// The array pointer
								CFormElmArray *array = safe_cast<CFormElmArray*>(*node);
								uint oldSize = (uint)array->Elements.size ();
								array->Elements.resize (arrayIndex+1);

								// Insert empty element
								uint i;
								for (i=oldSize; i<array->Elements.size (); i++)
								{
									// The new element
									CFormElm *newelm = NULL;
									switch (type)
									{
									case UFormDfn::EntryType:
										{
											// Create an atom
											CFormElmAtom *atom = new CFormElmAtom (form, array, *parentDfn, indexDfn);
											newelm = atom;
										}
										break;
									case UFormDfn::EntryDfn:
										{
											CFormElmStruct *_struct = new CFormElmStruct (form, array, *parentDfn, indexDfn);
											_struct->build (*nodeDfn);
											newelm = _struct;
										}
										break;
									case UFormDfn::EntryVirtualDfn:
										// todo array of virtual struct
										//newelm = new CFormElmVirtualStruct (form, array, *parentDfn, indexDfn);
										break;
									default:
										nlstop;
									}

									nlassert (newelm);

									// Node created
									created = true;

									// Set the element pointer
									array->Elements[i].Element = newelm;
								}
							}
							else
							{
								// Error message
								smprintf (error, 512, "Out of array bounds (%d >= %d).", arrayIndex, arraySize);
								goto exit;
							}
						}
					}
					else
					{
						// Error message
						smprintf (error, 512, "Array is not defined.");
						goto exit;
					}
				}
				break;
			case TokenArrayEnd:
				{
					// No need of an array index any more
					wantArrayIndex = false;

					// Index found ?
					if (arrayIndex == 0xffffffff)
					{
						// Error message
						smprintf (error, 512, "Missing array index.");
					}
					else
					{
						// Let the parent DFN
						nlassert (*parentDfn);

						// New current node
						CFormElmArray *parentNode = safe_cast<CFormElmArray*> (*node);

						// Get the element
						*node = parentNode->Elements[arrayIndex].Element;

						// Is a dfn ?
						*nodeDfn = (*parentDfn)->getEntry (indexDfn).getDfnPtr ();

						// Is a type ?
						*nodeType = (*parentDfn)->getEntry (indexDfn).getTypePtr ();

						// Type ?
						type = (*parentDfn)->getEntry (indexDfn).getType ();

						// Can't be an array of array
						array = false;

						// Not any more in index
						inArrayIndex = false;

						// What kind of node ?
						if ( (action == Create) && ( *node == NULL) )
						{
							switch (type)
							{
							case UFormDfn::EntryType:
								{
									// Create an atom
									CFormElmAtom *atom = new CFormElmAtom (form, parentNode, *parentDfn, indexDfn);
									*node = atom;
								}
								break;
							case UFormDfn::EntryDfn:
								{
									CFormElmStruct *_struct = new CFormElmStruct (form, parentNode, *parentDfn, indexDfn);
									_struct->build (*nodeDfn);
									*node = _struct;
								}
								break;
							case UFormDfn::EntryVirtualDfn:
								// todo array of virtual struct
								// *node = new CFormElmVirtualStruct (form, parentNode, *parentDfn, indexDfn);
								break;
							default:
								nlstop;
							}

							nlassert (*node);

							// Node created
							created = true;

							// Set the element pointer
							parentNode->Elements[arrayIndex].Element = *node;
						}

						// Is a virtual DFN ?
						if ((*node) && (*node)->isVirtualStruct ())
						{
							// Should be NULL
							nlassert (*nodeDfn == NULL);

							// Set the current dfn
							*nodeDfn = safe_cast<const CFormElmVirtualStruct*> (*node)->FormDfn;
						}
					}
				}
				break;
			default:
				{
					// Error message
					smprintf (error, 512, "Keyword (%s) is not an array index.", token.c_str());
					goto exit;
				}
			}
		}

		// Concat current address
		currentName += token;
		startToken = endToken;
	}
exit:;

	// Error ?
	bool errorAppend = endToken != NULL;

	// Continue ?
	if (!errorAppend)
	{
		// Delete the node ?
		if ( (action == Delete) && (*node) )
		{
			// Get its parent
			CFormElm *parent = safe_cast<CFormElm*> ((*node)->getParent ());

			// Don't erase the root structure
			if (parent && !parent->isArray ())
			{
				// Unlink the primitive from its parent
				parent->unlink (*node);

				// Erase the node
				delete (*node);
				*node = parent;
				parent = (CFormElm*) (parent->getParent ());

				// For each parent
				while (parent && !(*node)->isUsed (form) && !parent->isArray ())
				{
					// Unlink the primitive from its parent
					parent->unlink (*node);

					// Erase it and get next parent
					delete (*node);
					*node = parent;
					parent = (CFormElm*) (parent->getParent ());
				}

				// No more node
				*node = NULL;
			}
		}
	}

	// Node not found in get node ? Look in parents !
	if ( ((*node) == NULL) && (action == Return) && backupFirstElm )
	{
		// Get the path name
		string formName;
		backupFirstElm->getFormName (formName);
		uint formNameSize = (uint)formName.size ();
		if ((formNameSize > 0) && (formName[formNameSize-1] != '.') && (formName[formNameSize-1] != '['))
			formName += ".";
		formName += name;

		// Backup first parent default value
		bool defaultValue = false;
		const CFormDfn *defaultParentDfnParent=0;
		uint defaultIndexDfnParent=0;
		const CFormDfn *defaultNodeDfnParent=0;
		const CType *defaultNodeTypeParent=0;
		CFormElm *defaultNodeParent=0;
		UFormDfn::TEntryType defaultTypeParent = UFormDfn::EntryType;
		bool defaultArrayParent=false;
		bool defaultCreatedParent=false;
		bool defaultParentVDfnArray=false;

		// Look in parent form
		for (uint parent=0; parent<form->getParentCount (); parent++)
		{
			// Get the parent
			CForm *parentPtr = form->getParent (parent);
			nlassert (parentPtr);

			if (parentPtr->getFilename() == form->getFilename())
			{
				nlerror("parent is identical to current sheet %s!", form->getFilename().c_str());
				return false;
			}

			// Get the node by name in the parent
			const CFormDfn *parentDfnParent = NULL;
			uint indexDfnParent = 0xffffffff;
			const CFormDfn *nodeDfnParent = NULL;
			const CType *nodeTypeParent = NULL;
			CFormElm *nodeParent = (CFormElm*)&parentPtr->getRootNode ();
			UFormDfn::TEntryType typeParent;
			bool arrayParent;
			bool createdParent;
			bool parentVDfnArray;
			if (getInternalNodeByName (parentPtr, formName.c_str (), &parentDfnParent, indexDfnParent, &nodeDfnParent, &nodeTypeParent, &nodeParent, typeParent, arrayParent, action, createdParent, parentVDfnArray, false, round+1))
			{
				// Node found ?
				if (nodeParent)
				{
					// Found copy return values
					*parentDfn = parentDfnParent;
					indexDfn = indexDfnParent;
					*nodeDfn = nodeDfnParent;
					*nodeType = nodeTypeParent;
					*node = nodeParent;
					type = typeParent;
					array = arrayParent;
					created = createdParent;

					return true;
				}
				else
				{
					// Backup the first parent default value found
					if (!defaultValue)
					{
						defaultParentDfnParent = parentDfnParent;
						defaultIndexDfnParent = indexDfnParent;
						defaultNodeDfnParent = nodeDfnParent;
						defaultNodeTypeParent = nodeTypeParent;
						defaultNodeParent = nodeParent;
						defaultTypeParent = typeParent;
						defaultArrayParent = arrayParent;
						defaultCreatedParent = createdParent;
						defaultParentVDfnArray = parentVDfnArray;
						defaultValue = true;
					}
				}
			}
		}

		// Default value available ?
		if (defaultValue)
		{
			*parentDfn = defaultParentDfnParent;
			indexDfn = defaultIndexDfnParent;
			*nodeDfn = defaultNodeDfnParent;
			*nodeType = defaultNodeTypeParent;
			*node = defaultNodeParent;
			type = defaultTypeParent;
			array = defaultArrayParent;
			created = defaultCreatedParent;
			return true;
		}
	}

	// Recurse warning !
	if (*node)
	{
		if (round > NLGEORGES_MAX_RECURSION)
		{
			// Turn around..
			string formName;
			(*node)->getFormName (formName);
			warning (false, formName.c_str (), form->getFilename ().c_str(), "getInternalNodeByName", "Recursive call on the same node (%s), look for loop references or inheritances.", name);
			return false;
		}
	}

	if (verbose && errorAppend)
	{
		nlassert (*error);

		// Get the best form name
		warning (false, currentName.c_str (), form->getFilename ().c_str(), "getInternalNodeByName", "Getting the node (%s) : %s", name, error);
	}

	return !errorAppend;
}

// ***************************************************************************

const char* CFormElm::tokenize (const char *name, string &str, uint &/* errorIndex */, uint &code)
{
	if (*name == 0)
	{
		return NULL;
	}

	if (*name == '[')
	{
		code = TokenArrayBegin;
		str = "[";
		return name+1;
	}

	if (*name == ']')
	{
		code = TokenArrayEnd;
		str = "]";
		return name+1;
	}

	if (*name == '.')
	{
		code = TokenPoint;
		str = ".";
		return name+1;
	}

	str = "";
	while ( (*name != '.') && (*name != '[') && (*name != ']') && (*name != 0) )
	{
		// Add a char
		str += *name;
		name++;
	}

	code = TokenString;
	return name;
}

// ***************************************************************************

void CFormElm::unlink (CFormElm * /* child */)
{
	// No children
	nlstop;
}

// ***************************************************************************

bool CFormElm::setValueByName (const char *value, const char *name, bool *created)
{
	// The parent Dfn
	const CFormDfn *parentDfn;
	const CFormDfn *nodeDfn;
	const CType *nodeType;
	CFormElm *node;
	uint indexDfn;
	bool array;
	bool _created;
	UFormDfn::TEntryType type;

	// Search for the node
	if (createNodeByName (name, &parentDfn, indexDfn, &nodeDfn, &nodeType, &node, type, array, _created))
	{
		// Is this a type ?
		if (type == UFormDfn::EntryType)
		{
			// The atom
			CFormElmAtom *atom = node ? safe_cast<CFormElmAtom*> (node) : NULL;

			// Evale
			nlassert (nodeType);
			atom->setValue (value);

			// Created flag
			if (created)
				*created = _created;
			return true;
		}
		else
		{
			// Error message
			warning (false, "setValueByName", "The node (%s) is not an atom element. Can't set the value.", name);
		}
	}
	else
	{
		// Error message
		warning (false, "setValueByName", "Can't created / set the node (%s).", name);

		// Created flag
		if (created)
			*created = false;
	}

	// Error
	return false;
}

// ***************************************************************************

bool CFormElm::setValueByName (sint8 value, const char *name, bool *created)
{
	return setValueByName (toString (value).c_str (), name, created);
}

// ***************************************************************************

bool CFormElm::setValueByName (uint8 value, const char *name, bool *created)
{
	return setValueByName (toString (value).c_str (), name, created);
}

// ***************************************************************************

bool CFormElm::setValueByName (sint16 value, const char *name, bool *created)
{
	return setValueByName (toString (value).c_str (), name, created);
}

// ***************************************************************************

bool CFormElm::setValueByName (uint16 value, const char *name, bool *created)
{
	return setValueByName (toString (value).c_str (), name, created);
}

// ***************************************************************************

bool CFormElm::setValueByName (sint32 value, const char *name, bool *created)
{
	return setValueByName (toString (value).c_str (), name, created);
}

// ***************************************************************************

bool CFormElm::setValueByName (uint32 value, const char *name, bool *created)
{
	return setValueByName (toString (value).c_str (), name, created);
}

// ***************************************************************************

bool CFormElm::setValueByName (float value, const char *name, bool *created)
{
	return setValueByName (toString (value).c_str (), name, created);
}

// ***************************************************************************

bool CFormElm::setValueByName (double value, const char *name, bool *created)
{
	return setValueByName (toString (value).c_str (), name, created);
}

// ***************************************************************************

bool CFormElm::setValueByName (bool value, const char *name, bool *created)
{
	return setValueByName (toString (value).c_str (), name, created);
}

// ***************************************************************************

bool CFormElm::setValueByName (NLMISC::CRGBA value, const char *name, bool *created)
{
	char tmp[512];
	smprintf (tmp, 512, "%d,%d,%d", value.R, value.G, value.B);
	return setValueByName (tmp, name, created);
}

// ***************************************************************************

void CFormElm::warning (bool exception, const char *formName, const char *formFileName, const char *function, const char *format, ... )
{
	// Make a buffer string
	va_list args;
	va_start( args, format );
	char buffer[1024];
	vsnprintf( buffer, 1024, format, args );
	va_end( args );

	// Set the warning
	NLGEORGES::warning (exception, "(CFormElm::%s) on node (%s) in form (%s) : %s", function, formName, formFileName, buffer);
}

// ***************************************************************************

void CFormElm::warning (bool exception, const char *function, const char *format, ... ) const
{
	va_list args;
	va_start( args, format );

	string formName;
	getFormName (formName);
	warning (exception, formName.c_str (), getForm ()->getFilename ().c_str (), function, format, args);

	va_end( args );
}

// ***************************************************************************
// class CFormElmStruct
// ***************************************************************************

CFormElmStruct::CFormElmStruct (CForm *form, CFormElm *parentNode, const CFormDfn *parentDfn, uint parentIndex) : CFormElm (form, parentNode, parentDfn, parentIndex)
{
	FormDfn = NULL;
}

// ***************************************************************************

CFormElmStruct::~CFormElmStruct ()
{
	// Job done in clean()
}

// ***************************************************************************

void CFormElmStruct::clean ()
{
	// For each element of the array
	uint elm;
	for (elm =0; elm<Elements.size(); elm++)
	{
		if (Elements[elm].Element)
			delete Elements[elm].Element;
		Elements[elm].Element = NULL;
	}
}

// ***************************************************************************

bool CFormElmStruct::isStruct () const
{
	return true;
}

// ***************************************************************************

bool CFormElmStruct::getStructSize (uint &size) const
{
	size = (uint)Elements.size();
	return true;
}

// ***************************************************************************

bool CFormElmStruct::getStructNodeName (uint element, string &result) const
{
	if (element<Elements.size())
	{
		result = Elements[element].Name;
		return true;
	}
	else
	{
		warning (false, "getStructNodeName", "Index (%d) out of bound (%d).", element, Elements.size() );
		return false;
	}
}

// ***************************************************************************

bool CFormElmStruct::getStructNode (uint element, const UFormElm **result) const
{
	if (element<Elements.size())
	{
		*result = Elements[element].Element;
		return true;
	}
	else
	{
		warning (false, "getStructNode", "Index (%d) out of bound (%d).", element, Elements.size() );
		return false;
	}
}

// ***************************************************************************

UFormDfn *CFormElmStruct::getStructDfn ()
{
	return (CFormDfn*)FormDfn;
}

// ***************************************************************************

bool CFormElmStruct::getStructNode (uint element, UFormElm **result)
{
	if (element<Elements.size())
	{
		*result = Elements[element].Element;
		return true;
	}
	else
	{
		warning (false, "getStructNode", "Index (%d) out of bound (%d).", element, Elements.size() );
		return false;
	}
}

// ***************************************************************************

xmlNodePtr  CFormElmStruct::write (xmlNodePtr root, const CForm *form, const char *structName, bool forceWrite) const
{
	// Is used ?
	if (isUsed (form) || forceWrite)
	{
		// *** Header
		xmlNodePtr node = xmlNewChild ( root, NULL, (const xmlChar*)"STRUCT", NULL);

		// Element name
		if (structName != NULL)
		{
			// Struct name
			xmlSetProp (node, (const xmlChar*)"Name", (const xmlChar*)structName);
		}

		// For each elements of the structure
		uint elm;
		for (elm=0; elm<Elements.size(); elm++)
		{
			// Create a node if it exist
			if (Elements[elm].Element)
				Elements[elm].Element->write (node, form, Elements[elm].Name.c_str());
		}

		// Return the new node
		return node;
	}
	return NULL;
}

// ***************************************************************************

void CFormElmStruct::read (xmlNodePtr node, CFormLoader &loader, const CFormDfn *dfn, CForm *form)
{
	// Get the smart pointer on the dfn
	FormDfn = (CFormDfn*)dfn;

	// Build the Form
	build (dfn);

	// Count parent
	uint dfnCount = dfn->countParentDfn ();

	// Array of Dfn
	std::vector<const CFormDfn*> dfnArray;
	dfnArray.reserve (dfnCount);
	dfn->getParentDfn (dfnArray);

	// For each Dfn
	uint dfnId;
	uint elmIndex=0;
	for (dfnId=0; dfnId<dfnCount; dfnId++)
	{
		// Lookup for the name in the DFN
		uint elm;
		for (elm=0; elm<dfnArray[dfnId]->Entries.size(); elm++)
		{
			// Found ?
		  //			bool found = false;

			// Read the struct
			xmlNodePtr child = NULL;

			// Node can be NULL
			if (node)
				child = node->children;

			while (child)
			{
				// Good node ?
				const char *name = (const char*)xmlGetProp (child, (xmlChar*)"Name");
				if (name && (dfnArray[dfnId]->Entries[elm].getName () == name) )
				{
					// Type
					bool atom=false;
					bool array=false;
					bool _struct=false;
					bool vStruct=false;

					// Is an atom ?
					if (strcmp ((const char*)child->name, "ATOM") == 0)
					{
						atom = true;
					}
					// Is a struct ?
					else if (strcmp ((const char*)child->name, "STRUCT") == 0)
					{
						_struct = true;
					}
					// Is a struct ?
					else if (strcmp ((const char*)child->name, "VSTRUCT") == 0)
					{
						vStruct = true;
					}
					// Is an array ?
					else if (strcmp ((const char*)child->name, "ARRAY") == 0)
					{
						array = true;
					}

					// Continue ?
					if (atom || _struct || vStruct || array)
					{
						// Same type ?
						if (
							(atom && (dfnArray[dfnId]->Entries[elm].getType ()==UFormDfn::EntryType) && (!dfnArray[dfnId]->Entries[elm].getArrayFlag ()) ) ||
							(array && dfnArray[dfnId]->Entries[elm].getArrayFlag () && ( (dfnArray[dfnId]->Entries[elm].getType () == UFormDfn::EntryType) || (dfnArray[dfnId]->Entries[elm].getType () == UFormDfn::EntryDfn) ) ) ||
							(_struct && (dfnArray[dfnId]->Entries[elm].getType () == UFormDfn::EntryDfn) && (!dfnArray[dfnId]->Entries[elm].getArrayFlag ()) ) ||
							(vStruct && (dfnArray[dfnId]->Entries[elm].getType () == UFormDfn::EntryVirtualDfn) && (!dfnArray[dfnId]->Entries[elm].getArrayFlag ()) )
							)
						{
							// Ok keep it
							xmlFree((void*) name);
							break;
						}
						else
						{
							// Make a warning message
							warning (false, "read", "In block line %p, node (%s) type in DFN have changed.",
								child->content, child->name);
						}
					}
					else
					{
						if (name)
						{
							// Delete the value
							xmlFree ((void*)name);
						}

						// Throw exception
						warning (true, "read", "XML Syntax error in block line %p, node (%s) name should be STRUCT, ATOM or ARRAY.",
							child->content, child->name);
					}
				}

				if (name)
				{
					// Delete the value
					xmlFree ((void*)name);
				}

				// Next child
				child = child->next;
			}

			// Found ?
			if (child)
			{
				// Create a new element
				if (dfnArray[dfnId]->Entries[elm].getArrayFlag ())
				{
					// Array of type
					CFormElmArray *newElm = NULL;
					if (dfnArray[dfnId]->Entries[elm].getType () == UFormDfn::EntryType)
					{
						// Load the new element
						newElm = new CFormElmArray (form, NULL, dfnArray[dfnId]->Entries[elm].getTypePtr (), this, dfnArray[dfnId], elm);
					}
					// Array of struct
					else if (dfnArray[dfnId]->Entries[elm].getType () == UFormDfn::EntryDfn)
					{
						newElm = new CFormElmArray (form, dfnArray[dfnId]->Entries[elm].getDfnPtr (), NULL, this, dfnArray[dfnId], elm);
					}

					// Should be created
					nlassert (newElm);
					Elements[elmIndex].Element = newElm;
					newElm->read (child, loader, form);
				}
				else if (dfnArray[dfnId]->Entries[elm].getType () == UFormDfn::EntryType)
				{
					// Load the new element
					CFormElmAtom *newElm = new CFormElmAtom (form, this, dfnArray[dfnId], elm);
					Elements[elmIndex].Element = newElm;
					newElm->read (child, loader, dfnArray[dfnId]->Entries[elm].getTypePtr (), form);
				}
				else if (dfnArray[dfnId]->Entries[elm].getType () == UFormDfn::EntryDfn)
				{
					// Load the new element
					CFormElmStruct *newElm = new CFormElmStruct (form, this, dfnArray[dfnId], elm);
					Elements[elmIndex].Element = newElm;
					newElm->read (child, loader, dfnArray[dfnId]->Entries[elm].getDfnPtr (), form);
				}
				else // if dfnArray[dfnId]->Entries[elm].getType () == CFormDfn::CEntry::EntryVirtualDfn)
				{
					// Should be a struct
					nlassert (dfnArray[dfnId]->Entries[elm].getType () == UFormDfn::EntryVirtualDfn);

					// Load the new element
					CFormElmVirtualStruct *newElm = new CFormElmVirtualStruct (form, this, dfnArray[dfnId], elm);
					Elements[elmIndex].Element = newElm;
					newElm->read (child, loader, form);
				}
			}
			else
				Elements[elmIndex].Element = NULL;

			elmIndex++;
		}
	}
}

// ***************************************************************************

bool CFormElmStruct::isUsed (const CForm *form) const
{
	for (uint i=0; i<Elements.size(); i++)
	{
		if (Elements[i].Element && Elements[i].Element->isUsed (form))
			return true;
	}
	return false;
}

// ***************************************************************************

void CFormElmStruct::build (const CFormDfn *dfn)
{
	// Clean the form
	clean ();

	// Set the DFN
	FormDfn = (CFormDfn*)dfn;

	// Get the parents
	vector<const CFormDfn*> arrayDfn;
	arrayDfn.reserve (dfn->countParentDfn ());
	dfn->getParentDfn (arrayDfn);

	// Count element
	uint elementCount = 0;
	uint dfnIndex;
	for (dfnIndex=0; dfnIndex<arrayDfn.size(); dfnIndex++)
	{
		elementCount += arrayDfn[dfnIndex]->getNumEntry();
	}

	// Resize the element array
	Elements.resize (elementCount);

	elementCount = 0;
	for (dfnIndex=0; dfnIndex<arrayDfn.size(); dfnIndex++)
	{
		// For each element
		for (uint elm=0; elm<arrayDfn[dfnIndex]->Entries.size(); elm++)
		{
			// Copy the name
			Elements[elementCount].Name = arrayDfn[dfnIndex]->Entries[elm].Name;
			elementCount++;
		}
	}
}

// ***************************************************************************

void CFormElmStruct::unlink (CFormElm *child)
{
  uint i;
	for (i=0; i<Elements.size (); i++)
	{
		if (Elements[i].Element == child)
		{
			Elements[i].Element = NULL;
			break;
		}
	}

	// Element not found!
	nlassert (i != Elements.size ());
}

// ***************************************************************************

void CFormElmStruct::getFormName (std::string &result, const CFormElm *child) const
{
	// Reset the result
	if (child == NULL)
	{
		result = "";
		result.reserve (50);
	}

	// Get parent form name
	if (ParentNode)
		ParentNode->getFormName (result, this);

	// Get node name
	if (child)
	{
		// Look for the child
		uint i;
		for (i=0; i<Elements.size (); i++)
		{
			// This one ?
			if (Elements[i].Element == child)
			{
				// Add the field name
				result += ".";
				result += Elements[i].Name;
				break;
			}
		}

		// Draw some warning
		if (i==Elements.size ())
		{
			warning (false, "getFormName", "Child node not found.");
		}
	}
}

// ***************************************************************************

void CFormElmStruct::warning (bool exception, const char *function, const char *format, ... ) const
{
	// Make a buffer string
	va_list args;
	va_start( args, format );
	char buffer[1024];
	vsnprintf( buffer, 1024, format, args );
	va_end( args );

	// Set the warning
	string formName;
	getFormName (formName, NULL);
	NLGEORGES::warning (exception, "(CFormElmStruct::%s) on node (%s) in form (%s) : %s", function, formName.c_str (), Form->getFilename ().c_str (), buffer);
}

// ***************************************************************************

void CFormElmStruct::getDependencies (std::set<std::string> &dependencies) const
{
	// Visit the dfn
	if (FormDfn)
		FormDfn->getDependencies (dependencies);

	// Visit elements
	for (uint i=0; i<Elements.size (); i++)
	{
		if (Elements[i].Element)
			Elements[i].Element->getDependencies (dependencies);
	}
}

// ***************************************************************************
// class CFormElmVirtualStruct
// ***************************************************************************

CFormElmVirtualStruct::CFormElmVirtualStruct (CForm *form, CFormElm *parentNode, const CFormDfn *parentDfn, uint parentIndex) : CFormElmStruct (form, parentNode, parentDfn, parentIndex)
{
}

// ***************************************************************************

xmlNodePtr  CFormElmVirtualStruct::write (xmlNodePtr root, const CForm *form, const char *structName, bool forceWrite) const
{
	// Is used ?
	if (isUsed (form) || forceWrite)
	{
		// *** Header
		xmlNodePtr node = xmlNewChild ( root, NULL, (const xmlChar*)"VSTRUCT", NULL);

		// Write the DFN filename in the node
		xmlSetProp (node, (const xmlChar*)"DfnName", (const xmlChar*)DfnFilename.c_str());

		// Element name
		if (structName != NULL)
		{
			// Struct name
			xmlSetProp (node, (const xmlChar*)"Name", (const xmlChar*)structName);
		}

		// For each elements of the structure
		uint elm;
		for (elm=0; elm<Elements.size(); elm++)
		{
			// Create a node if it exist
			if (Elements[elm].Element)
				Elements[elm].Element->write (node, form, Elements[elm].Name.c_str());
		}

		// Return the new node
		return node;
	}
	return NULL;
}

// ***************************************************************************

void CFormElmVirtualStruct::read (xmlNodePtr node, CFormLoader &loader, CForm *form)
{
	// Get the DFN filename
	const char *filename = (const char*)xmlGetProp (node, (xmlChar*)"DfnName");
	if (filename)
	{
		// Set the name
		DfnFilename = filename;

		// Delete the value
		xmlFree ((void*)filename);

		// Load the dfn
		FormDfn = loader.loadFormDfn (DfnFilename.c_str (), false);
		if (!FormDfn)
		{
			// Throw exception
			warning (true, "read", "Can't find DFN filename (%s).", DfnFilename.c_str ());
		}
	}
	else
	{
		// Throw exception
		warning (true, "read", "XML Syntax error in virtual struct in block line %p, should have a DfnName property.",
			node->content);
	}

	// Read the parent
	CFormElmStruct::read (node, loader, FormDfn, form);
}

// ***************************************************************************

bool CFormElmVirtualStruct::isVirtualStruct () const
{
	return true;
}

// ***************************************************************************

bool CFormElmVirtualStruct::getDfnName (std::string &dfnName ) const
{
	dfnName = DfnFilename;
	return true;
}

// ***************************************************************************

bool CFormElmVirtualStruct::isUsed (const CForm * /* form */) const
{
	return true;
}

// ***************************************************************************

void CFormElmVirtualStruct::warning (bool exception, const char *function, const char *format, ... ) const
{
	// Make a buffer string
	va_list args;
	va_start( args, format );
	char buffer[1024];
	vsnprintf( buffer, 1024, format, args );
	va_end( args );

	// Set the warning
	string formName;
	getFormName (formName, NULL);
	NLGEORGES::warning (exception, "(CFormElmVirtualStruct::%s) on node (%s) in form (%s) : %s", function, formName.c_str (), Form->getFilename ().c_str (), buffer);
}

// ***************************************************************************
// class CFormElmArray
// ***************************************************************************

CFormElmArray::CFormElmArray (CForm *form, const CFormDfn *formDfn, const CType *type, CFormElm *parentNode, const CFormDfn *parentDfn, uint parentIndex) : CFormElm (form, parentNode, parentDfn, parentIndex)
{
	FormDfn = (CFormDfn*)formDfn;
	Type = type;
}

// ***************************************************************************

CFormElmArray::~CFormElmArray ()
{
	// Job done in clean()
}

// ***************************************************************************

void CFormElmArray::clean ()
{
	// For each element of the array
	uint elm;
	for (elm =0; elm<Elements.size(); elm++)
	{
		if (Elements[elm].Element)
			delete Elements[elm].Element;
	}
	Elements.clear ();
}

// ***************************************************************************

bool CFormElmArray::isArray () const
{
	return true;
}

// ***************************************************************************

bool CFormElmArray::getArraySize (uint &size) const
{
	size = (uint)Elements.size ();
	return true;
}

// ***************************************************************************

bool CFormElmArray::getArrayNode (const UFormElm **result, uint arrayIndex) const
{
	if (arrayIndex<Elements.size())
	{
		*result = Elements[arrayIndex].Element;
		return true;
	}
	else
	{
		warning (false, "getArrayNode", "Index (%d) out of bound (%d).", arrayIndex, Elements.size() );
		return false;
	}
}

// ***************************************************************************

bool CFormElmArray::getArrayNodeName (std::string &result, uint arrayIndex) const
{
	if (arrayIndex<Elements.size())
	{
		if (Elements[arrayIndex].Name.empty ())
			result = "#" + toString (arrayIndex);
		else
			result = Elements[arrayIndex].Name;
		return true;
	}
	else
	{
		warning (false, "getArrayNodeName", "Index (%d) out of bound (%d).", arrayIndex, Elements.size() );
		return false;
	}
}

// ***************************************************************************

bool CFormElmArray::getArrayNode (UFormElm **result, uint arrayIndex)
{
	if (arrayIndex<Elements.size())
	{
		*result = Elements[arrayIndex].Element;
		return true;
	}
	else
	{
		warning (false, "getArrayNode", "Index (%d) out of bound (%d).", arrayIndex, Elements.size() );
		return false;
	}
}


// ***************************************************************************

bool CFormElmArray::getArrayValue (std::string &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const
{
	if (arrayIndex >= Elements.size())
	{
		warning (false, "getArrayValue", "Access out of bound, trying to access array index %u, array size is %u.", arrayIndex, Elements.size());
	}
	else if (Type)
	{
		return (Type->getValue (result, Form, safe_cast<const CFormElmAtom*> (Elements[arrayIndex].Element), *ParentDfn, ParentIndex, evaluate, (uint32*)where, NLGEORGES_FIRST_ROUND, NULL));
	}
	else
	{
		warning (false, "getArrayValue", "This array is not an array of atom. This is an array of structure.");
	}

	return false;
}

// ***************************************************************************

bool CFormElmArray::getArrayValue (sint8 &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const
{
	if (Type)
	{
		string str;
		if (Type->getValue (str, Form, safe_cast<const CFormElmAtom*> (Elements[arrayIndex].Element), *ParentDfn, ParentIndex, evaluate, (uint32*)where, NLGEORGES_FIRST_ROUND, NULL))
		{
			return convertValue (result, str.c_str ());
		}
	}
	else
	{
		warning (false, "getArrayValue", "This array is not an array of atom. This is an array of structure.");
	}

	return false;
}

// ***************************************************************************

bool CFormElmArray::getArrayValue (uint8 &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const
{
	if (Type)
	{
		string str;
		if (Type->getValue (str, Form, safe_cast<const CFormElmAtom*> (Elements[arrayIndex].Element), *ParentDfn, ParentIndex, evaluate, (uint32*)where, NLGEORGES_FIRST_ROUND, NULL))
		{
			return convertValue (result, str.c_str ());
		}
	}
	else
	{
		warning (false, "getArrayValue", "This array is not an array of atom. This is an array of structure.");
	}

	return false;
}

// ***************************************************************************

bool CFormElmArray::getArrayValue (sint16 &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const
{
	if (Type)
	{
		string str;
		if (Type->getValue (str, Form, safe_cast<const CFormElmAtom*> (Elements[arrayIndex].Element), *ParentDfn, ParentIndex, evaluate, (uint32*)where, NLGEORGES_FIRST_ROUND, NULL))
		{
			return convertValue (result, str.c_str ());
		}
	}
	else
	{
		warning (false, "getArrayValue", "This array is not an array of atom. This is an array of structure.");
	}

	return false;
}

// ***************************************************************************

bool CFormElmArray::getArrayValue (uint16 &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const
{
	if (Type)
	{
		string str;
		if (Type->getValue (str, Form, safe_cast<const CFormElmAtom*> (Elements[arrayIndex].Element), *ParentDfn, ParentIndex, evaluate, (uint32*)where, NLGEORGES_FIRST_ROUND, NULL))
		{
			return convertValue (result, str.c_str ());
		}
	}
	else
	{
		warning (false, "getArrayValue", "This array is not an array of atom. This is an array of structure.");
	}

	return false;
}

// ***************************************************************************

bool CFormElmArray::getArrayValue (sint32 &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const
{
	if (Type)
	{
		string str;
		if (Type->getValue (str, Form, safe_cast<const CFormElmAtom*> (Elements[arrayIndex].Element), *ParentDfn, ParentIndex, evaluate, (uint32*)where, NLGEORGES_FIRST_ROUND, NULL))
		{
			return convertValue (result, str.c_str ());
		}
	}
	else
	{
		warning (false, "getArrayValue", "This array is not an array of atom. This is an array of structure.");
	}

	return false;
}

// ***************************************************************************

bool CFormElmArray::getArrayValue (uint32 &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const
{
	if (Type)
	{
		string str;
		if (Type->getValue (str, Form, safe_cast<const CFormElmAtom*> (Elements[arrayIndex].Element), *ParentDfn, ParentIndex, evaluate, (uint32*)where, NLGEORGES_FIRST_ROUND, NULL))
		{
			return convertValue (result, str.c_str ());
		}
	}
	else
	{
		warning (false, "getArrayValue", "This array is not an array of atom. This is an array of structure.");
	}

	return false;
}

// ***************************************************************************

bool CFormElmArray::getArrayValue (float &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const
{
	if (Type)
	{
		string str;
		if (Type->getValue (str, Form, safe_cast<const CFormElmAtom*> (Elements[arrayIndex].Element), *ParentDfn, ParentIndex, evaluate, (uint32*)where, NLGEORGES_FIRST_ROUND, NULL))
		{
			return convertValue (result, str.c_str ());
		}
	}
	else
	{
		warning (false, "getArrayValue", "This array is not an array of atom. This is an array of structure.");
	}

	return false;
}

// ***************************************************************************

bool CFormElmArray::getArrayValue (double &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const
{
	if (Type)
	{
		string str;
		if (Type->getValue (str, Form, safe_cast<const CFormElmAtom*> (Elements[arrayIndex].Element), *ParentDfn, ParentIndex, evaluate, (uint32*)where, NLGEORGES_FIRST_ROUND, NULL))
		{
			return convertValue (result, str.c_str ());
		}
	}
	else
	{
		warning (false, "getArrayValue", "This array is not an array of atom. This is an array of structure.");
	}

	return false;
}

// ***************************************************************************

bool CFormElmArray::getArrayValue (bool &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const
{
	if (Type)
	{
		string str;
		if (Type->getValue (str, Form, safe_cast<const CFormElmAtom*> (Elements[arrayIndex].Element), *ParentDfn, ParentIndex, evaluate, (uint32*)where, NLGEORGES_FIRST_ROUND, NULL))
		{
			return convertValue (result, str.c_str ());
		}
	}
	else
	{
		warning (false, "getArrayValue", "This array is not an array of atom. This is an array of structure.");
	}

	return false;
}

// ***************************************************************************

bool CFormElmArray::getArrayValue (NLMISC::CRGBA &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const
{
	if (Type)
	{
		string str;
		if (Type->getValue (str, Form, safe_cast<const CFormElmAtom*> (Elements[arrayIndex].Element), *ParentDfn, ParentIndex, evaluate, (uint32*)where, NLGEORGES_FIRST_ROUND, NULL))
		{
			return convertValue (result, str.c_str ());
		}
	}
	else
	{
		warning (false, "getArrayValue", "This array is not an array of atom. This is an array of structure.");
	}

	return false;
}

// ***************************************************************************

xmlNodePtr CFormElmArray::write (xmlNodePtr root, const CForm *form, const char *structName, bool forceWrite) const
{
	// Arrau is used ?
	if (isUsed (form) || forceWrite)
	{
		// *** Header
		xmlNodePtr node = xmlNewChild ( root, NULL, (const xmlChar*)"ARRAY", NULL);

		// Element name
		if (structName != NULL)
		{
			// Struct name
			xmlSetProp (node, (const xmlChar*)"Name", (const xmlChar*)structName);
		}

		// For each elements of the structure
		uint elm;
		for (elm=0; elm<Elements.size(); elm++)
		{
			// Create a node
			if (Elements[elm].Element)
				Elements[elm].Element->write (node, form, Elements[elm].Name.empty ()?NULL:Elements[elm].Name.c_str (), true);
		}

		// Return the new node
		return node;
	}

	return NULL;
}

// ***************************************************************************

void CFormElmArray::read (xmlNodePtr node, CFormLoader &loader, CForm *form)
{
	// Clean the form
	clean ();

	// Count child
	if (node)
	{
		// Type of DFN array
		if (Type)
		{
			nlassert (FormDfn == NULL);

			// Count children
			uint childCount = CIXml::countChildren (node, "ATOM");

			// Resize the table
			Elements.resize (childCount);

			// For each children
			uint childNum=0;
			xmlNodePtr child = CIXml::getFirstChildNode (node, "ATOM");
			while (child)
			{
				// Get node name
				const char *name = (const char*)xmlGetProp (child, (xmlChar*)"Name");

				// Create a new node
				CFormElmAtom *newElt = new CFormElmAtom (form, this, ParentDfn, ParentIndex);
				Elements[childNum].Element = newElt;
				if (name)
				{
					Elements[childNum].Name = name;
					xmlFree ((void*)name);
				}
				newElt->read (child, loader, Type, form);

				// Next child
				child = CIXml::getNextChildNode (child, "ATOM");
				childNum++;
			}
		}
		else
		{
			nlassert (FormDfn);
			nlassert (Type == NULL);

			// Count children
			uint childCount = CIXml::countChildren (node, "STRUCT");

			// Resize the table
			Elements.resize (childCount);

			// For each children
			uint childNum=0;
			xmlNodePtr child = CIXml::getFirstChildNode (node, "STRUCT");
			while (child)
			{
				// Get node name
				const char *name = (const char*)xmlGetProp (child, (xmlChar*)"Name");

				// Create a new node
				CFormElmStruct *newElt = new CFormElmStruct (form, this, ParentDfn, ParentIndex);
				Elements[childNum].Element = newElt;
				if (name)
				{
					Elements[childNum].Name = name;
					xmlFree ((void*)name);
				}
				newElt->read (child, loader, FormDfn, form);

				// Next child
				child = CIXml::getNextChildNode (child, "STRUCT");
				childNum++;
			}
		}
	}
}

// ***************************************************************************

bool CFormElmArray::setParent (CFormElm * /* parent */)
{
	return true;
}

// ***************************************************************************

void CFormElmArray::unlink (CFormElm *child)
{
  uint i;
	for (i=0; i<Elements.size (); i++)
	{
		if (Elements[i].Element == child)
		{
			Elements[i].Element = NULL;
			break;
		}
	}

	// Element not found!
	nlassert (i != Elements.size ());
}

// ***************************************************************************

bool CFormElmArray::isUsed (const CForm *form) const
{
	/*for (uint i=0; i<Elements.size(); i++)
	{
		if (Elements[i] && Elements[i]->isUsed (form))
			return true;
	}*/
	return form == Form;
}

// ***************************************************************************

void CFormElmArray::getFormName (std::string &result, const CFormElm *child) const
{
	// Reset the result
	if (child == NULL)
	{
		result = "";
		result.reserve (50);
	}

	// Get parent form name
	if (ParentNode)
		ParentNode->getFormName (result, this);

	// Get node name
	if (child)
	{
		// Look for the child
		uint i;
		for (i=0; i<Elements.size (); i++)
		{
			// This one ?
			if (Elements[i].Element == child)
			{
				char name[512];
				smprintf (name, 512, "[%d]", i);

				// Add the field name
				result += name;
				break;
			}
		}

		// Draw some warning
		if (i==Elements.size ())
		{
			warning (false, "getFormName", "Child node not found.");
		}
	}
}

// ***************************************************************************

void CFormElmArray::warning (bool exception, const char *function, const char *format, ... ) const
{
	// Make a buffer string
	va_list args;
	va_start( args, format );
	char buffer[1024];
	vsnprintf( buffer, 1024, format, args );
	va_end( args );

	// Set the warning
	string formName;
	getFormName (formName, NULL);
	NLGEORGES::warning (exception, "(CFormElmArray::%s) on node (%s) in form (%s) : %s", function, formName.c_str (), Form->getFilename ().c_str (), buffer);
}

// ***************************************************************************

void CFormElmArray::getDependencies (std::set<std::string> &dependencies) const
{
	if (FormDfn)
	{
		// Add the dfn
		FormDfn->getDependencies (dependencies);

		// Add each elements
		for (uint i=0; i<Elements.size (); i++)
		{
			Elements[i].Element->getDependencies (dependencies);
		}
	}

	if (Type)
	{
		// Add the type
		Type->getDependencies (dependencies);
	}
}

// ***************************************************************************
// CFormElmAtom
// ***************************************************************************

CFormElmAtom::CFormElmAtom (CForm *form, CFormElm *parentNode, const CFormDfn *parentDfn, uint parentIndex) : CFormElm (form, parentNode, parentDfn, parentIndex)
{
	Type = NULL;
}

// ***************************************************************************

bool CFormElmAtom::isAtom () const
{
	return true;
}

// ***************************************************************************

const CType* CFormElmAtom::getType ()
{
	return Type;
}

// ***************************************************************************

void CFormElmAtom::getDependencies (std::set<std::string> &/* dependencies */) const
{
}

// ***************************************************************************

bool CFormElmAtom::getValue (string &result, TEval evaluate) const
{
	nlassert (Type);

	// Evale
	return Type->getValue (result, Form, this, *ParentDfn, ParentIndex, evaluate, NULL, NLGEORGES_FIRST_ROUND, "");
}

// ***************************************************************************

bool CFormElmAtom::getValue (sint8 &result, TEval evaluate) const
{
	// Get the string value
	string value;
	if (getValue (value, evaluate))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElmAtom::getValue (uint8 &result, TEval evaluate) const
{
	// Get the string value
	string value;
	if (getValue (value, evaluate))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElmAtom::getValue (sint16 &result, TEval evaluate) const
{
	// Get the string value
	string value;
	if (getValue (value, evaluate))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElmAtom::getValue (uint16 &result, TEval evaluate) const
{
	// Get the string value
	string value;
	if (getValue (value, evaluate))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElmAtom::getValue (sint32 &result, TEval evaluate) const
{
	// Get the string value
	string value;
	if (getValue (value, evaluate))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElmAtom::getValue (uint32 &result, TEval evaluate) const
{
	// Get the string value
	string value;
	if (getValue (value, evaluate))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElmAtom::getValue (float &result, TEval evaluate) const
{
	// Get the string value
	string value;
	if (getValue (value, evaluate))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElmAtom::getValue (double &result, TEval evaluate) const
{
	// Get the string value
	string value;
	if (getValue (value, evaluate))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElmAtom::getValue (bool &result, TEval evaluate) const
{
	// Get the string value
	string value;
	if (getValue (value, evaluate))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

bool CFormElmAtom::getValue (NLMISC::CRGBA &result, TEval evaluate) const
{
	// Get the string value
	string value;
	if (getValue (value, evaluate))
	{
		return convertValue (result, value.c_str ());
	}

	return false;
}

// ***************************************************************************

xmlNodePtr  CFormElmAtom::write (xmlNodePtr root, const CForm *form, const char *structName, bool forceWrite) const
{
	// Atom is used ?
	if (isUsed (form) || forceWrite)
	{
		// *** Header
		xmlNodePtr node = xmlNewChild ( root, NULL, (const xmlChar*)"ATOM", NULL);

		// Element name
		if (structName != NULL)
		{
			// Struct name
			xmlSetProp (node, (const xmlChar*)"Name", (const xmlChar*)structName);
		}

		// The value
		if (!Value.empty ())
		{
			if (COXml::isStringValidForProperties (Value.c_str ()))
				xmlSetProp (node, (const xmlChar*)"Value", (const xmlChar*)Value.c_str());
			else
			{
				xmlNodePtr textNode = xmlNewText ((const xmlChar *)Value.c_str ());
				xmlAddChild (node, textNode);
			}
		}

		// Return the new node
		return node;
	}
	return NULL;
}

// ***************************************************************************

void CFormElmAtom::read (xmlNodePtr node, CFormLoader &/* loader */, const CType *type, CForm * /* form */)
{
	// Set the type
	Type = type;

	// Set the value ?
	if (node)
	{
		// Get the value
		const char *value = (const char*)xmlGetProp (node, (xmlChar*)"Value");
		if (value)
		{
			// Active value
			setValue (value);

			// Delete the value
			xmlFree ((void*)value);
		}
		else
		{
			// Get content
			const char *valueText = (const char*)xmlNodeGetContent (node);
			if (valueText)
			{
				setValue (valueText);

				// Delete the value
				xmlFree ((void*)valueText);
			}
		}
	}
}

// ***************************************************************************

void CFormElmAtom::setValue (const char *value)
{
	Value = value;
}

// ***************************************************************************

void CFormElmAtom::getValue (std::string &result) const
{
	result = Value;
}

// ***************************************************************************

void CFormElmAtom::getFormName (std::string &result, const CFormElm *child) const
{
	// Must be NULL
	nlassert (child == NULL);
	result = "";
	result.reserve (50);

	// Get parent form name
	if (ParentNode)
		ParentNode->getFormName (result, this);
}

// ***************************************************************************

void CFormElmAtom::warning (bool exception, const char *function, const char *format, ... ) const
{
	// Make a buffer string
	va_list args;
	va_start( args, format );
	char buffer[1024];
	vsnprintf( buffer, 1024, format, args );
	va_end( args );

	// Set the warning
	string formName;
	getFormName (formName, NULL);
	NLGEORGES::warning (exception, "(CFormElmAtom::%s) on node (%s) in form (%s) : %s", function, formName.c_str (), Form->getFilename ().c_str (), buffer);
}

// ***************************************************************************

} // NLGEORGES
