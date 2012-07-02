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

#ifndef NL_ACTION_H
#define NL_ACTION_H

#include "world_editor_doc.h"


/**
 * Basic editing action
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2002
 */
class IAction
{
public:

	// Virtual destructor
	virtual ~IAction () {}

	// Do / undo
	virtual void undo () = 0;
	virtual bool redo () = 0;

	// Used to deferentiate selection from real action on the doc
	virtual bool isAffectingContent() {return true;}
};

// An intermediate base class for selection or hide/show action
class CActionSelectionBase : public IAction
{
	bool isAffectingContent() { return false;}
};


// Add a landscape
class CActionAddLandscape : public IAction
{
public:
	// Landscape name
	CActionAddLandscape (const char *name);

private:

	// From IAction
	virtual void undo ();
	virtual bool redo ();

	std::string			_Filename;
	bool				_FirstTime;
	NLLIGO::CZoneRegion	_NewRegion;
};

// New landscape document
class CActionNewLandscape : public IAction
{
public:

	// Constructor
	CActionNewLandscape ();

private:

	// From IAction
	virtual void undo ();
	virtual bool redo ();
};

// Modify the landscape
class CActionLigoTile : public IAction
{
public:

	// Constructor
	CActionLigoTile (const CLigoData &data, const CDatabaseLocator &locator);

private:

	// From IAction
	virtual void undo ();
	virtual bool redo ();

	void set (const CLigoData &data);

	CDatabaseLocator	_Locator;
	CLigoData		_Old;
	CLigoData		_New;
};

// Move the landscape
class CActionLigoMove : public IAction
{
public:

	// Constructor
	CActionLigoMove (uint index, sint32 deltaX, sint32 deltaY);

private:

	// From IAction
	virtual void undo ();
	virtual bool redo ();

	uint			_Index;
	sint32			_DeltaX;
	sint32			_DeltaY;
};

// Modify the landscape
class CActionLigoResize : public IAction
{
public:

	// Constructor
	CActionLigoResize (uint index, sint32 newMinX, sint32 newMaxX, sint32 newMinY, sint32 newMaxY);

private:

	// From IAction
	virtual void undo ();
	virtual bool redo ();

	uint			_Index;
	sint32			_NewMinX;
	sint32			_NewMaxX;
	sint32			_NewMinY;
	sint32			_NewMaxY;
	NLLIGO::CZoneRegion		_Old;
};

// Import the landscape
class CActionImportPrimitive : public IAction
{
public:

	// Constructor
	CActionImportPrimitive (const char *oldPrimFile);

private:

	// From IAction
	virtual void undo ();
	virtual bool redo ();

	bool					_FirstLoad;
	std::string				_Filename;
	NLLIGO::CPrimitives		_Primitive;
};

// Import the landscape
class CActionLoadPrimitive : public IAction
{
public:

	// Constructor
	CActionLoadPrimitive (const char *oldPrimFile);

private:

	// From IAction
	virtual void undo ();
	virtual bool redo ();

	bool					_FirstLoad;
	std::string				_Filename;
	NLLIGO::CPrimitives		_Primitive;
};

// New primitive document
class CActionNewPrimitive : public IAction
{
public:

	// Constructor
	CActionNewPrimitive ();

private:

	// From IAction
	virtual void undo ();
	virtual bool redo ();
};

// Clear primitives
class CActionClearPrimitives : public IAction
{
public:

	// Constructor
	CActionClearPrimitives ();

private:

	// From IAction
	virtual void undo ();
	virtual bool redo ();

	CWorldEditorDoc::CDatabaseList	_Primitives;
};

// Delete a primitive
class CActionDeleteDatabaseElement : public IAction
{
public:

	// Constructor
	CActionDeleteDatabaseElement (uint prim);
	~CActionDeleteDatabaseElement ();

private:

	// From IAction
	virtual void undo ();
	virtual bool redo ();

	uint									_Index;
	CWorldEditorDoc::CDatabaseElement		*_Primitive;
};

// XChg the database element with the next one
class CActionXChgDatabaseElement : public IAction
{
public:

	// Constructor
	CActionXChgDatabaseElement (uint prim);
	~CActionXChgDatabaseElement ();

private:

	// From IAction
	virtual void undo ();
	virtual bool redo ();

	uint									_Index;
};

// Unselect all
class CActionUnselectAll : public CActionSelectionBase
{
public:

	// Constructor
	CActionUnselectAll ();

private:

	// From IAction
	virtual void undo ();
	virtual bool redo ();

	std::vector<CDatabaseLocator>	_SelectedPrimitives;
};

// Select all
class CActionSelectAll : public CActionSelectionBase
{
public:

	// Constructor
	CActionSelectAll ();

private:

	// From IAction
	virtual void undo ();
	virtual bool redo ();

	std::vector<CDatabaseLocator>	_SelectedPrimitives;
};

