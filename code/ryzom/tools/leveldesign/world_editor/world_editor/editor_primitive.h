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

#ifndef NL_EDITOR_PRIMITIVE_H
#define NL_EDITOR_PRIMITIVE_H

#include "world_editor_doc.h"

// ***************************************************************************
// The primitive quad grid
// ***************************************************************************

// Type for the primitive quad grid
class CQuadGridEntry
{
public:
	CQuadGridEntry ()
	{
		First = NULL;
		Second = NULL;
	}
	CQuadGridEntry (NLLIGO::IPrimitive *first, NLLIGO::IPrimitive *second = NULL)
	{
		First = first;
		Second = second;
	}

	NLLIGO::IPrimitive	*First;
	NLLIGO::IPrimitive	*Second;	// If second is NULL, it is only a primitive, if not, it is a link between two primitives
};
typedef NL3D::CQuadGrid<CQuadGridEntry>	TPrimQuadGrid;

// ***************************************************************************

extern TPrimQuadGrid PrimitiveQuadGrid;

// ***************************************************************************

// Linked list of modified primitive
extern std::list<NLLIGO::IPrimitive*>			ModifiedPrimitive;

// ***************************************************************************

// Linked list of selected primitive
extern std::list<NLLIGO::IPrimitive*>				Selection;

// ***************************************************************************

void InitQuadGrid ();

// ***************************************************************************

// Update modified primitives
void UpdatePrimitives ();

// ***************************************************************************

// Update modified primitives for selection
void UpdateSelection ();

// ***************************************************************************

// Put the primitive in the modified primitive to update in the quad grid
void InvalidatePrimitive (const CDatabaseLocatorPointer &locator, uint channels);

// Same but recursive
void InvalidatePrimitiveRec (const CDatabaseLocatorPointer &locator, uint channels);

// Invalidate primitive pointers
void InvalidateAllPrimitives ();
// verify the structures of the primitives, if they conform to the XML
void VerifyPrimitivesStructures();

// ***************************************************************************
// IPrimitiveEditor
// ***************************************************************************

class IPrimitiveEditor
{
	friend void UpdatePrimitives ();
	friend void InvalidatePrimitive (const CDatabaseLocatorPointer &locator, uint channels);
	friend void AddPrimitivesLogicTree (const CDatabaseLocatorPointer &locator, IPrimitiveEditor *primitiveEditor, class CToolsLogic *toolWnd);
	friend void UpdatePrimitiveSelection (IPrimitiveEditor *primitiveEditor, CDatabaseLocatorPointer &locator, bool &);
	friend void UpdateSelection ();
	friend void InvalidatePrimitive (const CDatabaseLocatorPointer &locator, uint channels);
	friend class CToolsLogic;
	friend class CMainFrame;
public:
	// used to mark errors in structures of the tree
	enum StructureError
	{
		NoError = 0,
		MissingField = (1<<0),
		InExcessField = (1<<1)
	};
	IPrimitiveEditor ()
	{
		_QuadIterator = PrimitiveQuadGrid.end ();
		_QuadIteratorLink = PrimitiveQuadGrid.end ();
		_SelectionIterator = Selection.end ();
		_TreeItem = NULL;
		_Channels = 0;
		_ModifiedIterator = ModifiedPrimitive.end ();
		_Error = NoError;

		_Expanded = true;
		_Hidden = false;
		_Selected = false;

	}

	IPrimitiveEditor (const IPrimitiveEditor &other)
	{
		_QuadIterator = PrimitiveQuadGrid.end ();
		_QuadIteratorLink = PrimitiveQuadGrid.end ();
		_SelectionIterator = Selection.end ();
		_TreeItem = NULL;
		_Channels = 0;
		_ModifiedIterator = ModifiedPrimitive.end ();
		_Error = NoError;

		_Expanded = other._Expanded;
		_Hidden = other._Hidden;
		_Selected = false;
	}

	virtual ~IPrimitiveEditor ();
	
	friend IPrimitiveEditor::StructureError VerifyPrimitiveRec (const CDatabaseLocatorPointer &locator, const std::string &path, std::string &errors);

public:
	// Use by removeFromLogicTree ();
	virtual void resetTreeItemRec ();

	// Remove the primitive from the logic tree
	void removeFromLogicTree ();

	void setHidden(bool hidden) const
	{
		_Hidden = hidden;
	}
	bool getHidden() const
	{
		return _Hidden;
	}
	void setExpanded(bool expanded) const
	{
		_Expanded = expanded;
	}
	bool getExpanded() const
	{
		return _Expanded;
	}
	void setSelected(bool selected) const
	{
		_Selected = selected;
	}
	bool getSelected() const
	{
		return _Selected;
	}

protected:
	// callback used to read the editor properties
	void postReadCallback(const NLLIGO::IPrimitive *thePrim);
	// callback used to write the editor properties
	void preWriteCallback(const NLLIGO::IPrimitive *thePrim) const;
private:

