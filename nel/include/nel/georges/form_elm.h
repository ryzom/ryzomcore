// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_FORM_ELM_H
#define NL_FORM_ELM_H

#include "nel/georges/u_form_elm.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/rgba.h"

#include "form_dfn.h"

#define NLGEORGES_FIRST_ROUND 0
#define NLGEORGES_MAX_RECURSION 100

namespace NLGEORGES
{

class CType;
class CFormDfn;
class CForm;

/**
  * Base class of form elements
  */
class CFormElm : public UFormElm
{
	friend class CForm;
	friend class CType;
	friend class CFormDfn;
public:

	// Contructor
	CFormElm (CForm *form, CFormElm *parentNode, const CFormDfn *parentDfn, uint parentIndex);

	// Destructor
	virtual ~CFormElm () NL_OVERRIDE;

	/// Reset contents
	virtual void	clean() {}

	// Get the form pointer
	CForm			*getForm () const;

	// Is the element used by this form ?
	virtual bool	isUsed (const CForm *form) const;

	// Get the form name of the element
	virtual void	getFormName (std::string &result, const CFormElm *child = nullptr) const = 0;

	// From UFormElm
	virtual bool	getNodeByName (const UFormElm **result, const std::string &name, TWhereIsNode *where, bool verbose, uint32 round=0) const NL_OVERRIDE;
	virtual bool	getNodeByName (UFormElm **result, const std::string &name, TWhereIsNode *where, bool verbose, uint32 round=0) NL_OVERRIDE;
	virtual bool	getValueByName (std::string &result, const std::string &name, TEval evaluate, TWhereIsValue *where, uint32 round=0) const NL_OVERRIDE;
	virtual bool	getValueByName (sint8 &result, const std::string &name, TEval evaluate, TWhereIsValue *where, uint32 round=0) const NL_OVERRIDE;
	virtual bool	getValueByName (uint8 &result, const std::string &name, TEval evaluate, TWhereIsValue *where, uint32 round=0) const NL_OVERRIDE;
	virtual bool	getValueByName (sint16 &result, const std::string &name, TEval evaluate, TWhereIsValue *where, uint32 round=0) const NL_OVERRIDE;
	virtual bool	getValueByName (uint16 &result, const std::string &name, TEval evaluate, TWhereIsValue *where, uint32 round=0) const NL_OVERRIDE;
	virtual bool	getValueByName (sint32 &result, const std::string &name, TEval evaluate, TWhereIsValue *where, uint32 round=0) const NL_OVERRIDE;
	virtual bool	getValueByName (uint32 &result, const std::string &name, TEval evaluate, TWhereIsValue *where, uint32 round=0) const NL_OVERRIDE;
	virtual bool	getValueByName (float &result, const std::string &name, TEval evaluate, TWhereIsValue *where, uint32 round=0) const NL_OVERRIDE;
	virtual bool	getValueByName (double &result, const std::string &name, TEval evaluate, TWhereIsValue *where, uint32 round=0) const NL_OVERRIDE;
	virtual bool	getValueByName (bool &result, const std::string &name, TEval evaluate, TWhereIsValue *where, uint32 round=0) const NL_OVERRIDE;
	virtual bool	getValueByName (NLMISC::CRGBA &result, const std::string &name, TEval evaluate, TWhereIsValue *where, uint32 round=0) const NL_OVERRIDE;
	virtual bool	setValueByName (const std::string &value, const std::string &name, bool *created) NL_OVERRIDE;
	virtual bool	setValueByName (sint8 value, const std::string &name, bool *created) NL_OVERRIDE;
	virtual bool	setValueByName (uint8 value, const std::string &name, bool *created) NL_OVERRIDE;
	virtual bool	setValueByName (sint16 value, const std::string &name, bool *created) NL_OVERRIDE;
	virtual bool	setValueByName (uint16 value, const std::string &name, bool *created) NL_OVERRIDE;
	virtual bool	setValueByName (sint32 value, const std::string &name, bool *created) NL_OVERRIDE;
	virtual bool	setValueByName (uint32 value, const std::string &name, bool *created) NL_OVERRIDE;
	virtual bool	setValueByName (float value, const std::string &name, bool *created) NL_OVERRIDE;
	virtual bool	setValueByName (double value, const std::string &name, bool *created) NL_OVERRIDE;
	virtual bool	setValueByName (bool value, const std::string &name, bool *created) NL_OVERRIDE;
	virtual bool	setValueByName (NLMISC::CRGBA value, const std::string &name, bool *created) NL_OVERRIDE;
	virtual UFormElm	*getParent () const;
	virtual const CType *getType () NL_OVERRIDE;
	virtual bool	isArray () const NL_OVERRIDE;
	virtual bool	getArraySize (uint &size) const NL_OVERRIDE;
	virtual bool	getArrayNode (const UFormElm **result, uint arrayIndex) const NL_OVERRIDE;
	virtual bool	getArrayNode (UFormElm **result, uint arrayIndex) NL_OVERRIDE;
	virtual bool	getArrayNodeName (std::string &result, uint arrayIndex) const NL_OVERRIDE;
	virtual bool	getArrayValue (std::string &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const NL_OVERRIDE;
	virtual bool	getArrayValue (sint8 &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const NL_OVERRIDE;
	virtual bool	getArrayValue (uint8 &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const NL_OVERRIDE;
	virtual bool	getArrayValue (sint16 &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const NL_OVERRIDE;
	virtual bool	getArrayValue (uint16 &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const NL_OVERRIDE;
	virtual bool	getArrayValue (sint32 &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const NL_OVERRIDE;
	virtual bool	getArrayValue (uint32 &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const NL_OVERRIDE;
	virtual bool	getArrayValue (float &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const NL_OVERRIDE;
	virtual bool	getArrayValue (double &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const NL_OVERRIDE;
	virtual bool	getArrayValue (bool &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const NL_OVERRIDE;
	virtual bool	getArrayValue (NLMISC::CRGBA &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const NL_OVERRIDE;
	virtual bool	isStruct () const NL_OVERRIDE;
	virtual bool	isVirtualStruct () const NL_OVERRIDE;
	virtual bool	getDfnName (std::string &dfnName ) const NL_OVERRIDE;
	virtual bool	getStructSize (uint &size) const NL_OVERRIDE;
	virtual bool	getStructNodeName (uint element, std::string &result) const NL_OVERRIDE;
	virtual bool	getStructNode (uint element, const UFormElm **result) const NL_OVERRIDE;
	virtual bool	getStructNode (uint element, UFormElm **result) NL_OVERRIDE;
	virtual bool	isAtom () const NL_OVERRIDE;
	virtual bool	getValue (std::string &resultname, TEval evaluate) const NL_OVERRIDE;
	virtual bool	getValue (sint8 &resultname, TEval evaluate) const NL_OVERRIDE;
	virtual bool	getValue (uint8 &resultname, TEval evaluate) const NL_OVERRIDE;
	virtual bool	getValue (sint16 &resultname, TEval evaluate) const NL_OVERRIDE;
	virtual bool	getValue (uint16 &resultname, TEval evaluate) const NL_OVERRIDE;
	virtual bool	getValue (sint32 &resultname, TEval evaluate) const NL_OVERRIDE;
	virtual bool	getValue (uint32 &resultname, TEval evaluate) const NL_OVERRIDE;
	virtual bool	getValue (float &resultname, TEval evaluate) const NL_OVERRIDE;
	virtual bool	getValue (double &resultname, TEval evaluate) const NL_OVERRIDE;
	virtual bool	getValue (bool &resultname, TEval evaluate) const NL_OVERRIDE;
	virtual bool	getValue (NLMISC::CRGBA &resultname, TEval evaluate) const NL_OVERRIDE;
	virtual UFormDfn	*getStructDfn () NL_OVERRIDE { return nullptr; }

	// ** Convert functions

	inline bool		convertValue (sint8 &result, const std::string &value) const;
	inline bool		convertValue (uint8 &result, const std::string &value) const;
	inline bool		convertValue (sint16 &result, const std::string &value) const;
	inline bool		convertValue (uint16 &result, const std::string &value) const;
	inline bool		convertValue (sint32 &result, const std::string &value) const;
	inline bool		convertValue (uint32 &result, const std::string &value) const;
	inline bool		convertValue (float &result, const std::string &value) const;
	inline bool		convertValue (double &result, const std::string &value) const;
	inline bool		convertValue (bool &result, const std::string &value) const;
	inline bool		convertValue (NLMISC::CRGBA &result, const std::string &value) const;

	// ** Get dependencies
	virtual void	getDependencies (std::set<std::string> &dependencies) const = 0;

	// ** Internal node access

	// Create a node by name. If the node already exists, return it
	bool	createNodeByName (const std::string &, const CFormDfn **parentDfn, uint &indexDfn,
										const CFormDfn **nodeDfn, const CType **nodeType,
										CFormElm **node, UFormDfn::TEntryType &type,
										bool &array, bool &created);

	/**
	  * Delete a node by name. If the node already exists, return it
	  *Delete its parent if not used
	  */
	bool	deleteNodeByName (const std::string &name, const CFormDfn **parentDfn, uint &indexDfn,
										const CFormDfn **nodeDfn, const CType **nodeType,
										CFormElm **node, UFormDfn::TEntryType &type,
										bool &array);

	// Search for a node by name
	bool	getNodeByName (const std::string &name, const CFormDfn **parentDfn, uint &indexDfn,
										const CFormDfn **nodeDfn, const CType **nodeType,
										CFormElm **node, UFormDfn::TEntryType &type,
										bool &array, bool &parentVDfnArray, bool verbose, uint32 round) const;

	/**
	  * Insert an array node by name
	  * The index asked must be < the size of the array.
	  */
	bool	arrayInsertNodeByName (const std::string &name, const CFormDfn **parentDfn, uint &indexDfn,
										const CFormDfn **nodeDfn, const CType **nodeType,
										CFormElm **node, UFormDfn::TEntryType &type,
										bool &array, bool verbose, uint arrayIndex) const;

	/**
	  * Delete an array node by name
	  * The index asked must be < the size of the array.
	  */
	bool	arrayDeleteNodeByName (const std::string &name, const CFormDfn **parentDfn, uint &indexDfn,
										const CFormDfn **nodeDfn, const CType **nodeType,
										CFormElm **node, UFormDfn::TEntryType &type,
										bool &array, bool verbose, uint arrayIndex) const;
protected:

	// Action to perform
	enum TNodeAction
	{
		Return,
		Create,
		Delete,
	};

	/**
	  * Is createNode == Create, (*node)->Form must be == to the form argument.
	  * Is createNode == Return, form argument is not used, can be undefined.
	  *
	  * Only form, name, and action, must be defined.
	  * Then, else (*parentDfn / indexDfn ) or *node must be defined.
	  * Other values are for result only.
	  */
	static bool	getInternalNodeByName (CForm *form, const std::string &name, const CFormDfn **parentDfn, uint &indexDfn,
										const CFormDfn **nodeDfn, const CType **nodeType,
										CFormElm **node, UFormDfn::TEntryType &type,
										bool &array, TNodeAction action, bool &created, bool &parentVDfnArray, bool verbose, uint32 round);

	/**
	  * Unlink a child
	  */
	virtual void unlink (CFormElm *child);

public:

	// Get next token, return NULL if last token
	static const char* tokenize (const char *name, std::string &str, uint &errorIndex, uint &code);

	// ** IO functions
	virtual xmlNodePtr	write (xmlNodePtr node, const CForm *form, const std::string &structName = "", bool forceWrite = false) const = 0;

protected:

	// The form of this node
	CForm				*Form;

	// The parent node of this node
	CFormElm			*ParentNode;

	// The parent DFN of this node
	const CFormDfn		*ParentDfn;

	// The index in the parent DFN for this node
	uint				ParentIndex;

	// Recurce Tag
	uint32				Round;

	// Error handling
	static void			warning (bool exception, const std::string &formName, const std::string &formFileName, const std::string &function, const char *format, ... );
	virtual void		warning (bool exception, const std::string &function, const char *format, ... ) const;

private:
	// Tokens
	enum TToken
	{
		TokenString = 0,
		TokenPoint,
		TokenArrayBegin,
		TokenArrayEnd,
	};
};

/**
  * Define a structure of elements
  *
  * This structure has pointers on named sub structures in Elements.
  * If a sub structure is empty, its pointer is NULL.
  */
class CFormElmStruct : public CFormElm
{
public:
	// Default constructor
	CFormElmStruct (CForm *form, CFormElm *parentNode, const CFormDfn *parentDfn, uint parentIndex);
	virtual ~CFormElmStruct () NL_OVERRIDE;

	// Clear sub elements
	void clean () NL_OVERRIDE;

	// Smart pointer on the form definition for this structure
	NLMISC::CSmartPtr<CFormDfn>	FormDfn;

	// Pointer on the parent element
	//CFormElmStruct					*Parent;

	// A struct element
	class CFormElmStructElm
	{
	public:
		CFormElmStructElm ()
		{
			Element = nullptr;
		}
		~CFormElmStructElm()
		{
			nlassert(!Element);
		}

		std::string		Name;
		CFormElm*		Element;
	};


	// Build form a DFN
	void				build (const CFormDfn *dfn);

	// From UFormElm
	bool				isStruct () const NL_OVERRIDE;
	bool				getStructSize (uint &size) const NL_OVERRIDE;
	bool				getStructNodeName (uint element, std::string &result) const NL_OVERRIDE;
	bool				getStructNode (uint element, const UFormElm **result) const NL_OVERRIDE;
	bool				getStructNode (uint element, UFormElm **result) NL_OVERRIDE;
	UFormDfn			*getStructDfn () NL_OVERRIDE;

	// From CFormElm
	bool				isUsed (const CForm *form) const NL_OVERRIDE;
	xmlNodePtr			write (xmlNodePtr node, const CForm *form, const std::string &structName, bool forceWrite = false) const NL_OVERRIDE;
	void				unlink (CFormElm *child) NL_OVERRIDE;
	void				getFormName (std::string &result, const CFormElm *child) const NL_OVERRIDE;
	void				getDependencies (std::set<std::string> &dependencies) const NL_OVERRIDE;

	// Call by CFormLoader
	void				read (xmlNodePtr node, CFormLoader &loader, const CFormDfn *dfn, CForm *form);

	// Sub Elements
	std::vector<CFormElmStructElm>		Elements;

	// Error handling
	virtual void		warning (bool exception, const std::string &function, const char *format, ... ) const NL_OVERRIDE;
};

/**
  * Define an array of elements
  */
class CFormElmVirtualStruct : public CFormElmStruct
{
public:

	CFormElmVirtualStruct (CForm *form, CFormElm *parentNode, const CFormDfn *parentDfn, uint parentIndex);
	virtual ~CFormElmVirtualStruct() NL_OVERRIDE { }

	// The Dfn filename used by this struct
	std::string			DfnFilename;

	// From UFormElm
	bool				isVirtualStruct () const NL_OVERRIDE;
	bool				getDfnName (std::string &dfnName ) const NL_OVERRIDE;

	// From CFormElm
	bool				isUsed (const CForm *form) const NL_OVERRIDE;
	xmlNodePtr			write (xmlNodePtr node, const CForm *form, const std::string &structName, bool forceWrite = false) const NL_OVERRIDE;

	// Call by CFormLoader
	void				read (xmlNodePtr node, CFormLoader &loader, CForm *form);

	// Error handling
	virtual void		warning (bool exception, const std::string &function, const char *format, ... ) const NL_OVERRIDE;
};

/**
  * Define an array of elements
  */
class CFormElmArray : public CFormElm
{
public:
	// Default constructor
	CFormElmArray (CForm *form, const CFormDfn *formDfn, const CType *type, CFormElm *parentNode, const CFormDfn *parentDfn, uint parentIndex);
	virtual ~CFormElmArray () NL_OVERRIDE;
	void clean () NL_OVERRIDE;

	// Smart pointer on the form definition for this structure
	NLMISC::CSmartPtr<CFormDfn>	FormDfn;

	// Pointer on the type (the smart pointer in hold by CFormDfn)
	const CType			*Type;

	// From UFormElm
	bool				isArray () const NL_OVERRIDE;
	bool				getArraySize (uint &size) const NL_OVERRIDE;
	bool				getArrayNode (const UFormElm **result, uint arrayIndex) const NL_OVERRIDE;
	bool				getArrayNode (UFormElm **result, uint arrayIndex) NL_OVERRIDE;
	bool				getArrayNodeName (std::string &result, uint arrayIndex) const NL_OVERRIDE;
	bool				getArrayValue (std::string &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const NL_OVERRIDE;
	bool				getArrayValue (sint8 &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const NL_OVERRIDE;
	bool				getArrayValue (uint8 &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const NL_OVERRIDE;
	bool				getArrayValue (sint16 &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const NL_OVERRIDE;
	bool				getArrayValue (uint16 &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const NL_OVERRIDE;
	bool				getArrayValue (sint32 &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const NL_OVERRIDE;
	bool				getArrayValue (uint32 &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const NL_OVERRIDE;
	bool				getArrayValue (float &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const NL_OVERRIDE;
	bool				getArrayValue (double &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const NL_OVERRIDE;
	bool				getArrayValue (bool &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const NL_OVERRIDE;
	bool				getArrayValue (NLMISC::CRGBA &result, uint arrayIndex, TEval evaluate, TWhereIsValue *where) const NL_OVERRIDE;

	// From CFormElm
	xmlNodePtr			write (xmlNodePtr node, const CForm *form, const std::string &structName, bool forceWrite = false) const NL_OVERRIDE;
	bool				setParent (CFormElm *parent);
	void				unlink (CFormElm *child) NL_OVERRIDE;
	bool				isUsed (const CForm *form) const NL_OVERRIDE;
	void				getFormName (std::string &result, const CFormElm *child) const NL_OVERRIDE;
	void				getDependencies (std::set<std::string> &dependencies) const NL_OVERRIDE;

	// Call by CFormLoader

	// Read an array
	void				read (xmlNodePtr node, CFormLoader &loader, CForm *form);

	// A struct element
	class CElement
	{
	public:
		CElement ()
		{
			Element = nullptr;
		}
		~CElement ()
		{
			nlassert(!Element);
		}

		std::string		Name;
		CFormElm*		Element;
	};

	// Array of elements
	std::vector<CElement>		Elements;

	// Error handling
	virtual void		warning (bool exception, const std::string &function, const char *format, ... ) const NL_OVERRIDE;
};

/**
  * Signed integer element
  */
class CFormElmAtom : public CFormElm
{
	friend class CForm;
	friend class CFormElm;
	friend class CType;
public:
	// Default constructor
	CFormElmAtom (CForm *form, CFormElm *parentNode, const CFormDfn *parentDfn, uint parentIndex);
	virtual ~CFormElmAtom() NL_OVERRIDE { }

	// Pointer on the parent element
	//CFormElmAtom				*Parent;

	// Pointer on the type (the smart pointer in hold by CFormDfn)
	const CType					*Type;

	// From CFormElm
	xmlNodePtr					write (xmlNodePtr node, const CForm *form, const std::string &structName, bool forceWrite = false) const NL_OVERRIDE;
	bool						setParent (CFormElm *parent);
	void						getFormName (std::string &result, const CFormElm *child) const NL_OVERRIDE;
	void						getDependencies (std::set<std::string> &dependencies) const NL_OVERRIDE;
	const CType*                      getType () NL_OVERRIDE;

	// Call by CFormLoader
	void						read (xmlNodePtr node, CFormLoader &loader, const CType *type, CForm *form);

	// From UFormElm
	bool						isAtom () const NL_OVERRIDE;
	bool						getValue (std::string &result, TEval evaluate) const NL_OVERRIDE;
	bool						getValue (sint8 &result, TEval evaluate) const NL_OVERRIDE;
	bool						getValue (uint8 &result, TEval evaluate) const NL_OVERRIDE;
	bool						getValue (sint16 &result, TEval evaluate) const NL_OVERRIDE;
	bool						getValue (uint16 &result, TEval evaluate) const NL_OVERRIDE;
	bool						getValue (sint32 &result, TEval evaluate) const NL_OVERRIDE;
	bool						getValue (uint32 &result, TEval evaluate) const NL_OVERRIDE;
	bool						getValue (float &result, TEval evaluate) const NL_OVERRIDE;
	bool						getValue (double &result, TEval evaluate) const NL_OVERRIDE;
	bool						getValue (bool &result, TEval evaluate) const NL_OVERRIDE;
	bool						getValue (NLMISC::CRGBA &result, TEval evaluate) const NL_OVERRIDE;

	// Set the value, the elt been used
	void						setValue (const std::string &value);

	// Get the raw value. Does not care about any parent or default values
	void						getValue (std::string &result) const;

private:
	// The value
	std::string					Value;

	// Error handling
	virtual void		warning (bool exception, const std::string &function, const char *format, ... ) const NL_OVERRIDE;
};

// ***************************************************************************
// CFormElm inlines
// ***************************************************************************

inline bool CFormElm::convertValue (sint8 &result, const std::string &value) const
{
	float tmp;
	if (NLMISC::fromString(value, tmp))
	{
		NLMISC::clamp (tmp, -128.f, 127.f);
		result = (sint8)tmp;
		return true;
	}
	else
	{
		// Error message
		warning (false, "convertValue", "Can't convert the string \"%s\" in sint8.", value.c_str());
	}
	return false;
}

// ***************************************************************************

inline bool CFormElm::convertValue (uint8 &result, const std::string &value) const
{
	float tmp;
	if (NLMISC::fromString(value, tmp))
	{
		NLMISC::clamp (tmp, 0.f, 255.f);
		result = (uint8)tmp;
		return true;
	}
	else
	{
		// Error message
		warning (false, "convertValue", "Can't convert the string \"%s\" in uint8.", value.c_str());
	}
	return false;
}

// ***************************************************************************

inline bool CFormElm::convertValue (sint16 &result,	const std::string &value) const
{
	float tmp;
	if (NLMISC::fromString(value, tmp))
	{
		NLMISC::clamp (tmp, -32768.f, 32767.f);
		result = (sint16)tmp;
		return true;
	}
	else
	{
		// Error message
		warning (false, "convertValue", "Can't convert the string \"%s\" in sint16.", value.c_str());
	}
	return false;
}

// ***************************************************************************

inline bool CFormElm::convertValue (uint16 &result,	const std::string &value) const
{
	float tmp;
	if (NLMISC::fromString(value, tmp))
	{
		NLMISC::clamp (tmp, 0.f, 65535.f);
		result = (uint16)tmp;
		return true;
	}
	else
	{
		// Error message
		warning (false, "convertValue", "Can't convert the string \"%s\" in uint16.", value.c_str());
	}
	return false;
}

// ***************************************************************************

inline bool CFormElm::convertValue (sint32 &result,	const std::string &value) const
{
	float tmp;
	if (NLMISC::fromString(value, tmp))
	{
		NLMISC::clamp (tmp, -2147483648.f, 2147483647.f);
		result = (sint32)tmp;
		return true;
	}
	else
	{
		// Error message
		warning (false, "convertValue", "Can't convert the string \"%s\" in sint32.", value.c_str());
	}
	return false;
}

// ***************************************************************************

inline bool CFormElm::convertValue (uint32 &result,	const std::string &value) const
{
	float tmp;
	if (NLMISC::fromString(value, tmp))
	{
		NLMISC::clamp (tmp, 0.f, 4294967295.f);
		result = (sint32)tmp;
		return true;
	}
	else
	{
		// Error message
		warning (false, "convertValue", "Can't convert the string \"%s\" in uint32.", value.c_str());
	}
	return false;
}

// ***************************************************************************

inline bool CFormElm::convertValue (float &result, const std::string &value) const
{
	if (NLMISC::fromString(value, result))
	{
		return true;
	}
	else
	{
		// Error message
		warning (false, "convertValue", "Can't convert the string \"%s\" in float.", value.c_str());
	}
	return false;
}

// ***************************************************************************

inline bool CFormElm::convertValue (double &result, const std::string &value) const
{
	double tmp;
	if (NLMISC::fromString(value, tmp))
	{
		result = tmp;
		return true;
	}
	else
	{
		// Error message
		warning (false, "convertValue", "Can't convert the string \"%s\" in double.", value.c_str());
	}
	return false;
}

// ***************************************************************************

inline bool CFormElm::convertValue (bool &result, const std::string &value) const
{
	int tmp;
	if (NLMISC::fromString(value, tmp))
	{
		result = tmp != 0;
		return true;
	}
	else
	{
		std::string temp = NLMISC::toLowerAscii(value);
		if (strcmp (temp.c_str (), "true") == 0)
		{
			result  = true;
			return true;
		}
		if (strcmp (temp.c_str (), "false") == 0)
		{
			result  = false;
			return true;
		}
	}

	// Error message
	warning (false, "convertValue", "Can't convert the string \"%s\" in boolean.", value.c_str());

	return false;
}

// ***************************************************************************

inline bool CFormElm::convertValue (NLMISC::CRGBA &result, const std::string &value) const
{
	float r, g, b;
	if (sscanf (value.c_str(), "%f,%f,%f", &r, &g, &b) == 3)
	{
		NLMISC::clamp (r, 0.f, 255.f);
		NLMISC::clamp (g, 0.f, 255.f);
		NLMISC::clamp (b, 0.f, 255.f);
		result.R = (uint8)r;
		result.G = (uint8)g;
		result.B = (uint8)b;
		return true;
	}

	// Error message
	warning (false, "convertValue", "Can't convert the string \"%s\" in RGB color.", value.c_str());

	return false;
}

// ***************************************************************************

} // NLGEORGES

#endif // NL_FORM_ELM_H
