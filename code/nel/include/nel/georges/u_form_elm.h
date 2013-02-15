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

#ifndef NL_U_FORM_ELM_H
#define NL_U_FORM_ELM_H

#include "nel/misc/types_nl.h"

namespace NLMISC
{
	class CRGBA;
}

namespace NLGEORGES
{

class UFormElm
{
public:
	// dtor
	virtual ~UFormElm() {}

	// ** Common methods

	/// Value evalution
	enum TEval
	{
		NoEval,				/// The value will not be evaluated at all, the litteral value will be returned
		Formula,			/// Eval the enumeration value, but don't evaluate the formula nor the value references
		Eval,				/// Full evaluation of the value
	};

	/// Where a node has been found
	enum TWhereIsNode
	{
		NodeForm,			/// The node has been found in the form
		NodeParentForm,		/// The node has been found in the parent form
		NodeDfn,			/// The node is a DFN
		NodeType,			/// The node is a Type
	};

	/**
	  * Return a node pointer with its name.
	  *
	  * \param result will be filled with the node pointer. Can be NULL if the node doesn't exist.
	  * \param name is the form node name
	  * \param where is a pointer on the information flag of the value. If Where is not NULL, it is filled with
	  * the position where the node has been found. If result is NULL, where is undefined.
	  * \return true if the result has been filled, false if the node is not referenced.
	  *
	  * About the node existance
	  *
	  * An atom node exist if its value is defined.
	  * A struct node exist if one of its children exist.
	  * An array node exist if one of its children exist.
	  * If the node doesn't exist, you can't have a pointer on it with getNodeByName(). It returns NULL.
	  * But, you can evaluate the value of non-existant atom nodes with getValueByName().
	  *
	  * About the form name:
	  *
	  * Struct elements name must be separated by '.'
	  * Struct indexes must be between '[' and ']'
	  *
	  * Exemple:
	  * "position.x"			:	get the element named x in the struct named position
	  * "entities[2].color"		:	get the node named color in the second element of the entities array
	  */
	virtual bool	getNodeByName (const UFormElm **result, const char *name, TWhereIsNode *where = NULL, bool reserved=true, uint32 round=0) const = 0;
	virtual bool	getNodeByName (UFormElm **result, const char *name, TWhereIsNode *where = NULL, bool reserved=true, uint32 round=0) = 0;


	/// Where a value has been found
	enum TWhereIsValue
	{
		ValueForm,				/// The value has been found in the form
		ValueParentForm,		/// The value has been found in the parent form
		ValueDefaultDfn,		/// The value has been found in the DFN default value
		ValueDefaultType,		/// The value has been found in the TYPE default value
		Dummy = 0xffffffff		/// Be sure the size == sizeof(uint32)
	};

	/**
	  * Get a form value with its name.
	  * The numbers are clamped to the type limit values.
	  *
	  * \param result is a reference on the value to fill with the result.
	  * \param name is the form name of the value to found.
	  * \param evaluate must be true if you want to have an evaluated value, false if you want the formula value.
	  * \param where is a pointer on the information flag of the value. If Where is not NULL, it is filled with
	  * the position where the value has been found.
	  * \return true if the result has been filled, false if the value has not been found or the cast has failed or the evaluation has failed.
	  * \see getNodeByName ()
	  */
	virtual bool	getValueByName (std::string &result, const char *namename, TEval evaluate = Eval, TWhereIsValue *where = NULL, uint32 round=0) const = 0;
	virtual bool	getValueByName (sint8 &result, const char *name, TEval evaluate = Eval, TWhereIsValue *where = NULL, uint32 round=0) const = 0;
	virtual bool	getValueByName (uint8 &result, const char *name, TEval evaluate = Eval, TWhereIsValue *where = NULL, uint32 round=0) const = 0;
	virtual bool	getValueByName (sint16 &result, const char *name, TEval evaluate = Eval, TWhereIsValue *where = NULL, uint32 round=0) const = 0;
	virtual bool	getValueByName (uint16 &result, const char *name, TEval evaluate = Eval, TWhereIsValue *where = NULL, uint32 round=0) const = 0;
	virtual bool	getValueByName (sint32 &result, const char *name, TEval evaluate = Eval, TWhereIsValue *where = NULL, uint32 round=0) const = 0;
	virtual bool	getValueByName (uint32 &result, const char *name, TEval evaluate = Eval, TWhereIsValue *where = NULL, uint32 round=0) const = 0;
	virtual bool	getValueByName (float &result, const char *name, TEval evaluate = Eval, TWhereIsValue *where = NULL, uint32 round=0) const = 0;
	virtual bool	getValueByName (double &result, const char *name, TEval evaluate = Eval, TWhereIsValue *where = NULL, uint32 round=0) const = 0;
	virtual bool	getValueByName (bool &result, const char *name, TEval evaluate = Eval, TWhereIsValue *where = NULL, uint32 round=0) const = 0;