	/* Remove the primitive from the quad grid.
	 * This method removes the link to the other primitive if it exists.
	 */
	void removeFromQuadGrid ();

	// Remove the primitive from the selection
	void removeFromSelection ();

	// Remove the primitive from modified primitive
	void removeFromModified ();

	// Invalidated channels
	uint							_Channels;

	// Index in the tree
	std::list<NLLIGO::IPrimitive*>::iterator	_ModifiedIterator;
	HTREEITEM						_TreeItem;

	// Iterator on the quad grid
	TPrimQuadGrid::CIterator		_QuadIterator;
	TPrimQuadGrid::CIterator		_QuadIteratorLink;

	// Iterator on the quad grid
	std::list<NLLIGO::IPrimitive*>::iterator	_SelectionIterator;

	// 
	mutable bool	_Expanded;
	mutable bool	_Hidden;
	mutable bool	_Selected;

	StructureError _Error;
};


// ***************************************************************************
// CPrimNodeEditor
// ***************************************************************************

class CPrimNodeEditor : public NLLIGO::CPrimNode, public IPrimitiveEditor
{
public:
	CPrimNodeEditor ()
	{
	}

	~CPrimNodeEditor ()
	{
		removeFromLogicTree ();
	}

	// From IPrimitiveEditor
	virtual void resetTreeItemRec ()
	{
		IPrimitiveEditor::resetTreeItemRec ();
		uint i;
		for (i=0; i<getNumChildren (); i++)
		{
			NLLIGO::IPrimitive *child;
			nlverify (getChild (child, i));
			IPrimitiveEditor *childEditor = dynamic_cast<IPrimitiveEditor*> (child);
			if (childEditor)
				childEditor->resetTreeItemRec ();
		}
	}

	// From IPrimitive
	virtual NLLIGO::IPrimitive *copy () const;
	// Read the primitive, to post call the CPrimitiveEditor load method
	virtual bool read (xmlNodePtr xmlNode, const char *filename, uint version, NLLIGO::CLigoConfig &config);
	// Write the primitive, to pre call the CPrimitiveEditor load method
	virtual void write (xmlNodePtr xmlNode, const char *filename) const;
	// Callback called when the primitive is updated, giving a chance to track the primitive's modifications during the loading
	virtual void onModifyPrimitive (NLLIGO::CPrimitives &primitives) const;

	// From IClassable
	virtual std::string	getClassName() {return "CPrimNode";}
	static	NLMISC::IClassable	*creator() {return new CPrimNodeEditor;}
};

// ***************************************************************************
// CPrimPointEditor
// ***************************************************************************

class CPrimPointEditor : public NLLIGO::CPrimPoint, public IPrimitiveEditor
{
public:
	CPrimPointEditor ()
	{
	}
	
	~CPrimPointEditor ()
	{
		removeFromLogicTree ();
	}

	// From IPrimitiveEditor
	virtual void resetTreeItemRec ()
	{
		IPrimitiveEditor::resetTreeItemRec ();
		uint i;
		for (i=0; i<getNumChildren (); i++)
		{
			NLLIGO::IPrimitive *child;
			nlverify (getChild (child, i));
			IPrimitiveEditor *childEditor = dynamic_cast<IPrimitiveEditor*> (child);
			if (childEditor)
				childEditor->resetTreeItemRec ();
		}
	}

	// From IClassable
	virtual NLLIGO::IPrimitive *copy () const;
	// Read the primitive, to post call the CPrimitiveEditor load method
	virtual bool read (xmlNodePtr xmlNode, const char *filename, uint version, NLLIGO::CLigoConfig &config);
	// Write the primitive, to pre call the CPrimitiveEditor load method
	virtual void write (xmlNodePtr xmlNode, const char *filename) const;
	// Callback called when the primitive is updated, giving a chance to track the primitive's modifications during the loading
	virtual void onModifyPrimitive (NLLIGO::CPrimitives &primitives) const;

	// From IClassable
	virtual std::string	getClassName() {return "CPrimPoint";}
	static	NLMISC::IClassable	*creator() {return new CPrimPointEditor;}
};

// ***************************************************************************
// CPrimPathEditor
// ***************************************************************************

class CPrimPathEditor : public NLLIGO::CPrimPath, public IPrimitiveEditor
{
public:
	CPrimPathEditor ()
	{
	}

	~CPrimPathEditor ()
	{
		removeFromLogicTree ();
	}

	// From IPrimitiveEditor
	virtual void resetTreeItemRec ()
	{
		IPrimitiveEditor::resetTreeItemRec ();
		uint i;
		for (i=0; i<getNumChildren (); i++)
		{
			NLLIGO::IPrimitive *child;
			nlverify (getChild (child, i));
			IPrimitiveEditor *childEditor = dynamic_cast<IPrimitiveEditor*> (child);
			if (childEditor)
				childEditor->resetTreeItemRec ();
		}
	}