// Unselect
class CActionUnselect : public CActionSelectionBase
{
public:

	// Constructor
	CActionUnselect (const CDatabaseLocator &locator);

private:

	// From IAction
	virtual void undo ();
	virtual bool redo ();

	CDatabaseLocator	_Locator;
};

// Select
class CActionSelect : public CActionSelectionBase
{
public:

	// Constructor
	CActionSelect (const CDatabaseLocator &locator);

private:

	// From IAction
	virtual void undo ();
	virtual bool redo ();

	CDatabaseLocator	_Locator;
};

// Unselect all sub primitive
class CActionUnselectAllSub : public CActionSelectionBase
{
public:

	// Constructor
	CActionUnselectAllSub ();

private:

	// From IAction
	virtual void undo ();
	virtual bool redo ();

	std::vector<CDatabaseLocator>	_SelectedPrimitives;
};

// Select all sub primitive
class CActionSelectAllSub : public CActionSelectionBase
{
public:

	// Constructor
	CActionSelectAllSub ();

private:

	// From IAction
	virtual void undo ();
	virtual bool redo ();

	std::vector<CDatabaseLocator>	_SelectedPrimitives;
};

// Unselect sub primitive
class CActionUnselectSub : public CActionSelectionBase
{
public:

	// Constructor
	CActionUnselectSub (const CDatabaseLocator &locator);

private:

	// From IAction
	virtual void undo ();
	virtual bool redo ();

	CDatabaseLocator	_Locator;
};

// Select sub primitive
class CActionSelectSub : public CActionSelectionBase
{
public:

	// Constructor
	CActionSelectSub (const CDatabaseLocator &locator);

private:

	// From IAction
	virtual void undo ();
	virtual bool redo ();

	CDatabaseLocator	_Locator;
};

// Interactife action 
class IActionInteractive : public IAction
{
public:
	virtual bool getText (std::string &result) { return false; }
	virtual bool getHelp (std::string &result) { return false; }
};

// Move selection action
class CActionMove : public IActionInteractive
{
public:

	CActionMove (bool subSelection);

	// Init after 
	void setTranslation (const NLMISC::CVector &translation);

	// From IAction
	virtual void undo ();
	virtual bool redo ();

	// From IActionInteractive
	virtual bool getText (std::string &result);

private:

	// Undo entry
	class  CUndoEntry
	{
	public:
		CUndoEntry (const CDatabaseLocator &locator, const NLMISC::CVector &oldPosition)
		{
			Locator = locator;
			OldPosition = oldPosition;
		}

		// The locator
		CDatabaseLocator	Locator;

		// The old position
		NLMISC::CVector		OldPosition;
	};

	// Vector of primitive to modify
	std::vector<CUndoEntry>	_Entities;

	// The translation
	NLMISC::CVector			_Translation;

	// Sub selection ?
	bool					_SubSelection;
};

// Rotate selection action
class CActionRotate : public IActionInteractive
{
public:

	CActionRotate (bool subSelection, const NLMISC::CVector &pivot);

	// Init after 
	void setAngle(float angle, bool rotate, bool turn);

	// From IAction
	virtual void undo ();
	virtual bool redo ();

	// From IActionInteractive
	virtual bool getHelp (std::string &result);
	virtual bool getText (std::string &result);

private:

	// Undo entry
	class  CUndoEntry
	{
	public:
		CUndoEntry (const CDatabaseLocator &locator, const NLMISC::CVector &oldPosition, bool pointPrimitive, float pointPrimitiveAngle)
		{
			Locator = locator;
			OldPosition = oldPosition;
			PointPrimitive = pointPrimitive;
			PointPrimitiveAngle = pointPrimitiveAngle;
		}

		// The locator
		CDatabaseLocator	Locator;

		// The old position
		NLMISC::CVector		OldPosition;

		// Is it a point primitive
		bool				PointPrimitive;

		// Is it a point primitive
		float				PointPrimitiveAngle;
	};

	// Vector of primitive to modify
	std::vector<CUndoEntry>	_Entities;

	// The translation
	float					_Angle;

	// Sub selection ?
	bool					_SubSelection;

	// Rotate or turn ?
	bool					_Rotate:1;
	bool					_Turn:1;

	// Pivot
	NLMISC::CVector			Pivot;
};

// Scale selection action
class CActionScale : public IActionInteractive
{
public:

	CActionScale (bool subSelection, const NLMISC::CVector &pivot);

	// Init after 
	void setScale (float factor, bool scale, bool radius);

	// From IAction
	virtual void undo ();
	virtual bool redo ();

	// From IActionInteractive
	virtual bool getHelp (std::string &result);
	virtual bool getText (std::string &result);

private:

	// Undo entry
	class  CUndoEntry
	{
	public:
		CUndoEntry (const CDatabaseLocator &locator, const NLMISC::CVector &oldPosition, bool hasRadius, float radius)
		{
			Locator = locator;
			OldPosition = oldPosition;
			HasRadius = hasRadius;
			Radius = radius;
		}

		// The locator
		CDatabaseLocator	Locator;

		// The old position
		NLMISC::CVector		OldPosition;

		// Does it have a radius
		bool				HasRadius;
		float				Radius;
	};

	// Vector of primitive to modify
	std::vector<CUndoEntry>	_Entities;

	// The translation
	float					_Factor;
	bool					_Scale:1;
	bool					_Radius:1;

	// Sub selection ?
	bool					_SubSelection;

	// Pivot
	NLMISC::CVector			Pivot;
};

// Select
class CActionAddVertex : public IActionInteractive
{
public:

	// Constructor
	CActionAddVertex (const CDatabaseLocator &locator, const NLMISC::CVector &newVertex);

private:

	// From IAction
	virtual void undo ();
	virtual bool redo ();

	CDatabaseLocator	_Locator;
	NLMISC::CVector		_NewVertex;
};

// Delete a vertex
class CActionDeleteSub : public IAction
{
public:

	// Constructor
	CActionDeleteSub (const CDatabaseLocatorPointer &locator);

private:

	// From IAction
	virtual void undo ();
	virtual bool redo ();

	CDatabaseLocator	_Locator;
	NLLIGO::CPrimVector	_OldVertex;
};

// Delete a primitive
class CActionDelete : public IAction
{
public:

	// Constructor
	CActionDelete (const CDatabaseLocatorPointer &locator);
	~CActionDelete ();

private:

	// From IAction
	virtual void undo ();
	virtual bool redo ();

	CDatabaseLocator	_Locator;
	NLLIGO::IPrimitive	*_OldPrimitive;
};

// Add a primitive by class
class CActionAddPrimitiveByClass : public IAction
{
public:

	// Constructor
	CActionAddPrimitiveByClass (const CDatabaseLocator &locator, const char *className, const NLMISC::CVector &initPos, float deltaPos,
								const std::vector<NLLIGO::CPrimitiveClass::CInitParameters>	initParameters);

private:

	// From IAction
	virtual void undo ();
	virtual bool redo ();

	NLMISC::CVector			_InitPos;
	float					_DeltaPos;
	NLLIGO::IPrimitive		*_OldPrimitive;
	std::string				_ClassName;
	CDatabaseLocator		_Locator;
	std::vector<NLLIGO::CPrimitiveClass::CInitParameters>	_InitParameters;
};

// Change primitive property
class CActionSetPrimitivePropertyString : public IAction
{
public:

	// Constructor
	CActionSetPrimitivePropertyString (const CDatabaseLocatorPointer &locator, const char *propertyName, const char *newValue, bool _default);

private:

	// From IAction
	virtual void undo ();
	virtual bool redo ();

	bool					_OldExist;
	std::string				_PropertyName;
	std::string				_PropertyOldValue;
	bool					_PropertyOldValueDefault;
	std::string				_PropertyNewValue;
	bool					_PropertyNewValueDefault;
	CDatabaseLocator		_Locator;
};

// Change primitive property
class CActionSetPrimitivePropertyStringArray : public IAction
{
public:

	// Constructor
	CActionSetPrimitivePropertyStringArray (const CDatabaseLocatorPointer &locator, const char *propertyName, const std::vector<std::string> &newValue, bool _default);

private:

	// From IAction
	virtual void undo ();
	virtual bool redo ();

	bool					_OldExist;
	std::string				_PropertyName;
	std::vector<std::string>	_PropertyOldValue;
	bool						_PropertyOldValueDefault;
	std::vector<std::string>	_PropertyNewValue;
	bool						_PropertyNewValueDefault;
	CDatabaseLocator		_Locator;
};

// Change primitive property
class CActionAddPrimitive : public IAction
{
public:

	// Constructor, will copy the primitive
	CActionAddPrimitive (const NLLIGO::IPrimitive &primitiveToAdd, const CDatabaseLocator &locator);

	// Constructor, will keep the primitive allocated with new
	CActionAddPrimitive (NLLIGO::IPrimitive *primitiveToAdd, const CDatabaseLocator &locator);

	// Destructor
	~CActionAddPrimitive ();

private:

	// From IAction
	virtual void undo ();
	virtual bool redo ();

	bool					_FirstTime;
	NLLIGO::IPrimitive		*_Primitive;
	CDatabaseLocator		_Locator;
};


// Show / hide a primitive
class CActionShowHide : public CActionSelectionBase
{
public:
	CActionShowHide (const CDatabaseLocatorPointer &locator, bool show);

private:

	// From IAction
	virtual void undo ();
	virtual bool redo ();

	bool					_Show;
	CDatabaseLocator		_Locator;
};

#endif // NL_ACTION_H

/* End of action.h */