	/// Warning, only R, G and B members are filled, not A.
	virtual bool	getValueByName (NLMISC::CRGBA &result, const char *name, TEval evaluate = Eval, TWhereIsValue *where = NULL, uint32 round=0) const = 0;

	/**
	  * Set a form value with its name. If the node doesn't exist, it is created.
	  *
	  * \param value is a reference on the value to set in the form.
	  * \param name is the form name of the value to set or create.
	  * \param where is a pointer on the information flag of the value. If Where is not NULL, it is filled with
	  * the position where the value has been found.
	  * \param created is a pointer on the creatation flag. If created is not NULL, it is filled with
	  * true if the value has been created, false it the value has been filled.
	  * \return true if the value has been set, false if the value has not been found or hasn't been created.
	  */
	virtual bool	setValueByName (const char *value, const char *name, bool *created = NULL) = 0;
	virtual bool	setValueByName (sint8 value, const char *name, bool *created = NULL) = 0;
	virtual bool	setValueByName (uint8 value, const char *name, bool *created = NULL) = 0;
	virtual bool	setValueByName (sint16 value, const char *name, bool *created = NULL) = 0;
	virtual bool	setValueByName (uint16 value, const char *name, bool *created = NULL) = 0;
	virtual bool	setValueByName (sint32 value, const char *name, bool *created = NULL) = 0;
	virtual bool	setValueByName (uint32 value, const char *name, bool *created = NULL) = 0;
	virtual bool	setValueByName (float value, const char *name, bool *created = NULL) = 0;
	virtual bool	setValueByName (double value, const char *name, bool *created = NULL) = 0;
	virtual bool	setValueByName (bool value, const char *name, bool *created = NULL) = 0;
	virtual bool	setValueByName (NLMISC::CRGBA value, const char *name, bool *created = NULL) = 0;


	// ** Array element methods


	/// Return true if the element is an array
	virtual bool	isArray () const = 0;

	/// Return true if the element is an array and fill size with the array size
	virtual bool	getArraySize (uint &size) const = 0;

	/**
	  * Get a array sub element const pointer.
	  * If return true, fill result with the arrayIndex cell's element
	  * Can be NULL if the node doesn't exist.
	  */
	virtual bool	getArrayNode (const UFormElm **result, uint arrayIndex) const = 0;

	/**
	  * Get a array sub element mutable pointer.
	  * If return true, fill result with the arrayIndex cell's element pointer.
	  * Can be NULL if the node doesn't exist.
	  */
	virtual bool	getArrayNode (UFormElm **result, uint arrayIndex) = 0;