	// From IPrimitive
	virtual NLLIGO::IPrimitive *copy () const;
	// Read the primitive, to post call the CPrimitiveEditor load method
	virtual bool read (xmlNodePtr xmlNode, const char *filename, uint version, NLLIGO::CLigoConfig &config);
	// Write the primitive, to pre call the CPrimitiveEditor load method
	virtual void write (xmlNodePtr xmlNode, const char *filename) const;
	// Callback called when the primitive is updated, giving a chance to track the primitive's modifications during the loading
	virtual void onModifyPrimitive (NLLIGO::CPrimitives &primitives) const;

	// From IClassable
	virtual std::string	getClassName() {return "CPrimPath";}
	static	NLMISC::IClassable	*creator() {return new CPrimPathEditor;}


};

// ***************************************************************************
// CPrimZoneEditor
// ***************************************************************************

class CPrimZoneEditor : public NLLIGO::CPrimZone, public IPrimitiveEditor
{
public:
	CPrimZoneEditor ()
	{
	}

	~CPrimZoneEditor ()
	{
		removeFromLogicTree ();
	}

	// From IPrimitiveEditor
	virtual void resetTreeItemRec ()
	{
		IPrimitiveEditor::resetTreeItemRec ();
		uint i;
		for (i=0; i<getNumChildren (); i++)
		{
			NLLIGO::IPrimitive *child;
			nlverify (getChild (child, i));
			IPrimitiveEditor *childEditor = dynamic_cast<IPrimitiveEditor*> (child);
			if (childEditor)
				childEditor->resetTreeItemRec ();
		}
	}

	// From IPrimitive
	virtual NLLIGO::IPrimitive *copy () const;
	// Read the primitive, to post call the CPrimitiveEditor load method
	virtual bool read (xmlNodePtr xmlNode, const char *filename, uint version, NLLIGO::CLigoConfig &config);
	// Write the primitive, to pre call the CPrimitiveEditor load method
	virtual void write (xmlNodePtr xmlNode, const char *filename) const;
	// Callback called when the primitive is updated, giving a chance to track the primitive's modifications during the loading
	virtual void onModifyPrimitive (NLLIGO::CPrimitives &primitives) const;

	// From IClassable
	virtual std::string	getClassName() {return "CPrimZone";}
	static	NLMISC::IClassable	*creator() {return new CPrimZoneEditor;}
};

// ***************************************************************************
// CPrimAliasEditor
// ***************************************************************************

class CPrimAliasEditor : public NLLIGO::CPrimAlias, public IPrimitiveEditor
{
public:
	CPrimAliasEditor ()
	{
	}

	~CPrimAliasEditor ()
	{
		removeFromLogicTree ();
	}

	// From IPrimitiveEditor
	virtual void resetTreeItemRec ()
	{
		IPrimitiveEditor::resetTreeItemRec ();
		uint i;
		for (i=0; i<getNumChildren (); i++)
		{
			NLLIGO::IPrimitive *child;
			nlverify (getChild (child, i));
			IPrimitiveEditor *childEditor = dynamic_cast<IPrimitiveEditor*> (child);
			if (childEditor)
				childEditor->resetTreeItemRec ();
		}
	}

	// From IPrimitive
	virtual NLLIGO::IPrimitive *copy () const;
	// Read the primitive, to post call the CPrimitiveEditor load method
	virtual bool read (xmlNodePtr xmlNode, const char *filename, uint version, NLLIGO::CLigoConfig &config);
	// Write the primitive, to pre call the CPrimitiveEditor load method
	virtual void write (xmlNodePtr xmlNode, const char *filename) const;
	// Callback called when the primitive is updated, giving a chance to track the primitive's modifications during the loading
	virtual void onModifyPrimitive (NLLIGO::CPrimitives &primitives) const;

	// From IClassable
	virtual std::string	getClassName() {return "CPrimAlias";}
	static	NLMISC::IClassable	*creator();
};

// ***************************************************************************
// CPrimBitmap
// ***************************************************************************

// A new primitive for files
class CPrimBitmap : public CPrimNodeEditor
{
public:
	CPrimBitmap ();

	// Init
	void			init (const char *filename);

	// \name From IClassable
	NLMISC_DECLARE_CLASS (CPrimBitmap)

	// Get the texture
	NL3D::CTextureBlank	*getTexture () const;

private:

	// Loaded
	mutable bool	_Loaded;

	// The texture
	mutable NLMISC::CSmartPtr<NL3D::CTextureBlank>	_Texture;
};

IPrimitiveEditor::StructureError VerifyPrimitiveRec (const CDatabaseLocatorPointer &locator, const std::string &path, std::string &errors);


const IPrimitiveEditor *getPrimitiveEditor(const NLLIGO::IPrimitive *primitive);
IPrimitiveEditor *getPrimitiveEditor(NLLIGO::IPrimitive *primitive);

#endif // NL_EDITOR_PRIMITIVE_H

/* End of editor_primitive.h */