	/**
	  * Get an array value. The node must be an array of atom element.
	  *
	  * \param result is a reference on the value to fill with the result.
	  * \param arrayIndex is the array index to evaluate.
	  * \param evaluate must be true if you want to have an evaluated value, false if you want the formula value.
	  * \param where is a pointer on the information flag of the value. If Where is not NULL, it is filled with
	  * the position where the value has been found.
	  * \return true if the result has been filled, false if the value has not been found or the cast has failed or the evaluation has failed.
	  */
	virtual bool	getArrayValue (std::string &result, uint arrayIndex, TEval evaluate = Eval, TWhereIsValue *where = NULL) const = 0;
	virtual bool	getArrayValue (sint8 &result, uint arrayIndex, TEval evaluate = Eval, TWhereIsValue *where = NULL) const = 0;
	virtual bool	getArrayValue (uint8 &result, uint arrayIndex, TEval evaluate = Eval, TWhereIsValue *where = NULL) const = 0;
	virtual bool	getArrayValue (sint16 &result, uint arrayIndex, TEval evaluate = Eval, TWhereIsValue *where = NULL) const = 0;
	virtual bool	getArrayValue (uint16 &result, uint arrayIndex, TEval evaluate = Eval, TWhereIsValue *where = NULL) const = 0;
	virtual bool	getArrayValue (sint32 &result, uint arrayIndex, TEval evaluate = Eval, TWhereIsValue *where = NULL) const = 0;
	virtual bool	getArrayValue (uint32 &result, uint arrayIndex, TEval evaluate = Eval, TWhereIsValue *where = NULL) const = 0;
	virtual bool	getArrayValue (float &result, uint arrayIndex, TEval evaluate = Eval, TWhereIsValue *where = NULL) const = 0;
	virtual bool	getArrayValue (double &result, uint arrayIndex, TEval evaluate = Eval, TWhereIsValue *where = NULL) const = 0;
	virtual bool	getArrayValue (bool &result, uint arrayIndex, TEval evaluate = Eval, TWhereIsValue *where = NULL) const = 0;

	/// Warning, only R, G and B members are filled, not A.
	virtual bool	getArrayValue (NLMISC::CRGBA &result, uint arrayIndex, TEval evaluate = Eval, TWhereIsValue *where = NULL) const = 0;

	/// Return the name of a table element.
	virtual bool	getArrayNodeName (std::string &result, uint arrayIndex) const = 0;


	// ** Struct element methods


	/// Return true if the element is a struct or a virtual struct
	virtual bool	isStruct () const = 0;

	/// Return true if the element is a virtual struct
	virtual bool	isVirtualStruct () const = 0;

	/// Get the dfn filename for this virtual struct. Must be a virtual struct node.
	virtual bool	getDfnName (std::string &dfnName ) const = 0;

	/// Return the struct size
	virtual bool	getStructSize (uint &size) const = 0;

	/// Return the element name
	virtual bool	getStructNodeName (uint element, std::string &result) const = 0;

	/// Return a const element pointer. Can be NULL if the node doesn't exist.
	virtual bool	getStructNode (uint element, const UFormElm **result) const = 0;

	/// Return a mutable element pointer. Can be NULL if the node doesn't exist.
	virtual bool	getStructNode (uint element, UFormElm **result) = 0;

	/// Return the struct dfn
	virtual class UFormDfn	*getStructDfn () = 0;

	// ** Atom element methods


	/// Returns the type of the atom. NULL otherwise.
	virtual const class UType     *getType () = 0;

	/// Return true if the element is an atom
	virtual bool	isAtom () const = 0;

	/**
	  * Return the atom value.
	  * The numbers are clamped to the type limit values.
	  *
	  * \param result is the reference on the value to fill with result
	  * \param evaluate must be true if you want to have an evaluated value, false if you want the formula value.
	  */
	virtual bool	getValue (std::string &result, TEval evaluate = Eval) const = 0;
	virtual bool	getValue (sint8 &result, TEval evaluate = Eval) const = 0;
	virtual bool	getValue (uint8 &result, TEval evaluate = Eval) const = 0;
	virtual bool	getValue (sint16 &result, TEval evaluate = Eval) const = 0;
	virtual bool	getValue (uint16 &result, TEval evaluate = Eval) const = 0;
	virtual bool	getValue (sint32 &result, TEval evaluate = Eval) const = 0;
	virtual bool	getValue (uint32 &result, TEval evaluate = Eval) const = 0;
	virtual bool	getValue (float &result, TEval evaluate = Eval) const = 0;
	virtual bool	getValue (double &result, TEval evaluate = Eval) const = 0;
	virtual bool	getValue (bool &result, TEval evaluate = Eval) const = 0;

	/// Warning, only R, G and B members are filled, not A.
	virtual bool	getValue (NLMISC::CRGBA &result, TEval evaluate = Eval) const = 0;
};

} // NLGEORGES

#endif // NL_U_FORM_ELM_H
